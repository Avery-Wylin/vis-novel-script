#version 430 core
in vec2 pos_f;
out vec4 color_out;

layout(location = 1) uniform float time;
layout(location = 2) uniform vec3 color_1;
layout(location = 3) uniform vec3 color_2;
layout(location = 4) uniform vec3 color_3;

void main(void){
    float v = floor ( floor( ((cos(6.2832*(10*pos_f.x+10*time))+1)*.025 + pos_f.y )*50 ) * .02 * 4) * .25 ;
//     v = floor((pos_f.x*pos_f.x  + cos(6.2832 * time)*.5+ pos_f.y)*10)*.1;
    color_out.rgb = mix(color_1, color_2, v);
    color_out.a = 1;
}
