
// uniform mat4 rotation;
// attribute vec4 pos;
// attribute vec4 color;
// varying vec4 v_color;
// void main() {
//    gl_Position = rotation * pos;
//    v_color = color * pos;
// };

#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
