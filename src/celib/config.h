#ifndef CE_CONFIG_H
#define CE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

#define CE_CONFIG_API \
    CE_ID64_0("ce_config_a0", 0xd75a5088e70ad1bbULL)

#define CE_CONFIG_TYPE \
    CE_ID64_0("config", 0x82645835e6b73232ULL)

#define CE_CONFIG_VARIABLE_REF \
    CE_ID64_0("config_variable_ref", 0xcba63cf174888fe5ULL)

#define CE_CONFIG_VARIABLE_INT \
    CE_ID64_0("config_variable_int", 0x6f579c4309784aadULL)

#define CE_CONFIG_VARIABLE_STR \
    CE_ID64_0("config_variable_str", 0x28475786507229b9ULL)

#define CE_CONFIG_VARIABLES \
    CE_ID64_0("variables", 0x4fb1ab3fd540bd03ULL)

#define CE_CONFIG_NAME \
    CE_ID64_0("name", 0xd4c943cba60c270bULL)

#define CE_CONFIG_VALUE \
    CE_ID64_0("value", 0x920b430f38928dc9ULL)

#define CE_CONFIG_VARIABLE_REF \
    CE_ID64_0("config_variable_ref", 0xcba63cf174888fe5ULL)

#define CE_CONFIG_VARIABLE_INT \
    CE_ID64_0("config_variable_int", 0x6f579c4309784aadULL)

#define CE_CONFIG_VARIABLE_STR \
    CE_ID64_0("config_variable_str", 0x28475786507229b9ULL)

typedef struct ce_alloc_t0 ce_alloc_t0;
typedef struct ce_cdb_t0 ce_cdb_t0;

struct ce_config_a0 {
    //! Parse commandline arguments.
    int (*parse_args)(int argc,
                      const char **argv);

    //! Load config from file.
    int (*from_file)(ce_cdb_t0 db,
                     const char *path,
                     ce_alloc_t0 *alloc);

    bool (*exist)(uint64_t name);

    float (*read_float)(uint64_t name,
                        float defaultt);

    uint64_t (*read_uint)(uint64_t name,
                          uint64_t defaultt);

    const char *(*read_str)(uint64_t name,
                            const char *defaultt);

    void (*set_float)(uint64_t name,
                      float value);

    void (*set_uint)(uint64_t name,
                     uint64_t value);

    void (*set_str)(uint64_t name,
                    const char *value);

    //! Dump all variables to log
    void (*log_all)();
};

CE_MODULE(ce_config_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_CONFIG_H
