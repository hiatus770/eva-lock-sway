#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;

vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x*(a*x + b)) / (x*(c*x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 bloom_color = texture(bloom, TexCoords).rgb;
    vec3 scene_color = texture(scene, TexCoords).rgb;
    scene_color += bloom_color; 
    
    float gamma = 2.2; 
    float exposure = 1.2f; 
    vec3 result = vec3(1.0) - exp(-scene_color * exposure); 
    
    result = pow(result, vec3(1.0/gamma)); 
    FragColor = vec4(result, 1.0); 
}
