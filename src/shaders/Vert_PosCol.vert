
#version 430

uniform mat4 world;
uniform mat4 viewProj;

const uint MAX_POINT_COUNT = 20;

// Currently as a uniform, will change to texture.
uniform vec3 positions[MAX_POINT_COUNT];
uniform vec3 color = vec3(1.0, 0.0, 1.0);

out vec3 vs_out_color;

void main()
{
    gl_Position = viewProj * world * vec4(positions[gl_VertexID], 1.0);
    vs_out_color = color;
}
