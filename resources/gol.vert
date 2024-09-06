#version 460

out vec2 v_uv;

void main() {
    vec2 pos = vec2(gl_VertexID % 2, gl_VertexID / 2) * -1 + 1.;

    vec2 position = pos * 2. - 1.;

    v_uv = pos;
    gl_Position = vec4(position, 0, 1);
}
