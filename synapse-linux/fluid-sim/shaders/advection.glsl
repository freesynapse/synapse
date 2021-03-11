#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec2 a_position;

uniform vec2 u_texel_size;

out vec2 v_uv;

void main()
{
	v_uv = a_position * 0.5 + 0.5;

	gl_Position = vec4(a_position, 0.0, 1.0);
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;

uniform sampler2D u_velocity_sampler;
uniform sampler2D u_source_sampler;

uniform vec2 u_texel_size;
uniform float u_dt;
uniform float u_dissipation;

vec4 bilerp(sampler2D _sampler, vec2 _uv, vec2 _tsize)
{
	vec2 st = _uv / _tsize - 0.5;
	vec2 iuv = floor(st);
	vec2 fuv = fract(st);

	vec4 a = texture(_sampler, (iuv + vec2(0.5, 0.5)) * _tsize);
	vec4 b = texture(_sampler, (iuv + vec2(1.5, 0.5)) * _tsize);
	vec4 c = texture(_sampler, (iuv + vec2(0.5, 1.5)) * _tsize);
	vec4 d = texture(_sampler, (iuv + vec2(1.5, 1.5)) * _tsize);

	return mix(mix(a, b, fuv.x), mix(c, d, fuv.x), fuv.y);
}

void main()
{
	vec2 coord = v_uv - u_dt * texture(u_velocity_sampler, v_uv).xy * u_texel_size;
	vec4 result = texture(u_source_sampler, coord);

	float decay = 1.0 + u_dissipation * dt;

	frag_color = result / decay;
}
