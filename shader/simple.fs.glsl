#version 460 core

layout(location = 0) out vec3 color;

in VS_OUT {
    vec3 N;
    vec3 L;
    vec3 V;
} fs_in;

// ambient color
uniform vec3 ambient_albedo = vecc3(0.1);
// diffuse color
uniform vec3 diffuse_albedo = vec3(0.7, 0.7, 0.7);

void main(void)
{
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(fs_in.L);
    vec3 V = normalize(fs_in.L);

    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;

    color = ambient_albedo + diffuse;
}
