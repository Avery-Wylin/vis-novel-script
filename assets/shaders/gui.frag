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

    // Bubble style
//     vec3 n = normalize(vec3(u * sign(uv_f - vec2(.5)),(1-f*6)));
//     float d = (dot( n, vec3(-.57,.57,.57))+1)*.5;
//     color_out.rgb = mix(object_color2,object_color, d);
//     color_out.rgb += pow(d,15);

    // Flat line style
//     color_out.rgb =  f > 0.15 ?object_color2:object_color.rgb;

    // Clean Style
    vec3 n = normalize(vec3(u * sign(uv_f - vec2(.5)),(1-f*4)));
    float d = (dot( n,vec3(0,.707,.707))+1)*.5;
    color_out.rgb = f > .2 ? object_color2 : object_color * d;
    color_out.a = 1;
}
