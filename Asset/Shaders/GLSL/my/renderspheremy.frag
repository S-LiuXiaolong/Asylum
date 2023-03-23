
#version 430

uniform vec4 lightDir;
uniform int isWireMode;

in vec3 wnorm;
in vec3 vdir;
in vec3 color;

out vec4 my_FragColor0;

void main()
{
	// my_FragColor0 = vec4(1.0, 1.0, 1.0, 0.0);

	//TODO: Modify here to display element density
	// my_FragColor0.rgb = clamp(base.rgb * d + vec3(s), vec3(0.0), vec3(1.0));
	my_FragColor0.rgb = vec3(1.0, 0, 0);
	my_FragColor0.a = 1.0;
	
}
