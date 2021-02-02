#version 460 core

out vec3 color;

uniform sampler2D back_tex;
uniform sampler2D model_tex;

uniform int flip_obj_y = 0;
uniform int flip_obj_rgb = 0;

void main()
{
    int tex_height = textureSize(back_tex, 0).y;

    ivec2 tex_coord = ivec2(gl_FragCoord.xy);
    ivec2 model_tex_coord = ivec2(gl_FragCoord.xy);
    if (flip_obj_y != 0) {
        model_tex_coord.y = tex_height - tex_coord.y;
    }

    vec3 back_color = texelFetch(back_tex, tex_coord, 0).rgb;
    vec4 model_color = texelFetch(model_tex, model_tex_coord, 0).rgba;

    if (flip_obj_rgb != 0) {
        model_color.rgb = model_color.bgr;
    }

    if (model_color.a != 0) {
        color = model_color.rgb;
    }
    else {
        color = back_color;
    }
}