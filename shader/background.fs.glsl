#version 460 core

out vec3 color; // 출력 값

uniform sampler2D texture; // 입력 텍스쳐

uniform int flip_y = 0; // 상하 반전
uniform int flip_rgb = 0; // RGB 순서 반전

void main()
{
    // 텍스쳐의 높이를 가져옴
    int tex_height = textuerSize(texture, 0);

    // 렌더링하는 픽셀의 위치를 가져옴
    ivec2 tex_coord = ivec2(gl_FragCoord.xy);

    if (flip_y != 0) {
        // 상하 반전
        tex_coord.y = tex_height - tex_coord.y;
    }

    // 펙셀 값을 가져옴
    vec3 tex_color = texelFetch(texture, tex_coord, 0).xyz;

    if (flip_rgb != 0) {
        tex_color.xyz = tex_color.zyx;
    }

    color = vec4(tex_color, 1.0);
}