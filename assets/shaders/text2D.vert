#version 420 core

in vec2 pos;
in vec3 uv;
in vec3 vertex_color;

out vec3 uv_f;
out vec2 pos_f;
out float char_num;

uniform mat4 camera;
uniform vec3 transform;

void main(void){
    gl_Position = camera * vec4( transform.z*pos.xy+transform.xy, -0.1, 1.0);
    uv_f = uv;
    pos_f = pos;
    char_num = gl_VertexID/4;
}
