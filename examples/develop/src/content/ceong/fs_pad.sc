$input v_view, v_normal

#include "common.sh"

uniform vec4 color;

void main() {
    gl_FragData[0] = color;
}
