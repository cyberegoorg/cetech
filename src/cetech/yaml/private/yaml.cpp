#include <string>
#include <cstring>
#include <cetech/macros.h>
#include <cetech/os/errors.h>

extern "C" {
#include <cetech/yaml/yaml.h>
}

#include "include/yaml-cpp/yaml.h"

#define YAML_EX_BEGIN try {
#define YAML_EX_END \
    } catch (const YAML::Exception& e) {\
        ::abort();\
    }

#define YAML_EX_SCOPE(body) \
    YAML_EX_BEGIN\
    body\
    YAML_EX_END

struct yamlcpp_handler {
    YAML::Node nodes[4096];
    char used[4096];

    yamlcpp_handler() {
        memset(used, 0, CETECH_ARRAY_LEN(used));
    }
};


static inline void _yaml_merge_internal(YAML::Node &root,
                                        const YAML::Node &parent) {
    for (YAML::const_iterator it = parent.begin(); it != parent.end(); ++it) {
        auto str = it->first.as<std::string>();
        auto node = root[str];

        if (!node) {
            root[str] = it->second;
        } else {
            if (it->second.IsMap()) {
                _yaml_merge_internal(node, it->second);
            }
        }
    }
}

static yaml_node_t new_node(yaml_document_t handler,
                            const YAML::Node &node) {
    struct yamlcpp_handler *yaml_handler = (yamlcpp_handler *) handler.d;

    for (uint32_t i = 1; i < CETECH_ARRAY_LEN(yaml_handler->nodes); ++i) {
        if (yaml_handler->used[i]) {
            continue;
        }

        yaml_handler->nodes[i] = YAML::Clone(node);
        yaml_handler->used[i] = 1;

        return (yaml_node_t) {.doc = handler, .idx = i};
    }

    CETECH_ASSERT("yaml", false);
    return (yaml_node_t) {};
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
        return (yaml_node_t) {};
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
    if (nh == NULL) {
        return;
    }

    nh->used[node.idx] = 0;
}

extern "C" void
yaml_node_foreach_dict(yaml_node_t node,
                       yaml_foreach_map_clb_t foreach_clb,
                       void *data) {

    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    auto& cpp_node = nh->nodes[node.idx];

    for (YAML::const_iterator it = cpp_node.begin();
         it != cpp_node.end(); ++it) {
        yaml_node_t key = new_node(node.doc, it->first);
        yaml_node_t value = new_node(node.doc, it->second);

        foreach_clb(key, value, data);

        // TODO: ?????????
        yaml_node_free(key);
        //yaml_node_free(value);
    }
}

extern "C"
void yaml_node_foreach_seq(yaml_node_t node,
                           yaml_foreach_seq_clb_t foreach_clb,
                           void *data) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    auto cpp_node = nh->nodes[node.idx];

    for (size_t i = 0; i < cpp_node.size(); ++i) {
        yaml_node_t value = new_node(node.doc, cpp_node[i]);
        foreach_clb(i, value, data);
        yaml_node_free(value);
    }
}

extern "C" int yaml_as_string(yaml_node_t node,
                              char *output,
                              size_t max_len) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    YAML_EX_SCOPE({
                      return snprintf(output, max_len, "%s",
                                      nh->nodes[node.idx].as<std::string>().c_str());
                  })
}

extern "C" void yaml_merge(yaml_node_t root,
                           yaml_node_t parent) {

    yamlcpp_handler *root_h = (yamlcpp_handler *) root.doc.d;
    yamlcpp_handler *parent_h = (yamlcpp_handler *) parent.doc.d;

    auto root_node = root_h->nodes[root.idx];
    auto parent_node = parent_h->nodes[parent.idx];
    _yaml_merge_internal(root_node, parent_node);
}

extern "C" int yaml_as_bool(yaml_node_t node) {
    yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;

    YAML_EX_SCOPE({ return nh->nodes[node.idx].as<bool>(); })
}

void yaml_as_vec3(yaml_node_t body,
                  float *value) {
    CETECH_ASSERT("yaml", yaml_is_valid(body));

    yaml_node_t x = yaml_get_seq_node(body, 0);
    CETECH_ASSERT("yaml", yaml_is_valid(x));
    value[0] = yaml_as_float(x);
    yaml_node_free(x);


    yaml_node_t y = yaml_get_seq_node(body, 1);
    CETECH_ASSERT("yaml", yaml_is_valid(y));
    value[1] = yaml_as_float(y);
    yaml_node_free(y);


    yaml_node_t z = yaml_get_seq_node(body, 2);
    CETECH_ASSERT("yaml", yaml_is_valid(z));
    value[2] = yaml_as_float(z);
    yaml_node_free(z);
}

void yaml_as_vec4(yaml_node_t body,
                  float *value) {
    CETECH_ASSERT("yaml", yaml_is_valid(body));

    yaml_node_t x = yaml_get_seq_node(body, 0);
    CETECH_ASSERT("yaml", yaml_is_valid(x));
    value[0] = yaml_as_float(x);
    yaml_node_free(x);


    yaml_node_t y = yaml_get_seq_node(body, 1);
    CETECH_ASSERT("yaml", yaml_is_valid(y));
    value[1] = yaml_as_float(y);
    yaml_node_free(y);


    yaml_node_t z = yaml_get_seq_node(body, 2);
    CETECH_ASSERT("yaml", yaml_is_valid(z));
    value[2] = yaml_as_float(z);
    yaml_node_free(z);

    yaml_node_t w = yaml_get_seq_node(body, 3);
    CETECH_ASSERT("yaml", yaml_is_valid(w));
    value[3] = yaml_as_float(w);
    yaml_node_free(w);

}

void yaml_as_mat44(yaml_node_t body,
                   float *value) {
    CETECH_ASSERT("yaml", yaml_is_valid(body));

    for (uint8_t i = 0; i < 16; ++i) {
        yaml_node_t x = yaml_get_seq_node(body, 0);
        CETECH_ASSERT("yaml", yaml_is_valid(x));
        value[i] = yaml_as_float(x);
        yaml_node_free(x);
    }

}

//mat33f_t yaml_as_mat33f_t(yaml_node_t body) {
//    CETECH_ASSERT("yaml", yaml_is_valid(body));
//
//    mat33f_t m = {};
//
//    yaml_node_t x = yaml_get_seq_node(body, 0);
//    CETECH_ASSERT("yaml", yaml_is_valid(x));
//    m.x = yaml_as_vec3f_t(x);
//    yaml_node_free(x);
//
//
//    yaml_node_t y = yaml_get_seq_node(body, 1);
//    CETECH_ASSERT("yaml", yaml_is_valid(y));
//    m.y = yaml_as_vec3f_t(y);
//    yaml_node_free(y);
//
//
//    yaml_node_t z = yaml_get_seq_node(body, 2);
//    CETECH_ASSERT("yaml", yaml_is_valid(z));
//    m.z = yaml_as_vec3f_t(z);
//    yaml_node_free(z);
//
//    return m;
//}

#define YAML_NODE_AS_DECL(type)\
    extern "C" type yaml_as_##type(yaml_node_t node) {\
        yamlcpp_handler *nh = (yamlcpp_handler *) node.doc.d;\
        YAML_EX_SCOPE({ return nh->nodes[node.idx].as<type>(); })\
    }\


YAML_NODE_AS_DECL(int);
YAML_NODE_AS_DECL(float);

