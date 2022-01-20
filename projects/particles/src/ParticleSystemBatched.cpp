
#include "ParticleSystem.hpp"

#include <Synapse/Renderer/Renderer.hpp>
#include <Synapse/Utils/Random/Random.hpp>
#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Debug/Profiler.hpp>

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>


namespace Syn
{

    //-----------------------------------------------------------------------------------
    // BATCHED PARTICLE RENDERING
    //
    ParticleSystemBatched::ParticleSystemBatched(uint32_t _count)
    {
        m_particleCount = _count;
        SYN_CORE_TRACE("Batched rendering system with ", m_particleCount, " particles.");
    
        m_particlePool = new Particle[m_particleCount];
        m_poolIndex = 0;
    }

    //-----------------------------------------------------------------------------------
    void ParticleSystemBatched::onRender(const Ref<Camera>& _camera, const Ref<Shader>& _shader)
    {
        SYN_PROFILE_FUNCTION();

        if (m_vao == nullptr)
        {
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_DYNAMIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
                { VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float4, "a_uv "},  // the z-coord of uv is used for particle size
                { VERTEX_ATTRIB_LOCATION_COLOR, ShaderDataType::Float4, "a_color" },
            });
            vbo->setData(nullptr, m_particleCount * sizeof(ParticleVertex) * 4); // buffer orphaning

            uint32_t indices[m_particleCount*6];
            int offset = 0;
            for (int i = 0; i < m_particleCount*6; i += 6)
            {
                indices[i+0] = offset + 0;
                indices[i+1] = offset + 1;
                indices[i+2] = offset + 2;

                indices[i+3] = offset + 2;
                indices[i+4] = offset + 3;
                indices[i+5] = offset + 0;
                offset += 4;

                #ifdef DEBUG_PARTICLE_SYSTEM
                if (i < 4)
                {
                    SYN_CORE_TRACE("i0 = ", indices[i+0]);
                    SYN_CORE_TRACE("i1 = ", indices[i+1]);
                    SYN_CORE_TRACE("i2 = ", indices[i+2]);
                    SYN_CORE_TRACE("i3 = ", indices[i+3]);
                    SYN_CORE_TRACE("i4 = ", indices[i+4]);
                    SYN_CORE_TRACE("i5 = ", indices[i+5]);
                }
                #endif
            }
            Ref<IndexBuffer> ibo = API::newIndexBuffer(GL_TRIANGLES, GL_DYNAMIC_DRAW);
            ibo->setData((void*)indices, sizeof(indices) / sizeof(uint32_t));

            m_vao = API::newVertexArray(vbo, ibo);

            //
            Renderer::get().executeRenderCommands();
        
        } 
        // end geometry setup


        // particles could be sorted before stored as vertex data
        // loop through all particles, storing their data as shader vertex data
        ParticleVertex particleVertexData[m_particleCount*4];
        for (uint32_t i = 0; i < m_particleCount; i++)
        {
            Particle& particle = m_particlePool[i];
            float life = particle.lifeRemaining / particle.lifeTime;
            glm::vec4 color = glm::lerp(particle.colorEnd, particle.colorBegin, life);
            color.a = color.a * life;
            
            // size and activity status tucked away in .zw of uv-coordinates
            float size = glm::lerp(particle.sizeEnd, particle.sizeBegin, life);
            float active = (float)((particle.active) & (particle.lifeRemaining >= 0.0f));

            particleVertexData[i*4+0] = ParticleVertex({{ particle.position.x - 0.5f, particle.position.y - 0.5f, 0.0f }, { 0.0f, 0.0f, size, active }, color});
            particleVertexData[i*4+1] = ParticleVertex({{ particle.position.x + 0.5f, particle.position.y - 0.5f, 0.0f }, { 1.0f, 0.0f, size, active }, color});
            particleVertexData[i*4+2] = ParticleVertex({{ particle.position.x + 0.5f, particle.position.y + 0.5f, 0.0f }, { 1.0f, 1.0f, size, active }, color});
            particleVertexData[i*4+3] = ParticleVertex({{ particle.position.x - 0.5f, particle.position.y + 0.5f, 0.0f }, { 0.0f, 1.0f, size, active }, color});
            
            #ifdef DEBUG_PARTICLE_SYSTEM
            if (!i)
            {
                particle.debug();
                ParticleVertex pv0 = particleVertexData[0];
                Log::debug_vector("", "vertex0 pos  ", pv0.position);
                Log::debug_vector("", "vertex0 uv   ", pv0.uv);
                Log::debug_vector("", "vertex0 color", pv0.color);
                ParticleVertex pv1 = particleVertexData[1];
                Log::debug_vector("", "vertex1 pos  ", pv1.position);
                Log::debug_vector("", "vertex1 uv   ", pv1.uv);
                Log::debug_vector("", "vertex1 color", pv1.color);
                ParticleVertex pv2 = particleVertexData[2];
                Log::debug_vector("", "vertex2 pos  ", pv2.position);
                Log::debug_vector("", "vertex2 uv   ", pv2.uv);
                Log::debug_vector("", "vertex2 color", pv2.color);
                ParticleVertex pv3 = particleVertexData[3];
                Log::debug_vector("", "vertex3 pos  ", pv3.position);
                Log::debug_vector("", "vertex3 uv   ", pv3.uv);
                Log::debug_vector("", "vertex3 color", pv3.color);
            }
            #endif

        }

        //SYN_CORE_TRACE("sizeof(particleVertexData) = ", sizeof(particleVertexData));
        //SYN_CORE_TRACE("n = ", sizeof(particleVertexData) / sizeof(ParticleVertex) / 4);
        
        //
        _shader->enable();
        _shader->setMatrix4fv("u_view_projection_matrix", _camera->getViewProjectionMatrix());

        // upload vertex data
        //   
        Ref<VertexBuffer> vbo = m_vao->getVertexBuffer();
        vbo->bind();
        vbo->addSubData((void*)particleVertexData, m_particleCount * sizeof(ParticleVertex) * 4, 0);
        vbo->unbind();
        //Renderer::get().executeRenderCommands();
        // draw
        m_vao->bind();
        Renderer::drawIndexed(m_particleCount * 6, true, GL_TRIANGLES);
        Renderer::get().executeRenderCommands();
    }


}
