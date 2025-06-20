#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;
uniform float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    // vec2 tempSize = vec2(1920, 1080);
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, TexCoords).rgb * weight[0]; // current fragment's contribution
    if (horizontal)
    {
        for (int i = 1; i < 10; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i / 2, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i / 2, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 10; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i / 2)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i / 2)).rgb * weight[i];
        }
    }
    // FragColor = vec4(result, 1.0);
    FragColor = vec4(result.x * 1.03,  result.y, result.z, 1.0);
    // FragColor = vec4(texture(image, TexCoords).rgba.w,texture(image, TexCoords).rgba.w, texture(image, TexCoords).rgba.w, 1.0);
}
