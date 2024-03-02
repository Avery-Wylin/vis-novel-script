#version 420 core

in vec3 pos;
in vec3 uv;
in vec4 vertex_color;
in vec3 normal;

out vec3 uv_f;
out vec3 pos_f;
out vec3 vertex_color_f;
out vec3 normal_f;

uniform mat4 transform;
uniform mat4 camera;


void main(void){
    pos_f = (transform * vec4(pos,1)).xyz;
    gl_Position = camera * vec4(pos_f,1);
    uv_f = uv;
    vertex_color_f = vertex_color.xyz;
    normal_f = (transform * vec4(normal,0)).xyz;
}
