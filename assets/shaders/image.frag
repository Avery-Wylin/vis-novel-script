#version 420 core

in vec2 uv_f;
in vec3 pos_f;

uniform vec3 tex_id;

layout(binding = 0) uniform sampler2DArray texarray;
// layout(binding = 0) uniform sampler2D tex;

out vec4 color_out;

void main(void){

//        color_out = mix( texture(tex, vec2(uv_f.x, uv_f.y)), texture(tex, vec2(uv_f.x, uv_f.y)), tex_id.z);
    vec4 from = texture(texarray, vec3(uv_f.x, uv_f.y, tex_id.x));
    vec4 to = texture(texarray, vec3(uv_f.x, uv_f.y, tex_id.y));
    to.a *= tex_id.z;
    from.a *= 1-tex_id.z;
    float alpha = from.a + to.a;
    color_out.rgb = (from.a/alpha)*from.rgb + (to.a/alpha)*to.rgb;
    color_out.a = alpha;
//     color_out = vec4(1,0,0,1);
//     color_out.rg = uv_f;
//     color_out.ba = vec2(0,1);

}
