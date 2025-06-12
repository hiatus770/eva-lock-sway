#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 main_color; 

void main()
{
    our_color = aColor;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // gl_Position = projection * view * model * vec4(aPos, 1.0);
    // vec3 pos = vec3(view * model * vec4(aPos.xyz, 1.0));
    // gl_Position = vec4(aPos.xyz, 1.0);
}
