#version 420 core

in vec3 uv_f;
in vec3 pos_f;
in vec3 vertex_color_f;
in vec3 normal_f;
in vec3 to_camera;

out vec4 color_out;

uniform vec3 cam_pos;

void main(void){

    // calculate normal
    vec3 n = normalize(normal_f);
    vec3 tc = normalize(pos_f - cam_pos);
    float f = .25*(dot(n, vec3(0,1,0))+1) + .25*(dot(n, -tc)+1);

    // blend color
    color_out.rgb = vertex_color_f.rgb * f;
    color_out.a = 1;

}
