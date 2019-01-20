$input v_texcoord0, v_view, v_normal

#include "common.sh"

SAMPLER2D(u_texColor,0);

void main() {
    gl_FragData[0] = vec4(texture2D(u_texColor, v_texcoord0).xyz, 1.0f);
}
