#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec2 a_position;

uniform vec2 u_texel_size;

out vec2 v_uv;
out vec2 v_l;
out vec2 v_r;
out vec2 v_b;
out vec2 v_t;

void main()
{
	v_uv = a_position * 0.5 + 0.5;
	v_l = v_uv - vec2(u_texel_size.x, 0.0);
	v_r = v_uv + vec2(u_texel_size.x, 0.0);
	v_b = v_uv - vec2(0.0, u_texel_size.y);
	v_t = v_uv + vec2(0.0, u_texel_size.y);

	gl_Position = vec4(a_position, 0.0, 1.0);
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;
in vec2 v_l;
in vec2 v_r;
in vec2 v_b;
in vec2 v_t;

uniform sampler2D u_velocity_sampler;
uniform sampler2D u_pressure_sampler;

void main()
{
	float L = texture(u_pressure_sampler, v_l).x;
	float R = texture(u_pressure_sampler, v_r).x;
	float B = texture(u_pressure_sampler, v_b).x;
	float T = texture(u_pressure_sampler, v_t).x;

	vec2 vel = texture(u_velocity_sampler, v_uv).xy;
	vel -= vec2(R - L, T - B);

	frag_color = vec4(vel, 0.0, 1.0);
}

