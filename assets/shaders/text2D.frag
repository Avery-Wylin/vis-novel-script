#version 420 core

in vec3 uv_f;
in vec2 pos_f;
in vec4 col_f;
in float char_num;

out vec4 color_out;
uniform vec3 object_color;
uniform vec3 object_color2;
uniform uint factor;

layout(binding = 0) uniform sampler2D texture0;

void main(void){

    float v = texture(texture0, uv_f.xy).x;
    if(v < 0.5 || char_num > factor){
        discard;
    }

    // mix color using alpha
    color_out.rgb = mix(object_color, col_f.rgb, col_f.a);

    // If selected (z!=0) then override color
    color_out.rgb = uv_f.z==0?color_out.rgb:object_color2 ;
    color_out.a = 1;
}
