#version 430

layout( location = 0 ) in vec3 vs_in_pos;
layout( location = 1 ) in vec3 vs_in_norm;
layout( location = 2 ) in vec2 vs_in_tex;

out vec3 vs_out_pos;
out vec3 vs_out_norm;
out vec2 vs_out_tex;

uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 viewProj;

void main()
{
	vec4 worldPos = world * vec4(vs_in_pos, 1);
	gl_Position = viewProj * worldPos;
	vs_out_pos  = worldPos.xyz;
	vs_out_norm = (worldIT * vec4(vs_in_norm, 0)).xyz;

	vs_out_tex = vs_in_tex;
}
