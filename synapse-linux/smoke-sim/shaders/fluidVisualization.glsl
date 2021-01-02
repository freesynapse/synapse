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

uniform sampler2D u_texture_sampler;

uniform vec3 u_fill_color = vec3(1.0f, 1.0f, 0.0f);
uniform vec2 u_scale = vec2(0.0013947f, 0.00204918f);

void main()
{
	float L = texture(u_texture_sampler, gl_FragCoord.xy * u_scale).x;
	frag_color = vec4(u_fill_color, L);
}


