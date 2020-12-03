#TYPE VERTEX_SHADER
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 4) in vec2 a_uv;

out vec2 v_uv;

void main()
{
    v_uv = a_uv;
    gl_Position = a_position;
}


#type FRAGMENT_SHADER
#version 330 core

//uniform sampler2D u_screen_texture_sampler;

in vec2 v_uv;
out vec4 frag_color;

void main()
{
    //frag_color = texture(u_screen_texture_sampler, v_uv);
    
    // dark blue
    frag_color = vec4(0.062745098f, 0.168627451f, 0.341176471f, 1.0f);
}


