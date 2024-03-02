#version 430 core

in vec2 pos;
in vec2 uv;

out vec2 uv_f;
out vec2 pos_f;

uniform mat4 camera;    // Orthographic projection of screen
uniform vec4 transform; // position and scale
layout(binding = 0) uniform sampler2D texture0;

void main(void){
    gl_Position = camera * vec4(transform.zw*pos.xy+transform.xy, -0.1, 1.0);
    uv_f = uv;
    pos_f = ( gl_Position.xy + 1 ) / 2; // Convert so position can be used as UV for framebuffer
}


