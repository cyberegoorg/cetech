#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <corelib/core.h>
#include <corelib/log.h>
#include <corelib/os.h>
#include <corelib/memory.h>
#include <corelib/allocator.h>
#include <corelib/murmur_hash.inl>


#define SEED 0

#define MACRO_BEGIN "CT_ID64_0(\""
#define MACRO_LEN 11
#define MACRO_TEMPLATE "CT_ID64_0(\"%.*s\", 0x%"PRIx64"ULL)"

void process_file(const char *filename,
                  struct ct_alloc *a) {
    ct_log_a0->info("hash", "Process file: %s", filename);

    struct ct_vio *file = ct_os_a0->vio->from_file(filename, VIO_OPEN_READ);
    uint64_t size = file->size(file);
    char *data = CT_ALLOC(a, char, size + 1);
    file->read(file, data, 1, size);
    file->close(file);

    data[size] = '\0';

    uint32_t c = 0;
    char buffer[1024] = {0};
    bool change = false;
    while (c < size) {
        if (data[c] == '\n') {
            ++c;
            continue;
        } else if (data[c] != 'C') {
            ++c;
            continue;
        }


        if (c + MACRO_LEN > size) {
            break;
        }

        if (memcmp(data + c, MACRO_BEGIN, MACRO_LEN)) {
            ++c;
            continue;
        }


        const char *macro_s = data + c;
        const char *string_s = macro_s + MACRO_LEN;
        const char *string_e = (char *) memchr(string_s,
                                               '"', size - c - MACRO_LEN);
        if (!string_e || string_e - string_s > 512) {
            ++c;
            continue;
        }

        const char *macro_e = (char *) memchr(string_e, ')',
                                              (size_t) (data + (int64_t) size -
                                                        string_e)) + 1;

        const uint32_t string_len = (uint32_t) (string_e - string_s);
        const uint32_t macro_len = (uint32_t) (macro_e - macro_s);

        const uint64_t hash = ct_hash_murmur2_64(string_s, string_len, SEED);
        sprintf(buffer, MACRO_TEMPLATE,
                string_len, string_s, hash);

        const uint32_t buffer_len = (uint32_t) strlen(buffer);
        if (c + buffer_len > size || 0 == memcmp(macro_s, buffer, buffer_len)) {
            c += buffer_len;
            continue;
        }

        change = true;
        const int64_t extra = (int64_t) buffer_len - (int64_t) macro_len;
        data = a->call->reallocate(a, data,
                                   (uint64_t) ((int64_t) size + extra + 1), 0,
                                   __FILE__, __LINE__);
        memmove(data + c + extra, data + c, size + 1 - c);
        memcpy(data + c , buffer, buffer_len);
        size = (uint64_t) ((int64_t) size + extra);
        c += buffer_len;
    }

    if (change) {
        file = ct_os_a0->vio->from_file(filename, VIO_OPEN_WRITE);
        file->write(file, data, size, 1);
        file->close(file);
    }

    CT_FREE(a, data);
}

int main(int argc,
         const char **argv) {
    struct ct_alloc *a = ct_memory_a0->system;
    ct_log_a0->register_handler(ct_log_a0->stdout_handler, NULL);

    ct_corelib_init();

    char **files;
    uint32_t files_count;

    const char *filter[] = {"*.c", "*.h", "*.inl", "*.cpp"};
    ct_os_a0->path->list("./src", CETECH_ARR_ARG(filter),
                         1, 0, &files, &files_count, a);

    for (uint32_t i = 0; i < files_count; ++i) {
        process_file(files[i], a);
    }

    ct_os_a0->path->list_free(files, files_count, a);

    ct_corelib_shutdown();
}
