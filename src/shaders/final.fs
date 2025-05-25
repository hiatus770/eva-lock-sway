#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;

void main() {
    vec3 bloom_color = texture(bloom, TexCoords).rgb;
    vec3 scene_color = texture(scene, TexCoords).rgb;
    FragColor = vec4(bloom_color + scene_color, 1.0);
    // FragColor = vec4(scene_color, 1.0);
    // FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    // FragColor = vec4(scene_color.g, bloom_color.g, 0.0, 1.0);
    // FragColor = vec4(bloom_color, 1.0);
}
