#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec2 a_position;


void main()
{
    gl_Position = vec4(a_position, 0.0f, 1.0f);
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec4 frag_color;


void main()
{
	frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);

}


