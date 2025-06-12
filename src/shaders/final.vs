#version 330 core

layout(location = 0) in vec3 aPos; // coordinates of the vertexes
layout(location = 1) in vec2 aTexCoords; // Location of the vertx coordinates used for rendering the final quad

out vec2 TexCoords;
out vec3 pos; 

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0); /// i dont even think this is doing anything???
    pos = aPos; 
}
