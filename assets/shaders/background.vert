#version 430 core

in vec2 pos;

out vec2 pos_f;

void main(void){
    gl_Position = vec4(pos.xy,0,1);
    pos_f = (pos+1)*.5;
}


