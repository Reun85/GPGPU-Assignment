#version 430


layout( location = 0 ) in vec3 vs_in_pos;


uniform mat4 world;
uniform mat4 viewProj;


void main()
{
//    gl_Position = viewProj * world * vec4(5,4,3, 1.0);
    gl_Position = viewProj * world * vec4(vs_in_pos, 1.0);
}
