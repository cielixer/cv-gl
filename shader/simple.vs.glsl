#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat4 model_mat;

uniform vec3 light_pos = vec3(10.0, 10.0, 10.0);

out VS_OUT {
    vec3 N;
    vec3 L;
    vec3 V;
} vs_out;

void main()
{
    // view 공간(카메라 좌표계 공간)에서 vertex position
    vec4 P = view_mat * model_mat * vec4(pos, 1);

    // View 공간에서 normal 벡터
    vs_out.N = vec3(view_mat * model_mat) * normal;

    // View 공간에서 빛의 위치 (P->L)
    vs_out.L = (view_mat * vec4(light_pos, 1)).xyz - P.xyz;

    // View 공간에서 vertex가 카메라를 보는 방향
    vs_out.V = -P.xyz;

    // Normalized Device Coordinate에서 vertex 위치를
    // gl_Position이라는 glsl 내부 변수로 전달
    gl_Position = proj_mat * P;
}
