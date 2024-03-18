#version 420 core

in vec3 pos;
in vec2 uv;
in vec3 vertex_color;

out vec3 pos_f;
out vec2 uv_f;
out vec3 vertex_color_f;

uniform mat4 camera;
uniform mat4 proj;
uniform mat4 transform;


void main(void){
    gl_Position = proj * camera * transform * vec4(pos,1);
    pos_f = pos;
    vertex_color_f = vertex_color;
    uv_f = uv;
}
