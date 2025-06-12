#version 330 core
layout(location = 0) out vec4 normal;
layout(location = 1) out vec4 bloom;

in vec3 our_color;
in vec2 tex_coord;

// texture samplers
uniform sampler2D tex;

void main()
{
    // linearly interpolate between both textures (80% container, 20% awesomeface)
    vec4 texColor = texture(tex, tex_coord).rgba;
    // frag_color = vec4(0.0f, 0.5f, 0.5f, 1.0f );
    // frag_color = vec4(texColor);
    // frag_color = texture(tex, tex_coord) * vec4(our_color, 1.0f);
    normal = texture(tex, tex_coord) * vec4(our_color, 1.0f);
    bloom = texture(tex, tex_coord) * vec4(our_color, 1.0f);
    // bloom = vec4(0.0, 1.0, 0.0, 1.0); 
}
