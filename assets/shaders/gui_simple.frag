#version 430 core

in vec2 uv_f;
in vec2 pos_f;
out vec4 color_out;
uniform vec3 object_color;
uniform vec3 object_color2;
uniform float factor;
uniform vec4 transform;

// layout(binding = 0) uniform sampler2D texture0;

void main(void){
    vec2 u,t;
    t = transform.zw * factor;
    u = t*(abs(uv_f - .5) - .5 ) + 0.5;
    u *= vec2(greaterThan(u,vec2(0)));
    float f = u.x * u.x + u.y * u.y;
    if(f>.25){
        discard;
    }

    color_out.rgb =  mix(object_color2, object_color,uv_f.y);
    color_out.a = .8;
}
