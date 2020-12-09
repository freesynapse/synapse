#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 4) in vec2 a_uv;

out vec2 v_uv;

void main()
{
    v_uv = a_uv;
    gl_Position = vec4(a_position, 1.0f);
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec4 out_color;

in vec2 v_uv;

uniform sampler2D u_texture_sampler;

void main()
{
    out_color = texture(u_texture_sampler, v_uv);
    //out_color = vec4(v_uv.xy, 0, 1);
    
    // dark blue
    //out_color = vec4(0.062745098f, 0.168627451f, 0.341176471f, 1.0f);
}


