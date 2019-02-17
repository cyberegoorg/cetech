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
#include <celib/containers/buffer.h>
#include <celib/os/path.h>
#include <celib/os/vio.h>

#define SEED 0

#define HEADER \
    "<meta charset=\"utf-8\" emacsmode=\"-*- markdown -*-\">\n"

//#define HEADER \
//  "<meta charset=\"utf-8\" emacsmode=\"-*- markdown -*-\"><link rel=\"stylesheet\" href=\"https://casual-effects.com/markdeep/latest/apidoc.css?\">\n"

#define FOOTER \
    "<!-- Markdeep: -->\n" \
    "<style class=\"fallback\">body{visibility:hidden;white-space:pre;font-family:monospace}</style>\n"\
    "<script src=\"../markdeep.min.js\"></script>\n"\
    "<script>window.alreadyProcessedMarkdeep|| (document.body.style.visibility=\"visible\");</script>\n"

const char *source_dir = "./src";
const char *build_dir = "./docs/gen";

void process_file(void *data) {
    const char *filename = data;

    ce_log_a0->info("hash", "Process file: %s", filename);

    ce_vio_t0 *file = ce_os_vio_a0->from_file(filename, VIO_OPEN_READ);
    uint64_t size = file->vt->size(file->inst);
    char *input_data = CE_ALLOC(ce_memory_a0->system, char, size + 1);
    file->vt->read(file->inst, input_data, 1, size);
    ce_os_vio_a0->close(file);

    input_data[size] = '\0';


    char *c = input_data;
    char *end = &input_data[size];

    char *output = NULL;
    char *comment_buffer = NULL;
    char *code_buffer = NULL;

    char *struct_buffer = NULL;
    char *struct_code_buffer = NULL;
    char *struct_comment_buffer = NULL;

    bool struct_parse = false;
    bool fce_parse = false;

    ce_buffer_printf(&output, ce_memory_a0->system, "%s", HEADER);

    while (c < end) {
        char line_buff[1024] = {};

        if (!sscanf(c, "%[^\n]s", line_buff)) {
            const uint32_t comment_n = ce_buffer_size(comment_buffer);
            if (comment_n && ((!struct_parse && !fce_parse))) {
                ce_buffer_printf(&output, ce_memory_a0->system, "%s\n",
                                 comment_buffer);

                ce_buffer_clear(comment_buffer);
            }

            ++c;
            continue;
        }

        size_t line_len = strlen(line_buff) + 1;

        char tmp_buffer[128] = {};
        char tmp_buffer2[128] = {};

        char *strit = NULL;
        if ((strit = strstr(line_buff, "//")) != 0) {
            ce_buffer_printf(
                    (struct_parse ? &struct_comment_buffer : &comment_buffer),
                    ce_memory_a0->system,
                    "%s\n", strit + 3);

            c += line_len;

            continue;

        } else if ((sscanf(line_buff, "struct %s {", tmp_buffer)) != 0) {

            if (strrchr(line_buff, '{') == NULL) {
                c += line_len;
                continue;
            }

            ce_buffer_printf(&output, ce_memory_a0->system,
                             "\n## %s\n", tmp_buffer);

            ce_buffer_printf(&code_buffer, ce_memory_a0->system,
                             "%s\n", line_buff);
            struct_parse = true;

            c += line_len;
            continue;
        } else if ((sscanf(line_buff, "static inline %s %[^(]s",
                           tmp_buffer, tmp_buffer2)) != 0) {
            if (strrchr(line_buff, ';') != NULL) {
                c += line_len;
                continue;
            }


            const char *fce_name = tmp_buffer2 + (tmp_buffer2[0] == '*');

            ce_buffer_printf(&output, ce_memory_a0->system,
                             "\n## %s\n", fce_name);

            ce_buffer_printf(&code_buffer, ce_memory_a0->system,
                             "%s\n", line_buff);

            if (strrchr(line_buff, ')') == NULL) {
                fce_parse = true;
            }

            c += line_len;
            continue;

        } else if (!fce_parse
                   && struct_parse
                   && (strstr(line_buff, ")(") != NULL)) {

            if (strrchr(line_buff, ',') != NULL) {
                fce_parse = true;
            }

            ce_buffer_printf(&struct_code_buffer, ce_memory_a0->system,
                             "%s\n", line_buff);

            ce_buffer_printf(&code_buffer, ce_memory_a0->system,
                             "%s\n", line_buff);

            char *begin = strstr(line_buff, "(");
            if (begin) {
                begin += 2;
            }

            char *end = strstr(begin, ")");
            if (end) {
                *end = 0;
            }

            const char *fce_name = begin + (begin[0] == '*');

            ce_buffer_printf(&struct_buffer, ce_memory_a0->system,
                             "\n### %s\n", fce_name);


            goto flush_fce;

        } else if (strstr(line_buff, "#define") != NULL) {
            char token_buffer[1024] = {};
            memcpy(token_buffer, line_buff, CE_ARRAY_LEN(line_buff));

            char *it = strtok(token_buffer, " ");
            it = strtok(NULL, " ");

            char *bracket = strrchr(it, '(');
            if (bracket != NULL) {
                *bracket = 0;
            }

            char name[128] = {};
            strncpy(name, it, CE_ARRAY_LEN(name));

            it = strtok(NULL, " ");
            if ((it != NULL) && (*it != '\n')) {
                ce_buffer_printf(&output, ce_memory_a0->system,
                                 "\n## %s\n", name);

                char *r = strrchr(line_buff, '\\');
                if (r) {
                    *r = 0;
                }

                ce_buffer_printf(&code_buffer, ce_memory_a0->system,
                                 "%s\n", line_buff);
            }
        }

        if (fce_parse) {
            char **b_code = struct_parse ? &struct_code_buffer
                                         : &code_buffer;

            char **b_com = struct_parse ? &struct_comment_buffer
                                        : &comment_buffer;

            ce_buffer_printf(b_code, ce_memory_a0->system, "%s\n", line_buff);

            if ((strstr(line_buff, ")") != NULL)) {
                if (!struct_parse) {
                    if (strstr(line_buff, ";") != NULL) {
                        ce_buffer_clear(*b_com);
                        ce_buffer_clear(*b_code);
                    }
                }

                fce_parse = false;
            }

        }

        if (struct_parse) {
            ce_buffer_printf(&code_buffer, ce_memory_a0->system,
                             "%s\n", line_buff);
        }

        flush_fce:
        if (!fce_parse) {
            char **b_output = (struct_parse ? &struct_buffer : &output);
            char **b_code = (struct_parse ? &struct_code_buffer : &code_buffer);
            char **b_comment = (struct_parse ? &struct_comment_buffer
                                             : &comment_buffer);

            const uint32_t comment_n = ce_buffer_size(*b_comment);
            const uint32_t readbuf_n = ce_buffer_size(*b_code);
            if (readbuf_n) {
                ce_buffer_printf(
                        b_output, ce_memory_a0->system,
                        "~~~~~~~~~~~~~~~~~~~~\n"
                        "%s\n"
                        "~~~~~~~~~~~~~~~~~~~~\n",
                        *b_code);

                ce_buffer_clear(*b_code);
            }

            if (comment_n) {
                ce_buffer_printf(b_output, ce_memory_a0->system, "%s\n",
                                 *b_comment);

                ce_buffer_clear(*b_comment);
            }

            if (struct_parse) {
                if (*c == '}') {
                    struct_parse = false;

                    goto flush_fce;
                }
            } else {
                const uint32_t n = ce_buffer_size(struct_buffer);
                if (n) {
                    ce_buffer_printf(&output, ce_memory_a0->system,
                                     "%s\n", struct_buffer);
                    ce_buffer_clear(struct_buffer);
                }
            }
        }

        c += line_len;
    }

    ce_buffer_printf(&output, ce_memory_a0->system, "%s", FOOTER);

    CE_FREE(ce_memory_a0->system, input_data);

    char basename[128];
    char output_filename[128];
    ce_os_path_a0->basename(filename, basename);
    snprintf(output_filename, CE_ARRAY_LEN(output_filename),
             "%s/%s.md.html", build_dir, basename);

    file = ce_os_vio_a0->from_file(output_filename, VIO_OPEN_WRITE);
    file->vt->write(file->inst, output, ce_array_size(output), 1);
    ce_os_vio_a0->close(file);

    ce_array_free(output, ce_memory_a0->system);
}

