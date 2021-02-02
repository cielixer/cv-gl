#version 460 core

void main()
{
    vec3 vertices[4] = {
        vec3(-1.0, 1.0, 0.0),
        vec3(1.0, 1.0, 0.0),
        vec3(1.0, -1.0, 0.0),
        vec3(-1.0, -1.0, 0.0)
    };

    gl_Position.xyz = vertices[gl_VertexID];
    gl_Position.w = 1.0;
}