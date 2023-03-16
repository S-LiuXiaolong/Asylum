
#version 430

in vec3 my_Position;
in vec3 my_Normal;
in vec3 my_Color;

uniform mat4 matWorld;
uniform mat4 matWorldInv;
uniform mat4 matViewProj;

uniform vec4 eyePos;

out vec3 wnorm;
out vec3 vdir;
out vec3 color;

void main()
{
	vec4 wpos = matWorld * vec4(my_Position, 1);

	vdir = eyePos.xyz - wpos.xyz;
	wnorm = (vec4(my_Normal, 0) * matWorldInv).xyz;
	color = my_Color;

	gl_Position = matViewProj * wpos;
}
