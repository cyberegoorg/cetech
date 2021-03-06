$input a_position, a_normal
$output v_view, v_normal

#include "common.sh"

void main() {
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_view = mul(u_modelView, vec4(a_position, 1.0));
    v_normal = normalize(mul(u_modelView, vec4(a_normal, 0.0)).xyz);
}
