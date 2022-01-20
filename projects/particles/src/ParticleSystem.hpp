
#pragma once

#include <Synapse/Core.hpp>
#include <Synapse/Debug/Log.hpp>
#include <Synapse/Renderer/Camera/Camera.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Renderer/Material/Texture2D.hpp>


//#define DEBUG_PARTICLE_SYSTEM


namespace Syn
{
    struct Particle
    {
        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec2 acceleration = glm::vec2(0.0f, -9.82f);
        glm::vec4 colorBegin, colorEnd;
        float rotation = 0.0f;
        float sizeBegin, sizeEnd;
        float lifeTime = 1.0f;
        float lifeRemaining = 0.0f;
        bool active = false;

        void debug()
        {
            SYN_CORE_TRACE("debug particle:");
            Log::debug_vector("", "pos", position);
            SYN_CORE_TRACE("lifeRemaining = ", lifeRemaining);
            SYN_CORE_TRACE("active = ", (active ? "TRUE":"FALSE"));
        }
    };

    struct ParticleProps
    {
        glm::vec2 position;
        glm::vec2 velocity, velocityVariation;
        glm::vec4 colorBegin, colorEnd;
        float sizeBegin, sizeEnd, sizeVariation;
        float lifeTime = 1.0f;
    };


    //-----------------------------------------------------------------------------------
    // PARTICLE RENDERING BASE CLASS
    //
    class ParticleSystemBase
    {
    public:
        ParticleSystemBase() {}
        virtual void onUpdate(float _dt);
        virtual void onRender(const Ref<Camera>&, const Ref<Shader>&) = 0;
        virtual void emit(const ParticleProps& _particle_props);
        // Accessors
        virtual const uint32_t getActiveParticleCount() const { return m_activeParticleCount; }

    protected:
        Particle* m_particlePool = nullptr;
        int m_poolIndex;
        uint32_t m_particleCount = 0;
        uint32_t m_activeParticleCount = 0;
        Ref<VertexArray> m_vao = nullptr;
    };


    //-----------------------------------------------------------------------------------
    // NAIVE PARTICLE RENDERING CLASS
    //
    class ParticleSystemNaive : public ParticleSystemBase
    {
    public:
        ParticleSystemNaive(uint32_t _count);
        ~ParticleSystemNaive() { if (m_particlePool) delete[] m_particlePool; }
        virtual void onRender(const Ref<Camera>& _camera, const Ref<Shader>& _shader=nullptr);

    private:

    };

    //-----------------------------------------------------------------------------------
    // BATCHED PARTICLE RENDERING CLASS
    //
    class ParticleSystemBatched : public ParticleSystemBase
    {
    public:
        ParticleSystemBatched(uint32_t _count);
        ~ParticleSystemBatched() { if (m_particlePool) delete[] m_particlePool; }
        void onRender(const Ref<Camera>& _camera, const Ref<Shader>& _shader=nullptr);

    private:
        struct ParticleVertex 
        { 
            glm::vec3 position;
            glm::vec4 uv;   // the .zw is used for size and alive
            glm::vec4 color;
            ParticleVertex() {}
            ParticleVertex(glm::vec3 _p, glm::vec4 _uv, glm::vec4 _c) :
                position(_p), uv(_uv), color(_c)
            {}
        };

    };


}

