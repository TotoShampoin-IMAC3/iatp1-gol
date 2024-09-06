#version 460

in vec2 v_uv;

out vec4 f_color;

// uniform usampler2D u_texture;
// void main() {
//     uint value = texture(u_texture, v_uv).r;
//     f_color = vec4(vec3(value),1);
// }

uniform sampler2D u_texture;
void main() {
    float value = texture(u_texture, v_uv).r;
    f_color = vec4(vec3(value), 1);

    // f_color = vec4(v_uv, 0, 1);
}
