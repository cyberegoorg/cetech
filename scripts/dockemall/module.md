# Modules
{% for module in modules %}
* [{{ module.name }}](#{{ module.name|replace(".", "")|lower }})
{% endfor %}

{% for module in modules %}
## {{ module.name }}
{{ module.comments }}

{% set functions = module.fce %}

#### Methods
{% for fce in functions %}
* [{{ fce.name }}({% for arg in fce.args %}{{ arg }}{% if not loop.last %}, {%endif%}{% endfor %})](#{{ fce.name|replace(".", "")|lower }})
{% endfor %}

{% for fce in functions %}

--------------------------------------------------------------------------------

#### {{ fce.name }}
```lua
function {{ fce.name }}({% for arg in fce.args %}{{ arg }}{% if not loop.last %}, {%endif%}{% endfor %}) end
```

{{ fce.comments }}

{% if fce.args|length > 0 %}
#### Arguments 
{% for arg in fce.args %}
{% if arg in fce.args_dsc %}
* `{{ arg }}` : **{{ fce.args_dsc[arg].type }}** - {{ fce.args_dsc[arg].dsc }}
{% else %}
* `{{ arg }}`
{% endif %}
{% endfor %}
{% endif %}

{% if fce.return_dsc %}
#### Return 
* **{{fce.return_dsc.type}}** - {{fce.return_dsc.dsc}}
{% endif %}
{% endfor %}

{% endfor %}