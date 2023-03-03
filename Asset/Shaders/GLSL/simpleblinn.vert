
#version 330

in vec3 my_Position;
in vec4 my_Color;

uniform mat4 matWorld;
uniform mat4 matViewProj;

uniform vec4 lightPos;
uniform vec4 eyePos;

out vec4 pos;
out vec4 color;

void main()
{
	mat4 world = matWorld;
	vec4 wpos = world * vec4(my_Position, 1.0);

	gl_Position = matViewProj * wpos;

	pos = wpos;
    color = my_Color;
}
