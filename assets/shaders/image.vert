#version 420 core

in vec2 pos;
in vec2 uv;

out vec2 uv_f;
out vec3 pos_f;

uniform vec3 factor;
uniform mat4 transform;
uniform mat4 camera;
uniform mat4 proj;


void main(void){
//     pos_f = (transform * vec4(pos.x*factor.x, pos.y*factor.y,0,1)).xyz;
//     gl_Position = camera * vec4(pos_f,1);
//     uv_f = vec2(pos.x, 1-pos.y);

    // Billboard effects
    vec4 p = camera * vec4(transform[3].xyz,1);
    gl_Position = proj * (p + vec4(pos.x * factor.x*factor.z, pos.y * factor.y*factor.z, 0, 1));
    pos_f = p.xyz;
    uv_f = uv;

}
