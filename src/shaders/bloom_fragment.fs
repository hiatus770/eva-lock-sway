#version 330 core
in vec3 our_color;
in vec3 tex_coord;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 bright_color;

void main()
{
    frag_color = vec4(our_color, 1.0f);
    bright_color = vec4(1.0, 0.0, 0.0, 1.0);
}
