#version 420 core

in vec2 uv_f;
in vec3 pos_f;

out vec4 color_out;
layout(binding = 0) uniform sampler2D tex;

void main(void){

    color_out = texture(tex, uv_f.xy);
//     color_out.rg = uv_f;
//     color_out.ba = vec2(0,1);

}
