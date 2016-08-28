//==============================================================================
// Includes
//==============================================================================

#include <celib/path/path.h>
#include "llm/fs.h"
#include "celib/containers/array.h"
#include "celib/string/string.h"

#include "llm/vio.h"
#include "llm/llm.h"

#include "engine/filesystem/filesystem.h"


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "filesystem"

#define MAX_PATH_LEN 128
#define MAX_ROOTS 32



//==============================================================================
// Global
//==============================================================================

static struct G {
    struct {
        stringid64_t id[MAX_ROOTS];
        const char *path[MAX_ROOTS];
    } rootmap;
} _G = {0};


//==============================================================================
// Interface
//==============================================================================

void filesystem_init() {
    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");
}

void filesystem_shutdown() {
    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Shutdown");
}

void filesystem_map_root_dir(stringid64_t root, const char *base_path) {
    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.path[i] != 0) {
            continue;
        }

        _G.rootmap.id[i] = root;
        _G.rootmap.path[i] = base_path;
    }
}

const char *filesystem_get_root_dir(stringid64_t root) {
    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.id[i].id != root.id) {
            continue;
        }

        return _G.rootmap.path[i];
    }

    return NULL;
}

int filesystem_get_fullpath(stringid64_t root, char *result, u64 maxlen, const char *filename) {
    const char *root_path = filesystem_get_root_dir(root);

    return path_join(result, maxlen, root_path, filename) == (str_lenght(root_path) + str_lenght(filename) + 1);
}

struct vio *filesystem_open(stringid64_t root, const char *path, enum open_mode mode) {
    char fullm_path[MAX_PATH_LEN] = {0};

    if (!filesystem_get_fullpath(root, fullm_path, sizeof(fullm_path) / sizeof(char), path)) {
        return NULL;
    }

    struct vio *file = vio_from_file(fullm_path, mode, memsys_main_allocator());

    if (!file) {
        return NULL;
    }

    return file;
}

void filesystem_close(struct vio *file) {
    vio_close(file);
}

int filesystem_create_directory(stringid64_t root, const char *path) {
    char fullm_path[MAX_PATH_LEN] = {0};

    if (!filesystem_get_fullpath(root, fullm_path, sizeof(fullm_path) / sizeof(char), path)) {
        return 0;
    }

    return llm_dir_make_path(fullm_path);
}


void filesystem_listdir(stringid64_t root,
                        const char *path,
                        const char *filter,
                        string_array_t *files,
                        struct allocator *allocator) {

    char fullm_path[MAX_PATH_LEN] = {0};
    if (!filesystem_get_fullpath(root, fullm_path, sizeof(fullm_path) / sizeof(char), path)) {
        return;
    }

    llm_dir_list(fullm_path, 1, files, allocator);
}

void filesystem_listdir_free(string_array_t *files, struct allocator *allocator) {
    llm_dir_list_free(files, allocator);
}


time_t filesystem_get_file_mtime(stringid64_t root, const char *path) {
    char fullm_path[MAX_PATH_LEN] = {0};
    if (!filesystem_get_fullpath(root, fullm_path, sizeof(fullm_path) / sizeof(char), path)) {
        return 0;
    }

    return llm_file_mtime(fullm_path);
}
