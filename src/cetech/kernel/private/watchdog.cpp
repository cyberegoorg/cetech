#include <unistd.h>
#include <errno.h>

#include <sys/inotify.h>

#include <celib/macros.h>
#include <celib/map.inl>
#include <celib/eventstream.inl>

#include <cetech/kernel/watchdog.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/errors.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/path.h>

#define LOG_WHERE "watchdog"

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_path_a0);

struct watchdog_instance {
    celib::EventStream event_stream;
    celib::Map<char *> wd2dir;
    celib::Map<int> dir2wd;
    cel_alloc *alloc;
    int inotify;
};

void add_dir(ct_watchdog_instance_t *inst,
             const char *path,
             bool recursive) {
    watchdog_instance *wi = static_cast<watchdog_instance *>(inst);

    int wd = 0;

    wd = inotify_add_watch(wi->inotify, path, IN_ALL_EVENTS);
    ct_log_a0.debug(LOG_WHERE, "New watch -> %s", path);

    if (-1 == wd) {
        ct_log_a0.error(LOG_WHERE, "Could not add watch -> %s",
                        strerror(errno));
        return;
    }

    char *path_dup = ct_memory_a0.str_dup(path, wi->alloc);
    uint64_t path_hash = ct_hash_a0.id64_from_str(path_dup);

    celib::map::set(wi->dir2wd, path_hash, wd);
    celib::map::set(wi->wd2dir, wd, path_dup);

    if (recursive) {
        char **files;
        uint32_t files_count;

        ct_path_a0.list(path, "*", 1, 1, &files, &files_count,
                        ct_memory_a0.main_scratch_allocator());

        for (uint32_t i = 0; i < files_count; ++i) {
            add_dir(inst, files[i], false);
        }

        ct_path_a0.list_free(files, files_count,
                             ct_memory_a0.main_scratch_allocator());
    }
}


ct_watchdog_ev_header *event_begin(ct_watchdog_instance_t *inst) {
    watchdog_instance *wi = static_cast<watchdog_instance *>(inst);

    return celib::eventstream::begin<ct_watchdog_ev_header>(
            wi->event_stream);
}

ct_watchdog_ev_header *event_end(ct_watchdog_instance_t *inst) {
    watchdog_instance *wi = static_cast<watchdog_instance *>(inst);

    return celib::eventstream::end<ct_watchdog_ev_header>(wi->event_stream);
}

ct_watchdog_ev_header *event_next(ct_watchdog_instance_t *inst,
                                  ct_watchdog_ev_header *header) {

    CEL_UNUSED(inst);

    return celib::eventstream::next<ct_watchdog_ev_header>(header);
}


void clean_events(watchdog_instance *wi) {
    auto *wd_it = celib::eventstream::begin<ct_watchdog_ev_header>(
            wi->event_stream);
    const auto *wd_end = celib::eventstream::end<ct_watchdog_ev_header>(
            wi->event_stream);

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
            ct_wd_ev_file_write_end *ev = reinterpret_cast<ct_wd_ev_file_write_end *>(wd_it);
            CEL_FREE(wi->alloc, ev->filename);
        }

        wd_it = celib::eventstream::next<ct_watchdog_ev_header>(wd_it);
    }


    celib::eventstream::clear(wi->event_stream);
}

void fetch_events(ct_watchdog_instance_t *inst) {
    watchdog_instance *wi = static_cast<watchdog_instance *>(inst);

    clean_events(wi);

#define BUF_LEN 1024
    char buf[BUF_LEN] __attribute__ ((aligned(8)));

    ssize_t numRead = read(wi->inotify, buf, BUF_LEN);
    if (numRead <= 0) {
        return;
    }

    struct inotify_event *event;
    for (char *p = buf;
         p < buf + numRead; p += sizeof(struct inotify_event) +
                                 event->len) {
        event = (struct inotify_event *) p;

        if (event->mask & IN_ISDIR) {

        } else {
            if (event->mask & (IN_CLOSE_WRITE | IN_MOVE)) {
                ct_wd_ev_file_write_end ev = {};

                char *path = celib::map::get<char *>(
                        wi->wd2dir,
                        static_cast<uint64_t>(event->wd),
                        NULL);

                ev.dir = path;
                ev.filename = ct_memory_a0.str_dup(event->name, wi->alloc);

                celib::eventstream::push<ct_watchdog_ev_header>(
                        wi->event_stream,
                        CT_WATCHDOG_EVENT_FILE_MODIFIED,
                        ev);

                continue;
            }
        }
    }
}

ct_watchdog *create(struct cel_alloc *alloc) {
    ct_watchdog *watchdog = CEL_ALLOCATE(alloc, ct_watchdog,
                                         sizeof(ct_watchdog));
    watchdog_instance *watchdog_inst = CEL_ALLOCATE(alloc, watchdog_instance,
                                                    sizeof(watchdog_instance));

    watchdog_inst->event_stream.init(alloc);
    watchdog_inst->wd2dir.init(alloc);
    watchdog_inst->dir2wd.init(alloc);

    if (NULL == watchdog) {
        return NULL;
    }

    int inotify = inotify_init1(IN_NONBLOCK);
    if (-1 == inotify) {
        ct_log_a0.error(LOG_WHERE, "Could not init inotify");
        return NULL;
    }
    watchdog_inst->inotify = inotify;
    watchdog_inst->alloc = alloc;

    *watchdog = {
            .inst = watchdog_inst,
            .add_dir = add_dir,
            .fetch_events = fetch_events,
            .event_begin = event_begin,
            .event_end = event_end,
            .event_next = event_next
    };

    return watchdog;
}

void destroy(struct ct_watchdog *watchdog) {
    watchdog_instance *wi = static_cast<watchdog_instance *>(watchdog->inst);

    clean_events(wi);
    wi->event_stream.destroy();

    cel_alloc *alloc = wi->alloc;

    auto ct_it = celib::map::begin(wi->wd2dir);
    auto ct_end = celib::map::end(wi->wd2dir);

    while (ct_it != ct_end) {
        CEL_FREE(alloc, ct_it->value);

        inotify_rm_watch(wi->inotify, static_cast<int>(ct_it->key));

        ++ct_it;
    }

    close(wi->inotify);

    wi->wd2dir.destroy();
    wi->dir2wd.destroy();


    CEL_FREE(alloc, watchdog);
}

static ct_watchdog_a0 wathdog_api = {
        .create = create,
        .destroy = destroy,
};

CETECH_MODULE_DEF(
        watchdog,
        {
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_path_a0);
        },
        {
            CEL_UNUSED(reload);
            api->register_api("ct_watchdog_a0", &wathdog_api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
        }
)