void print_usage() {
    ce_log_a0->info(
            "doc", "%s",

            "usage: doc --build BUILD_DIR --source SOURCE_DIR\n"
            "\n"
            "  Find *.h and *.inl files in SOURCE_DIR and generate doc to BUILD_DIR\n"
            "  Ignore all 'private' folders.\n"
            "\n"
            "    --build BUILD_DIR    - Documentation output dir\n"
            "    --source SOURCE_DIR  - Source dir\n"
            "    -h,--help            - Print this help\n"
    );
}

int main(int argc,
         const char **argv) {

    bool printusage = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--source") == 0) {
            source_dir = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "--build") == 0) {
            build_dir = argv[i + 1];
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

    ce_alloc_t0 *a = ce_memory_a0->system;
    ce_log_a0->register_handler(ce_log_a0->stdout_handler, NULL);

    if (printusage) {
        print_usage();
        return 1;
    }

    ce_init();

    ce_os_path_a0->make_path(build_dir);

    char **files;
    uint32_t files_count;

    const char *filter[] = {
//            "*/hash.inl",
//            "*/array.inl",
//            "*/os.h",
//            "*/fmath.inl",
            "*.inl", "*.h"
    };
    ce_os_path_a0->list(source_dir, CE_ARR_ARG(filter),
                         1, 0, &files, &files_count, a);

    ce_task_item_t0 tasks[files_count];

    ce_task_counter_t0 *counter = NULL;

    uint32_t add_it = 0;
    for (uint32_t i = 0; i < files_count; ++i) {
        if (strstr(files[i], "/private") != NULL) {
            continue;
        }

        tasks[add_it].data = files[i];
        tasks[add_it].work = process_file;

        ++add_it;
    }


    ce_task_a0->add(tasks, add_it, &counter);
    ce_task_a0->wait_for_counter(counter, 0);

    ce_shutdown();
}
