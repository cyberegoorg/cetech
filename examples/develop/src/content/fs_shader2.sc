$input v_texcoord0

/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common.sh"

SAMPLER2D(s_tex1, 0);

void main()
{
    gl_FragData[0] = texture2D(s_tex1,  v_texcoord0);
}