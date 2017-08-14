$input v_texcoord0

#include "common.sh"

SAMPLER2D(s_input_texture, 0);

void main()
{
    float gray = dot(texture2D(s_input_texture,  v_texcoord0), vec4(0.299, 0.587, 0.114, 1.0));
    gl_FragData[0] = vec4(gray, gray, gray, 1.0f);

//    gl_FragData[0] = texture2D(s_input_texture,  v_texcoord0);
//    gl_FragData[0] = texture2D(s_input_texture,  v_texcoord0) * vec4(0.0, 1.0, 0.0, 1.0);
//    gl_FragData[0] = texture2D(s_input_texture,  v_texcoord0) * vec4(0.0, 1.0, 0.0, 1.0);
//    gl_FragData[0] = vec4(v_texcoord0, 0.0f, 1.0f);
}