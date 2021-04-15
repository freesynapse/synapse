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

uniform float u_value;
uniform sampler2D u_texture_sampler;

void main()
{
	frag_color = u_value * texture(u_texture_sampler, v_uv);

}


