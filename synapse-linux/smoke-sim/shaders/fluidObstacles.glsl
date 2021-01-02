#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec4 a_position;

void main()
{
    gl_Position = a_position;
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec3 frag_color;

void main()
{
	frag_color = vec3(1, 0, 0);
}


