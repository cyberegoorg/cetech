#ifndef {{ guard_prefix }}_{{ name|upper }}_BLOB_H
#define {{ guard_prefix }}_{{ name|upper }}_BLOB_H

{% for include in includes %}
#include {{ include }}
{% endfor %}

typedef struct {{name}}_blob {
    {% for k, v in elements.items() %}
        {% if v.is_array %}
    // {{ v.full_type }} {{ v.name }}[{{ v.count }}];
        {% else %}
    {{ v.type }} {{ v.name }};
        {% endif %}
    {% endfor %}
} {{name}}_blob_t;

{% for k, v in elements.items() %}
    {% if v.is_array %}
        {% if v.is_first_array %}
#define {{ name }}_blob_{{ v.name }}(r) (({{v.type}}*) ((r) + 1))
        {% else %}
            {% if v.prev_item.type_count != None %}
#define {{ name }}_blob_{{ v.name }}(r) (({{v.type}}*) ({{ name }}_blob_{{ v.prev_item.name }}(r) + ((r)->{{ v.prev_item.count}}*{{v.prev_item.type_count}})))
            {% else %}
#define {{ name }}_blob_{{ v.name }}(r) (({{v.type}}*) ({{ name }}_blob_{{ v.prev_item.name }}(r) + ((r)->{{ v.prev_item.count}})))
            {% endif %}
        {% endif %}
    {% endif %}
{% endfor %}

#endif // {{ guard_prefix }}_{{ name|upper }}_BLOB_H
