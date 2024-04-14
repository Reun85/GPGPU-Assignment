#version 430

// shader külső paraméterei - most a három transzformációs mátrixot külön-külön vesszük át
uniform mat4 world;
uniform mat4 viewProj;

vec4 positions[6] = vec4[6](
	// 1. szakasz
	vec4( 0,  0, 0, 1),
	vec4( 1,  0, 0, 1),
	// 2. szakasz
	vec4( 0,  0, 0, 1),
	vec4( 0,  1, 0, 1),
	// 3. szakasz
	vec4( 0,  0, 0, 1),
	vec4( 0,  0, 1, 1)
);

vec3 colors[3] = vec3[3](
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1)
);

out vec3 vs_out_color;

void main()
{
	gl_Position = viewProj * world * positions[gl_VertexID];
	vs_out_color = colors[gl_VertexID/2];
}

