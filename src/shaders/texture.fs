#version 330 core
out vec4 frag_color;

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
	frag_color = texture(tex, tex_coord) * vec4(our_color, 1.0f);
	
}

