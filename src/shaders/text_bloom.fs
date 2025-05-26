
#version 330 core
layout(location = 0) out vec4 normal;
layout(location = 1) out vec4 bloom;
in vec2 tex_coords;

uniform sampler2D text;
uniform vec3 text_color;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, tex_coords).r);
    normal = vec4(text_color, 1.0) * sampled;
    bloom  = vec4(text_color, 1.0) * sampled;
    // normal = vec4(0.0, 1.0, 0.0, 1.0) * sampled;
    // bloom  = vec4(1.0, 0.0, 0.0, 1.0) * sampled;
    // color = vec4(1.0, 1.0, 0.0, 1.0);
}
