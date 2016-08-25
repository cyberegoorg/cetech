#include <string>

extern "C" {
#include "celib/errors/errors.h"
#include "celib/yaml/yaml.h"
}

#include "include/yaml-cpp/yaml.h"

#define YAML_EX_BEGIN try {
#define YAML_EX_END \
    } catch (const YAML::Exception& e) {\
        log_error("yaml", "%s", e.what()); \
        ::abort();\
    }

#define YAML_EX_SCOPE(body) \
    YAML_EX_BEGIN\
    body\
    YAML_EX_END

struct yaml_handler_s {
    YAML::Node nodes[4096];
    char used[4096];
};

static size_t new_node(yaml_handler_s *handler, const YAML::Node &node) {
    for (size_t i = 0; i < 4096; ++i) {
        if (handler->used[i]) {
            continue;
        }

        handler->nodes[i] = YAML::Clone(node);
        handler->used[i] = 1;

        return i;
    }

    CE_ASSERT_MSG("yaml", false, "Node pool overflow");
}

extern "C" yaml_handler_t yaml_load_str(const char *str) {
    yaml_handler_s *nh = new yaml_handler_s;

    new_node(nh, YAML::Load(str));

    return nh;
}

extern "C" size_t yaml_get_node(yaml_handler_t handler, yaml_node_t node_idx, const char *key) {
    yaml_handler_s *nh = (yaml_handler_s *) handler;

    return new_node(nh, nh->nodes[node_idx][key]);
}

extern "C" yaml_node_t yaml_get_seq_node(yaml_handler_t handler, yaml_node_t seq_node_idx, size_t idx) {
    yaml_handler_s *nh = (yaml_handler_s *) handler;

    return new_node(nh, nh->nodes[seq_node_idx][idx]);
}

extern "C" enum yaml_node_type yaml_node_type(yaml_handler_t handler, yaml_node_t node_idx) {
    yaml_handler_s *nh = (yaml_handler_s *) handler;

    static const enum yaml_node_type _types[5] = {
            [YAML::NodeType::Undefined] = YAML_TYPE_UNDEF,
            [YAML::NodeType::Null] = YAML_TYPE_NULL,
            [YAML::NodeType::Scalar] = YAML_TYPE_SCALAR,
            [YAML::NodeType::Sequence] = YAML_TYPE_SEQ,
            [YAML::NodeType::Map] = YAML_TYPE_MAP,
    };

    return _types[nh->nodes[node_idx].Type()];
}

extern "C" void yaml_node_free(yaml_handler_t handler, yaml_node_t node_idx) {
    yaml_handler_s *nh = (yaml_handler_s *) handler;

    nh->used[node_idx] = 0;
}

#define YAML_NODE_AS_DECL(type)\
    extern "C" type yaml_node_as_##type(yaml_handler_t handler, yaml_node_t node_idx) {\
        yaml_handler_s *nh = (yaml_handler_s *) handler;\
        YAML_EX_SCOPE({ return nh->nodes[node_idx].as<type>(); })\
    }\

YAML_NODE_AS_DECL(int);
YAML_NODE_AS_DECL(bool);
YAML_NODE_AS_DECL(float);

