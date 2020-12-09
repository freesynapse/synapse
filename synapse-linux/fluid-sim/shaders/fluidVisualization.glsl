#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec4 a_position;
layout(location = 4) in vec2 a_uv;

out vec2 v_uv;

void main()
{
	v_uv = a_uv;
    gl_Position = a_position;
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec4 frag_color;

in vec2 v_uv;

uniform sampler2D u_texture_sampler;

uniform vec3 u_fill_color;
uniform vec2 u_scale;

void main()
{
	float L = texture(u_texture_sampler, gl_FragCoord.xy * u_scale).x;
	frag_color = vec4(u_fill_color, L);
}


