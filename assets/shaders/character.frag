#version 420 core

in vec3 pos_f;
in vec3 vertex_color_f;
in vec3 normal_f;
in vec3 to_camera;
in vec2 uv_f;

uniform vec3 cam_pos;
layout(binding = 0) uniform sampler2D tex;

out vec4 color_out;

void main(void){

    // calculate normal
    vec3 n = normalize(normal_f);
    vec3 tc = normalize(pos_f - cam_pos);
    float f = .25*(dot(n, vec3(0,1,0))+1) + .25*(dot(n, -tc)+1);

    // blend color
    color_out = texture(tex, uv_f);
    if(color_out.a <.01 || uv_f.x < 0 || uv_f.y < 0 || uv_f.x > 1 || uv_f.y > 1)
        discard;
    color_out *= vec4(f,f,f,1);
}

