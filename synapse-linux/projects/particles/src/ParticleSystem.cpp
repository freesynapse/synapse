
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
    ParticleSystem::ParticleSystem(uint32_t _count) :
        m_particleCount(_count)
    {
        m_particlePool.resize(m_particleCount);
        m_poolIndex = m_particleCount - 1;

        m_texture = API::newTexture2D("../assets/textures/particle.png");

    }

    //-----------------------------------------------------------------------------------
    void ParticleSystem::onUpdate(float _dt)
    {
        SYN_PROFILE_FUNCTION();

        m_activeParticleCount = m_particleCount;

        for (auto& particle : m_particlePool)
        {
            if (!particle.active)
            {
                m_activeParticleCount--;
                continue;
            }

            if (particle.lifeRemaining <= 0.0f)
            {
                particle.active = false;
                m_activeParticleCount--;
                continue;
            }

            particle.lifeRemaining -= _dt;
            particle.position += particle.velocity * _dt;
            particle.velocity += particle.acceleration * _dt;
            particle.rotation += 1.0f * _dt;
        }
    }


    //-----------------------------------------------------------------------------------
    void ParticleSystem::onRender(const Ref<Camera>& _camera, const Ref<Shader>& _shader)
    {
        SYN_PROFILE_FUNCTION();

        if (m_vao == nullptr)
        {
            
            float vertices[] = {
                -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
                 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
                 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
            };

            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW/*GL_STREAM_DRAW*/);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
                { VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv "},
            });
            vbo->setData((void*)vertices, sizeof(vertices));

            m_vao = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }

        _shader->enable();
        _shader->setMatrix4fv("u_view_projection_matrix", _camera->getViewProjectionMatrix());
        
        m_texture->bind();
        _shader->setUniform1i("u_sampler", 0);

        for (auto& particle : m_particlePool)
        {
            float life = particle.lifeRemaining / particle.lifeTime;
            glm::vec4 color = glm::lerp(particle.colorEnd, particle.colorBegin, life);
            color.a = color.a * life;

            float size = glm::lerp(particle.sizeEnd, particle.sizeBegin, life);
            
            // create model matrix
            glm::mat4 model = glm::translate(glm::mat4(1.0f), { particle.position.x, particle.position.y, 0.0f }) *
                              glm::rotate(glm::mat4(1.0f), particle.rotation, { 0.0f, 0.0f, 1.0f }) *
                              glm::scale(glm::mat4(1.0f), { size, size, 0.0f });
            _shader->setMatrix4fv("u_model_matrix", model);
            _shader->setUniform4fv("u_color", color);
            m_vao->bind();
            Renderer::drawArrays(4, 0, true, GL_TRIANGLE_STRIP);
        }
    }


    //-----------------------------------------------------------------------------------
    void ParticleSystem::emit(const ParticleProps& _particle_props)
    {
        Particle& particle = m_particlePool[m_poolIndex];
        particle.active = true;
        particle.position = _particle_props.position;
        particle.rotation = Random::rfloat() * 2.0f * glm::pi<float>();

        particle.velocity = _particle_props.velocity;
        particle.velocity.x += _particle_props.velocityVariation.x * (Random::rfloat() - 0.5f);
        particle.velocity.y += _particle_props.velocityVariation.y * (Random::rfloat() - 0.5f);

        particle.colorBegin = _particle_props.colorBegin;
        particle.colorEnd = _particle_props.colorEnd;

        particle.lifeTime = _particle_props.lifeTime;
        particle.lifeRemaining = _particle_props.lifeTime;
        particle.sizeBegin = _particle_props.sizeBegin + _particle_props.sizeVariation * (Random::rfloat() - 0.5f);
        particle.sizeEnd = _particle_props.sizeEnd;

        //m_poolIndex = --m_poolIndex % m_particleCount; ==> This doesn't work when decreasing.
        m_poolIndex--;
        if (m_poolIndex == 0)   
            m_poolIndex = m_particleCount-1;
    }

}
