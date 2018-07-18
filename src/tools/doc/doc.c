#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <corelib/core.h>
#include <corelib/log.h>
#include <corelib/os.h>
#include <corelib/memory.h>
#include <corelib/allocator.h>
#include <corelib/murmur_hash.inl>
#include <corelib/hash.inl>
#include <corelib/task.h>
#include <corelib/buffer.inl>

#define SEED 0

#define HEADER "<meta charset=\"utf-8\" emacsmode=\"-*- markdown -*-\">\n"
//#define HEADER "<meta charset=\"utf-8\" emacsmode=\"-*- markdown -*-\"><link rel=\"stylesheet\" href=\"https://casual-effects.com/markdeep/latest/apidoc.css?\">\n"
#define FOOTER "<!-- Markdeep: --><style class=\"fallback\">body{visibility:hidden;white-space:pre;font-family:monospace}</style><script src=\"../markdeep.min.js\"></script></script><script>window.alreadyProcessedMarkdeep||(document.body.style.visibility=\"visible\")</script>\n"

void process_file(void *data) {
    const char *filename = data;

    if (strstr(filename, "/private") != NULL) {
        return;
    }

    ct_log_a0->info("hash", "Process file: %s", filename);

    struct ct_vio *file = ct_os_a0->vio->from_file(filename, VIO_OPEN_READ);
    uint64_t size = file->size(file);
    char *input_data = CT_ALLOC(ct_memory_a0->system, char, size + 1);
    file->read(file, input_data, 1, size);
    file->close(file);

    input_data[size] = '\0';

    char *output = NULL;

    char *c = input_data;
    char *end = &input_data[size];


    ct_buffer_printf(&output, ct_memory_a0->system, "%s", HEADER);

    char *comment_buffer = NULL;
    char *code_buffer = NULL;

    bool block_parse = false;
    bool fce_parse = false;
    while (c < end) {
        char line_buff[1024] = {0};

        if (!sscanf(c, "%[^\n]s", line_buff)) {
            ++c;
            continue;
        }

        size_t line_len = strlen(line_buff);

        char tmp_buffer[128] = {0};
        char tmp_buffer2[128] = {0};

        if ((sscanf(c, "//%[^\n]s", tmp_buffer)) != 0) {
            c += 3 + ct_buffer_printf(&comment_buffer, ct_memory_a0->system,
                                      "%s\n",
                                      tmp_buffer + (tmp_buffer[0] == ' '));

            continue;

        } else if (strcmp(line_buff, "//") == 0) {
            ct_buffer_printf(&comment_buffer, ct_memory_a0->system, "\n");

            c += 3;
            continue;

        } else if ((sscanf(c, "struct %s { \n", tmp_buffer)) != 0) {

            ct_buffer_printf(&output, ct_memory_a0->system,
                             "\n## Struct %s\n", tmp_buffer);

            ct_buffer_printf(&code_buffer, ct_memory_a0->system,
                             "%s\n", line_buff);
            block_parse = true;

            c += line_len;
            continue;
        } else if ((sscanf(c, "static inline %s %[^(]s",
                           tmp_buffer, tmp_buffer2)) != 0) {

            ct_buffer_printf(&output, ct_memory_a0->system,
                             "\n## Function  **%s**\n", tmp_buffer2);

            ct_buffer_printf(&code_buffer, ct_memory_a0->system,
                             "%s\n", line_buff);

            fce_parse = true;

            c += line_len;
            continue;
        }

        if (fce_parse) {
            ct_buffer_printf(&code_buffer, ct_memory_a0->system, "%s\n",
                             line_buff);

            if (strstr(line_buff, ")") != NULL) {
                fce_parse = false;
            }

        }

        if (block_parse) {
            ct_buffer_printf(&code_buffer, ct_memory_a0->system,
                             "%s\n", line_buff);


            if (*c == '}') {
                block_parse = false;
            }
        }


        if (!block_parse && !fce_parse) {

            const uint32_t readbuf_n = ct_buffer_size(code_buffer);
            if (readbuf_n) {
                ct_buffer_printf(
                        &output, ct_memory_a0->system,
                        "~~~~~~~~~~~~~~~~~~~~\n"
                        "%s\n"
                        "~~~~~~~~~~~~~~~~~~~~\n",
                        code_buffer);

                ct_buffer_clear(code_buffer);

                continue;
            }

            const uint32_t comment_n = ct_buffer_size(comment_buffer);
            if (comment_n) {
                ct_buffer_printf(&output, ct_memory_a0->system, "%s\n",
                                 comment_buffer);

                ct_buffer_clear(comment_buffer);

                continue;
            }
        }

        c += line_len;
    }

    ct_buffer_printf(&output, ct_memory_a0->system, "%s", FOOTER);

    CT_FREE(ct_memory_a0->system, input_data);

    char basename[128];
    char output_filename[128];
    ct_os_a0->path->basename(filename, basename);
    snprintf(output_filename, CT_ARRAY_LEN(output_filename),
             "./docs/gen/%s.md.html", basename);

    file = ct_os_a0->vio->from_file(output_filename, VIO_OPEN_WRITE);
    file->write(file, output, ct_array_size(output), 1);
    file->close(file);

    ct_array_free(output, ct_memory_a0->system);
}

int main(int argc,
         const char **argv) {
    struct ct_alloc *a = ct_memory_a0->system;
    ct_log_a0->register_handler(ct_log_a0->stdout_handler, NULL);

    ct_corelib_init();

    ct_os_a0->path->make_path("./docs/gen/");

    char **files;
    uint32_t files_count;

//    const char *filter[] = {"*/hash.inl"};
//    const char *filter[] = {"*/mouse.h"};
    const char *filter[] = {"*.inl", "*.h"};
    ct_os_a0->path->list("./src", CETECH_ARR_ARG(filter),
                         1, 0, &files, &files_count, a);

    struct ct_task_item tasks[files_count];

    struct ct_task_counter_t *counter = NULL;

    for (uint32_t i = 0; i < files_count; ++i) {
        tasks[i].data = files[i];
        tasks[i].work = process_file;
    }

    ct_task_a0->add(tasks, files_count, &counter);
    ct_task_a0->wait_for_counter(counter, 0);

    ct_corelib_shutdown();
}
