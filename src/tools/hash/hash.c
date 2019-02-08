#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <celib/core.h>
#include <celib/log.h>

#include <celib/memory/memory.h>
#include <celib/memory/allocator.h>
#include <celib/murmur_hash.h>
#include <celib/containers/hash.h>
#include <celib/task.h>
#include <celib/os/path.h>
#include <celib/os/vio.h>

#define SEED 0

#define MACRO_BEGIN "CE_ID64_0(\""
#define MACRO_LEN 11
#define MACRO_TEMPLATE "CE_ID64_0(\"%.*s\", 0x%" PRIx64 "ULL)"


void process_file(void *data) {
    const char *filename = data;

    ce_log_a0->info("hash", "Process file: %s", filename);

    struct ce_vio *file = ce_os_vio_a0->from_file(filename, VIO_OPEN_READ);
    uint64_t size = file->size(file);
    char *input_data = CE_ALLOC(ce_memory_a0->system, char, size + 1);
    file->read(file, input_data, 1, size);
    file->close(file);

    input_data[size] = '\0';

    uint32_t c = 0;
    char buffer[1024] = {};
    bool change = false;
    while (c < size) {
        if (input_data[c] == '\n') {
            ++c;
            continue;
        } else if (input_data[c] != 'C') {
            ++c;
            continue;
        }


        if (c + MACRO_LEN > size) {
            break;
        }

        if (memcmp(input_data + c, MACRO_BEGIN, MACRO_LEN)) {
            ++c;
            continue;
        }

        const char *macro_s = input_data + c;
        const char *string_s = macro_s + MACRO_LEN;
        const char *string_e = (char *) memchr(string_s,
                                               '"', size - c - MACRO_LEN);
        if (!string_e || string_e - string_s > 512) {
            ++c;
            continue;
        }

        const char *macro_e = (char *) memchr(
                string_e, ')',
                (size_t) (input_data + (int64_t) size - string_e)) + 1;

        const uint32_t string_len = (uint32_t) (string_e - string_s);
        const uint32_t macro_len = (uint32_t) (macro_e - macro_s);

        const uint64_t hash = ce_hash_murmur2_64(string_s, string_len, SEED);
        sprintf(buffer, MACRO_TEMPLATE, string_len, string_s, hash);

        const uint32_t buffer_len = (uint32_t) strlen(buffer);
        if (c + buffer_len > size || 0 == memcmp(macro_s, buffer, buffer_len)) {
            c += buffer_len;
            continue;
        }

        change = true;
        const int64_t extra = (int64_t) buffer_len - (int64_t) macro_len;
        input_data = ce_memory_a0->system->reallocate(ce_memory_a0->system, input_data,
                                   (uint64_t) ((int64_t) size + extra + 1), 0,
                                   __FILE__, __LINE__);
        memmove(input_data + c + extra, input_data + c, size + 1 - c);
        memcpy(input_data + c, buffer, buffer_len);
        size = (uint64_t) ((int64_t) size + extra);
        c += buffer_len;
    }

    if (change) {
        file = ce_os_vio_a0->from_file(filename, VIO_OPEN_WRITE);
        file->write(file, input_data, size, 1);
        file->close(file);
    }

    CE_FREE(ce_memory_a0->system, input_data);
}

void print_usage() {
    ce_log_a0->info(
            "doc", "%s",

            "usage: hash --source SOURCE_DIR\n"
            "\n"
            "  Find *.h, *.inl, *.c, *.cpp files in SOURCE_DIR and update static hash\n"
            "  Ignore all 'private' folders.\n"
            "\n"
            "    --source SOURCE_DIR  - Source dir\n"
            "    -h,--help            - Print this help\n"
    );
}

int main(int argc,
         const char **argv) {

    const char* source_dir = NULL;
    bool printusage = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--source") == 0) {
            source_dir = argv[i + 1];
            ++i;
        } else if ((strcmp(argv[i], "-h") == 0) ||
                   (strcmp(argv[i], "--help") == 0)) {
            printusage = true;
            break;
        } else {
            printusage = true;
            break;
        }
    }

    struct ce_alloc_t0 *a = ce_memory_a0->system;
    ce_log_a0->register_handler(ce_log_a0->stdout_handler, NULL);

    if (printusage) {
        print_usage();
        return 1;
    }


    ce_init();

    char **files;
    uint32_t files_count;

    const char *filter[] = {"*.c", "*.h", "*.inl", "*.cpp"};
    ce_os_path_a0->list(source_dir, CE_ARR_ARG(filter),
                         1, 0, &files, &files_count, a);

    struct ce_task_item tasks[files_count];

    struct ce_task_counter_t *counter = NULL;

    for (uint32_t i = 0; i < files_count; ++i) {
        tasks[i].data = files[i];
        tasks[i].work = process_file;
    }

    ce_task_a0->add(tasks, files_count, &counter);
    ce_task_a0->wait_for_counter(counter, 0);

    ce_os_path_a0->list_free(files, files_count, a);

    ce_shutdown();
}
