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

struct yamlcpp_handler {
    YAML::Node nodes[4096];
    char used[4096];
};

static yaml_node_t new_node(yaml_document_t handler,
                            const YAML::Node &node) {
    struct yamlcpp_handler *yaml_handler = (yamlcpp_handler *) handler.d;

    for (u32 i = 1; i < CE_ARRAY_LEN(yaml_handler->nodes); ++i) {
        if (yaml_handler->used[i]) {
            continue;
        }

        yaml_handler->nodes[i] = YAML::Clone(node);
        yaml_handler->used[i] = 1;

        return (yaml_node_t) {.doc = handler, .idx = i};
    }

    CE_ASSERT_MSG("yaml", false, "Node pool overflow");
}

extern "C" yaml_node_t yaml_load_str(const char *str,
                                     yaml_document_t *handler) {
    yamlcpp_handler *nh = new yamlcpp_handler;

    handler->d = nh;
    return new_node(*handler, YAML::Load(str));
}

extern "C" yaml_node_t yaml_get_node(yaml_node_t node,
                                     const char *key) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    auto cpp_node = nh->nodes[node.idx][key];
    if (!cpp_node) {
        return (yaml_node_t) {0};
    }

    return new_node(node.doc, cpp_node);
}


extern "C" yaml_node_t yaml_get_seq_node(yaml_node_t seq_node,
                                         size_t idx) {
    yamlcpp_handler *nh = (yamlcpp_handler *) seq_node.doc.d;

    return new_node(seq_node.doc, nh->nodes[seq_node.idx][idx]);
}

extern "C" enum yaml_node_type yaml_node_type(yaml_node_t node) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    static const enum yaml_node_type _types[5] = {
            [YAML::NodeType::Undefined] = YAML_TYPE_UNDEF,
            [YAML::NodeType::Null] = YAML_TYPE_NULL,
            [YAML::NodeType::Scalar] = YAML_TYPE_SCALAR,
            [YAML::NodeType::Sequence] = YAML_TYPE_SEQ,
            [YAML::NodeType::Map] = YAML_TYPE_MAP,
    };

    return _types[nh->nodes[node.idx].Type()];
}

extern "C" size_t yaml_node_size(yaml_node_t node) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;


    return nh->nodes[node.idx].size();

}

extern "C" void yaml_node_free(yaml_node_t node) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    nh->used[node.idx] = 0;
}

extern "C" void
yaml_node_foreach_dict(yaml_node_t node,
                       yaml_foreach_map_clb_t foreach_clb,
                       void *data) {

    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    auto cpp_node = nh->nodes[node.idx];

    for (YAML::const_iterator it = cpp_node.begin(); it != cpp_node.end(); ++it) {
        yaml_node_t key = new_node(node.doc, it->first);
        yaml_node_t value = new_node(node.doc, it->second);

        foreach_clb(key, value, data);

        yaml_node_free(key);
        yaml_node_free(value);
    }
}

extern "C" int yaml_node_as_string(yaml_node_t node,
                                   char *output,
                                   size_t max_len) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;


    YAML_EX_SCOPE({ return snprintf(output, max_len, "%s", nh->nodes[node.idx].as<std::string>().c_str()); })
}

extern "C" int yaml_node_as_bool(yaml_node_t node) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    YAML_EX_SCOPE({ return nh->nodes[node.idx].as<bool>(); })
}

#define YAML_NODE_AS_DECL(type)\
    extern "C" type yaml_node_as_##type(yaml_node_t node) {\
        yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;\
        YAML_EX_SCOPE({ return nh->nodes[node.idx].as<type>(); })\
    }\

YAML_NODE_AS_DECL(int);
YAML_NODE_AS_DECL(float);

