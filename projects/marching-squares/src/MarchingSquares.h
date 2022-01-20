
#pragma once

#include <Synapse/Renderer/Renderer.hpp>
#include <Synapse/Renderer/Shader/ShaderLibrary.hpp>
#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Utils/Random/Random.hpp>

namespace Syn
{

    class ScalarField
    {
    public:
        ScalarField(uint32_t _resolution, const Ref<Shader>& _shader) :
            m_resolution(_resolution), m_shader(_shader)
        { init(); }

        ~ScalarField() 
        { 
            delete[] m_field;
            delete[] m_debugFieldVertices;
            //delete[] m_lineVertices;
        }

        void onUpdate(float _dt);
        void onRender(const Ref<Camera>& _camera);

    private:
        void init();
        void updateScalarField();
        void updateIsoLines();


    private:
        // debug
        glm::vec3* m_debugFieldVertices = nullptr;
        Ref<VertexArray> m_vaoDebug     = nullptr;
        Ref<Shader> m_debugShader       = nullptr;
        
        // field pointers
        float* m_field              = nullptr;
        std::vector<glm::vec2> m_lineVertices;
        //glm::vec2* m_lineVertices   = nullptr;
        size_t m_lineVerticesSz   = 0;

        // dimensions of the scalar field
        uint32_t m_fieldSz          = 0;
        uint32_t m_resolution       = 0;
        glm::ivec2 m_matrixDim      = { 0, 0 };
        float m_isoLevel            = 0.5f;
        
        // noise parameters
        float m_zNoise              = 0.0f;

        // lookup table for marching squares
        static uint8_t s_edgeTable[16][8];
        // OpenGL rendering objects
        Ref<VertexArray> m_vaoLines = nullptr;
        Ref<Shader> m_shader        = nullptr;
    };


}
