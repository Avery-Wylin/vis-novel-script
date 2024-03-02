#version 420 core

in vec2 uv_f;
in vec2 pos_f;
in float char_num;

out vec4 color_out;
uniform vec3 object_color;

layout(binding = 0) uniform sampler2D texture0;

void main(void){

    vec4 tex = texture(texture0, uv_f);
    if(tex.r < 0.5){
        discard;
    }
    color_out.rgb = object_color;
    color_out.a = 1;
}
