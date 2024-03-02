#version 420 core

in vec3 pos;
in vec3 vertex_color;

out vec3 pos_f;
out vec3 vertex_color_f;

uniform mat4 camera;

void main(void){
    gl_Position = camera * vec4(pos,1);
    pos_f = pos;
    vertex_color_f = vertex_color;
}
