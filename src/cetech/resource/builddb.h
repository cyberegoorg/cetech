#ifndef CETECH_BUILDDB_H
#define CETECH_BUILDDB_H


struct ct_builddb_a0 {
    void (*put_file)(const char *filename,
                     time_t mtime);

    void (*set_file_depend)(const char *filename,
                            const char *depend_on);

    int (*get_filename_by_hash)(char *filename,
                                size_t max_len,
                                const char *hash);

    int (*get_filename_type_name)(char *filename,
                                  size_t max_len,
                                  uint64_t type,
                                  uint64_t  name);

    void (*add_dependency)(const char *who_filename,
                         const char *depend_on_filename);

    int (*need_compile)(const char *filename);
};

CE_MODULE(ct_builddb_a0);

#endif //CETECH_BUILDDB_H
