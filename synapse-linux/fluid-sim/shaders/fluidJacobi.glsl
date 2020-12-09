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

uniform sampler2D u_pressure_texture_sampler;
uniform sampler2D u_divergence_texture_sampler;
uniform sampler2D u_obstacles_texture_sampler;

uniform float u_alpha;
uniform float u_inverse_beta;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // find neighbours in the pressure field
    vec4 pN = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2( 0,  1));
    vec4 pS = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2( 0, -1));
    vec4 pE = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2( 1,  0));
    vec4 pW = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2(-1,  0));
    vec4 pC = texelFetch(u_pressure_texture_sampler, T, 0);

    // find neighbouring obstacles
    vec3 oN = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 0,  1)).xyz;
    vec3 oS = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 0, -1)).xyz;
    vec3 oE = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 1,  0)).xyz;
    vec3 oW = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2(-1,  0)).xyz;

    // use pressure center for solid cells
    if (oN.x > 0) pN = pC;
    if (oS.x > 0) pS = pC;
    if (oE.x > 0) pE = pC;
    if (oW.x > 0) pW = pC;

    vec4 bC = texelFetch(u_divergence_texture_sampler, T, 0);
    frag_color = (pN + pS + pE + pW + u_alpha * bC) * u_inverse_beta;

}


