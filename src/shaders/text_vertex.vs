#version 330 core
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 tex_coords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
    // gl_Position = vec4(vertex.xy, 0.0, 1.0);
    // gl_Position = vec4(vertex.xy, 0.0, 1.0);
    tex_coords = vertex.zw;
}
