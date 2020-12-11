#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec4 a_position;

void main()
{
    gl_Position = a_position;
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec4 frag_color;

uniform sampler2D u_velocity_texture_sampler;
uniform sampler2D u_source_texture_sampler;
uniform sampler2D u_obstacles_texture_sampler;

uniform vec2 u_inverse_size;
uniform float u_time_step;
uniform float u_dissipation;

void main()
{
	vec2 frag_coord = gl_FragCoord.xy;
	float solid = texture(u_obstacles_texture_sampler, u_inverse_size * frag_coord).x;
	if (solid > 0)
	{
		frag_color = vec4(0.0f);
		return;
	}

	vec2 u = texture(u_velocity_texture_sampler, u_inverse_size * frag_coord).xy;
	vec2 coord = u_inverse_size * (frag_coord - u_time_step * u);
	frag_color = u_dissipation * texture(u_source_texture_sampler, coord);
	
}


