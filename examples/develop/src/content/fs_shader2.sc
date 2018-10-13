$input v_texcoord0, v_view, v_normal

#include "common.sh"

SAMPLER2D(u_texColor,
0);

uniform vec4
u_vec4;
uniform vec4
u_vec42;

void main() {
    gl_FragData[0] = texture2D(u_texColor, v_texcoord0) * u_vec4;
    //gl_FragData[0] = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    gl_FragData[0] = u_vec4;
}
