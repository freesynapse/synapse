
#pragma once

#include "pch.hpp"

#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/Camera/Camera.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Renderer/Mesh/Mesh.hpp>


namespace Syn
{
    typedef struct vertex_t
    {
        glm::vec4 pos;      // .xyz = pos, .w = noise evaluation
        glm::vec3 normal; 
    } vertex_t;

    typedef struct vertex3_t
    {
        glm::vec3 pos;
        glm::vec3 normal; 
        vertex3_t() {}
        vertex3_t(const glm::vec3& _p, const glm::vec3& _n) :
            pos(_p), normal(_n) {}

    } vertex3_t;
    
    typedef struct triangle_t
    {
        glm::vec3 p0;   // position
        glm::vec3 n0;   // normal
        glm::vec3 p1;
        glm::vec3 n1;
        glm::vec3 p2;
        glm::vec3 n2;
    } triangle_t;

    class ScalarField3D
    {
    public:
        ScalarField3D(float _field_axis_length, 
                      uint32_t _resolution, 
                      const Ref<Camera>& _camera) :
            m_fieldAxis(_field_axis_length), 
            m_resolution(_resolution), 
            m_cameraPtr(_camera)
        { 
            init();
        }

        ~ScalarField3D() 
        { 
            delete[] m_fieldVertices;
        }

        void update(float _dt);
        void render(bool _wire_frame, bool _backface);

        uint32_t getTriangleCount() { return m_nTriangles; }
        void setZOffset(float _z) { m_zOffset = _z; }
        void setNoiseOctaves(int _n) { m_octaveCount = _n; Noise::set_param_octave_count(_n); }
        void setNoiseFrequency(float _f) { m_noiseFreq = _f; Noise::set_param_base_frequency(_f); }

    private:
        void init();
        void updateScalarField();
        void updateIsoSurface();
        void updateIsoSurfaceThreaded();
        void setAxes();

        glm::vec3 vertexInterp(const glm::vec4& _p0, const glm::vec4& _p1);
        vertex3_t vertexInterpN(const glm::vec4& _p0, const glm::vec4& _p1, 
                                const glm::vec3& _n0, const glm::vec3& _n1);
    private:
        // dimensions
        float m_fieldAxis                   = 0.0f;
        uint32_t m_resolution               = 0;
        uint32_t m_resolution2              = 0;
        float m_cellSize                    = 0.0f;
        float m_origin                      = 0.0f;
        
        // noise
        float m_zOffset                     = 0.0f;
        float m_noiseFreq                   = 0.01f;
        int m_octaveCount                   = 3;
        
        // scalar field
        uint32_t m_nScalars                 = 0;
        vertex_t* m_fieldVertices           = nullptr;
        Ref<Shader> m_fieldShader           = nullptr;
        
        // iso surface
        triangle_t* m_isoSurface            = nullptr;
        float m_isoLevel                    = 0.08f;
        uint32_t m_nTriangles               = 0;
        const uint8_t m_maxTrisPerCell      = 5;
        Ref<Shader> m_isoSurfaceShader      = nullptr;

        // rendering variables
        //glm::vec3 m_materialAlbedo          = glm::vec3(0.055f, 0.91f, 0.208f);   // green
        glm::vec3 m_materialAlbedo          = glm::vec3(0.0314f, 0.305f, 0.70f);    // blue
        float m_materialMetallic            = 0.50f;
        float m_materialRoughness           = 0.17f;
        float m_materialAO                  = 1.0f;
        glm::vec3 m_lightPositions[4] =
        {
            glm::vec3(14.0f,  12.0f,  12.0f),
            glm::vec3(14.0f,  12.0f, -12.0f),
            glm::vec3(14.0f, -12.0f,  12.0f),
            glm::vec3(14.0f, -12.0f, -12.0f)
        };

        // misc Synapse objects
        Ref<Camera> m_cameraPtr             = nullptr;
        Ref<VertexArray> m_vaoIsoSurface    = nullptr;
        Ref<VertexArray> m_vaoScalars       = nullptr;
    };

}


