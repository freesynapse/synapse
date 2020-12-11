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

uniform vec2 u_point;
uniform float u_radius;
uniform vec3 u_fill_color;

void main()
{
    // adds a 'splat' of dye with u_radius at u_point with u_fill_color.
    float d = distance(u_point, gl_FragCoord.xy);
    if (d < u_radius)
    {
        float a = (u_radius - d) * 0.5f;
        a = min(a, 1.0f);
        frag_color = vec4(u_fill_color, a);
    }
    else
    {
        frag_color = vec4(0.0f);
    }
}


