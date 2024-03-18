#version 420 core

in vec3 pos_f;
in vec2 uv_f;
in vec3 vertex_color_f;

uniform vec3 tex_id;

layout(binding = 0) uniform sampler2DArray texarray;

out vec4 color_out;

void main(void){

    vec4 from = texture(texarray, vec3(uv_f.x, uv_f.y, tex_id.x));
    vec4 to = texture(texarray, vec3(uv_f.x, uv_f.y, tex_id.y));
    to.a *= tex_id.z;
    from.a *= 1-tex_id.z;
    float alpha = from.a + to.a;
    color_out.rgb = (from.a/alpha)*from.rgb + (to.a/alpha)*to.rgb;
    color_out.a = alpha;
}
