#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 pos; 

uniform sampler2D scene;
uniform sampler2D bloom;

uniform float radius = 0.0f;
uniform float flash_alpha = 0.0;
uniform vec3  flash_color = vec3(1.0, 0.0, 0.0);

vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 bloom_color = texture(bloom, TexCoords).rgb;
    vec3 scene_color = texture(scene, TexCoords).rgb;
    scene_color += bloom_color;

    float gamma = 2.2;
    float exposure = 1.2f;
    vec3 result = vec3(1.0) - exp(-scene_color * exposure);
    
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
    
    if (pos.x*pos.x + pos.y*pos.y < radius*radius){
        FragColor = FragColor * vec4(1.0, 0.2, 0.1, 1.0);
    }

    // Auth feedback flash overlay
    if (flash_alpha > 0.0) {
        FragColor = mix(FragColor, vec4(flash_color, 1.0), flash_alpha * 0.55);
    }
}
