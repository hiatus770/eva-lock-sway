#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;

// void main() {
//     vec3 bloom_color = texture(bloom, TexCoords).rgb;
//     vec3 scene_color = texture(scene, TexCoords).rgb;
//     FragColor = vec4(bloom_color + scene_color, 1.0);
//     // FragColor = vec4(scene_color, 1.0);
//     // FragColor = vec4(1.0, 1.0, 1.0, 1.0);
//     // FragColor = vec4(scene_color.g, bloom_color.g, 0.0, 1.0);
//     // FragColor = vec4(bloom_color, 1.0);
// }

// uniform float exposure; 

void main()
{
    float exposure = 1.0f; 
    // 1) fetch hdr + bloom in floating point
    vec3 hdrColor  = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloom, TexCoords).rgb; 


    // 2) add (still in HDR)
    vec3 hdr        = hdrColor + bloomColor;

    // 3) tone-map by Reinhard: color / (color + 1)
    // vec3 mapped    = vec3(1.0) - exp(-hdr * exposure);
    vec3 mapped = hdr * (2.51 * hdr + 0.03) / (hdr * (2.43 * hdr + 0.59) + 0.14);

    // alternative simpler: mapped = hdr / (hdr + vec3(1.0));

    // 4) gamma correct to account for sRGB screen
    const float gamma = 2.0f;
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
}
