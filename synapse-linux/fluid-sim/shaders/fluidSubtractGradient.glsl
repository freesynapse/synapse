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
uniform sampler2D u_pressure_texture_sampler;
uniform sampler2D u_obstacles_texture_sampler;

uniform float u_gradient_scale;


void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    vec3 oC = texelFetch(u_obstacles_texture_sampler, T, 0).xyz;
    if (oC.x > 0)
    {
        frag_color = oC.xy;
        return;
    }

    // find neighbours in the pressure field
    float pN = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2( 0,  1)).x;
    float pS = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2( 0, -1)).x;
    float pE = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2( 1,  0)).x;
    float pW = texelFetchOffset(u_pressure_texture_sampler, T, 0, ivec2(-1,  0)).x;
    float pC = texelFetch(u_pressure_texture_sampler, T, 0).x;

    // find neighbouring obstacles
    vec3 oN = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 0,  1)).xyz;
    vec3 oS = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 0, -1)).xyz;
    vec3 oE = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2( 1,  0)).xyz;
    vec3 oW = texelFetchOffset(u_obstacles_texture_sampler, T, 0, ivec2(-1,  0)).xyz;

    // use pressure center for solid cells
    vec2 obstV = vec2(0.0f);
    vec2 vMask = vec2(1.0f);

    if (oN.x > 0) { pN = pC; obstV.y = oN.z; vMask.y = 0; }
    if (oS.x > 0) { pS = pC; obstV.y = oS.z; vMask.y = 0; }
    if (oE.x > 0) { pE = pC; obstV.x = oE.z; vMask.x = 0; }
    if (oW.x > 0) { pW = pC; obstV.x = oW.z; vMask.x = 0; }

    // enforce a slip-free boundary condition
    vec2 oldV = texelFetch(u_velocity_texture_sampler, T, 0).xy;
    vec2 grad = vec2(pE - pW, pN - pS) * u_gradient_scale;
    vec2 newV = oldV - grad;

    frag_color = (vMask * newV) + obstV;

}


