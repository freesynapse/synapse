
#pragma once

#include <Synapse/Renderer/Camera/Camera.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Renderer/Material/Texture2D.hpp>


namespace Syn
{
    struct ParticleVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 uv;
        float texID;
    };

    struct ParticleProps
    {
        glm::vec2 position;
        glm::vec2 velocity, velocityVariation;
        glm::vec4 colorBegin, colorEnd;
        float sizeBegin, sizeEnd, sizeVariation;
        float lifeTime = 1.0f;
    };

    
    class ParticleSystem
    {
    public:
        ParticleSystem(uint32_t _count);

        void onUpdate(float _dt);
        void onRender(const Ref<Camera>& _camera, const Ref<Shader>& _shader=nullptr);

        void emit(const ParticleProps& _particle_props);

        // Accessors
        const uint32_t getActiveParticleCount() const { return m_activeParticleCount; }


    private:
        struct Particle
        {
            glm::vec2 position;
            glm::vec2 velocity;
            glm::vec2 acceleration = glm::vec2(0.0f, -10.0f);
            glm::vec4 colorBegin, colorEnd;
            float rotation = 0.0f;
            float sizeBegin, sizeEnd;
            float lifeTime = 1.0f;
            float lifeRemaining = 0.0f;
            bool active = false;
        };
        std::vector<Particle> m_particlePool;
        int m_poolIndex;

        uint32_t m_particleCount = 0;
        uint32_t m_activeParticleCount = 0;

        Ref<Texture2D> m_texture = nullptr;
        Ref<VertexArray> m_vao = nullptr;

    };



}

