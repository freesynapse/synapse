#type VERTEX_SHADER
#version 330 core

layout(location = 0) in vec4 a_position;

void main()
{
    gl_Position = a_position;
}


#type FRAGMENT_SHADER
#version 330 core

layout(location = 0) out float frag_color;

uniform sampler2D u_velocity_texture_sampler;
uniform sampler2D u_obstacles_texture_sampler;

uniform float u_half_inverse_cell_size;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // find neighbours in the velocity field
    vec2 vN = texelFetchOffset(u_velocity_texture_sampler, T, 0, ivec2( 0,  1)).xy;
    vec2 vS = texelFetchOffset(u_velocity_texture_sampler, T, 0, ivec2( 0, -1)).xy;
    vec2 vE = texelFetchOffset(u_velocity_texture_sampler, T, 0, ivec2( 1,  0)).xy;
    vec2 vW = texelFetchOffset(u_velocity_texture_sampler, T, 0, ivec2(-1,  0)).xy;

    // find neighbouring obstacles
    vec3 oN = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 0,  1)).xyz;
    vec3 oS = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 0, -1)).xyz;
    vec3 oE = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 1,  0)).xyz;
    vec3 oW = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2(-1,  0)).xyz;

    // use pressure center for solid cells -- since no values can be collected from
    // 'inside' the obstacle
    if (oN.x > 0) vN = oN.yz;
    if (oS.x > 0) vS = oS.yz;
    if (oE.x > 0) vE = oE.yz;
    if (oW.x > 0) vW = oW.yz;

    frag_color = u_half_inverse_cell_size * (vE.x - vW.x + vN.y - vS.y);

}


