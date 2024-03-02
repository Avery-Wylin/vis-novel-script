#version 420 core

in vec3 pos_f;
in vec3 vertex_color_f;

out vec4 color_out;

void main(void){

    color_out.a = 1;
    color_out.xyz = vertex_color_f;
}
