#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec4 a_position;

void main()
{
    gl_Position = a_position;
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out vec2 frag_color;

uniform sampler2D u_velocity_texture_sampler;
uniform sampler2D u_temperature_texture_sampler;
uniform sampler2D u_density_texture_sampler;

uniform float u_ambient_temperature;
uniform float u_time_step;
uniform float u_sigma;
uniform float u_kappa;

void main()
{
    ivec2 TC = ivec2(gl_FragCoord.xy);
    float T = texelFetch(u_temperature_texture_sampler, TC, 0).r;
    vec2 V = texelFetch(u_velocity_texture_sampler, TC, 0).xy;

    frag_color = V;

    if (T > u_ambient_temperature)
    {
        float D = texelFetch(u_density_texture_sampler, TC, 0).x;
        frag_color += (u_time_step * (T - u_ambient_temperature) * u_sigma - D * u_kappa) * vec2(0.0f, 1.0f);
    }

}


