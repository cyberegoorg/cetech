$input v_texcoord0

/*
 * Copyright 2013-2014 Dario Manesku. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
 */

#include "common.sh"
SAMPLER2D(u_texColor, 0);

void main()
{
	gl_FragColor = texture2D(u_texColor, v_texcoord0) * vec4(1.0f, 1.0f, 1.0f, 1.0f);
	//gl_FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
}
