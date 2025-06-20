#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 our_color;
out vec2 tex_coord; 

void main()
{
    our_color = aColor;
    tex_coord = aTex; 
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // gl_Position = projection * view * model * vec4(aPos, 1.0);
    // vec3 pos = vec3(view * model * vec4(aPos.xyz, 1.0));
    // gl_Position = vec4(aPos.xyz, 1.0);
}
