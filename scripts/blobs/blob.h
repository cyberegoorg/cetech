#ifndef {{ guard_prefix }}_{{ name|upper }}_BLOB_H
#define {{ guard_prefix }}_{{ name|upper }}_BLOB_H

#include <stdint.h>
{% for include in includes %}
#include {{ include }}
{% endfor %}

namespace {{name}}_blob {

    typedef struct {
        uint32_t blob_version;

        {% for k, v in elements.items() %}
            {% if v.is_array %}
        // {{ v.full_type }} {{ v.name }}[{{ v.count }}];
            {% else %}
        {{ v.type }} {{ v.name }};
            {% endif %}
        {% endfor %}
    } blob_t;

    inline const blob_t* get(void* data) {
        return (blob_t*)(data);
    }

    inline const uint32_t blob_size(const blob_t* blob) {
        return (
            sizeof(blob_t) +
{% for k, v in elements.items() %}
    {% if v.is_array %}
        {% if v.type_count == None %}
            {% if v.count != "*" %}
            ( sizeof({{v.type}}) * blob->{{v.count}} ) +
            {% endif %}
        {% else %}
            ( sizeof({{v.type}}) * blob->{{v.count}} * {{v.type_count}}) +
        {% endif %}
    {% endif %}
{% endfor %}
        0);
    }

{% for k, v in elements.items() %}
    {% if v.is_array %}
        {% if v.is_first_array %}
    inline {{v.type}}* {{ v.name }}(const blob_t* blob) {
        return ({{v.type}}*)((blob) + 1);
    }
        {% else %}
            {% if v.prev_item.type_count != None %}
    inline {{v.type}}* {{ v.name }}(const blob_t* blob) {
        return (({{v.type}}*) ({{ v.prev_item.name }}(blob) + (blob->{{ v.prev_item.count}}*{{v.prev_item.type_count}})));
    }
            {% else %}
    inline {{v.type}}* {{ v.name }}(const blob_t* blob) {
        return (({{v.type}}*) ({{ v.prev_item.name }}(blob) + (blob->{{ v.prev_item.count}})));
    }
            {% endif %}
        {% endif %}
    {% else %}
    inline {{v.type}} {{ v.name }}(const blob_t* blob) {
        return blob->{{ v.name}};
    }
    {% endif %}
{% endfor %}

}

#endif // {{ guard_prefix }}_{{ name|upper }}_BLOB_H
