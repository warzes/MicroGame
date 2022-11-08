<VERTEX>
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vCol;

uniform mat4 MVP;

out vec3 color;

void main()
{
	gl_Position = MVP * vec4(vPos, 0.0, 1.0);
	color = vCol;
}
</VERTEX>
<FRAGMENT>
#version 330 core

in vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
</FRAGMENT>