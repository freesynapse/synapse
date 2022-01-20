
#include "pch.hpp"

#include "MarchingSquares.h"
#include <Synapse/Utils/Bits.h>
#include <Synapse/Utils/Timer/Timer.hpp>
#include <Synapse/Debug/Profiler.hpp>
#include <Synapse/Utils/Noise/Noise.hpp>


namespace Syn
{
    /* I give you... The square!
     *
     *      a-----b
     *      |     |
     *      |     |
     *      d-----c
     */    
    uint8_t ScalarField::s_edgeTable[16][8] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 }, // case  0: no scalars inside
        { 1, 2, 1, 4, 0, 0, 0, 0 }, // case  1: a inside   
        { 1, 2, 2, 3, 0, 0, 0, 0 }, // case  2: b inside
        { 1, 4, 2, 3, 0, 0, 0, 0 }, // case  3: a and b inside
        { 2, 3, 3, 4, 0, 0, 0, 0 }, // case  4: c inside
        { 1, 2, 1, 4, 2, 3, 3, 4 }, // case  5: a and c inside
        { 1, 2, 3, 4, 0, 0, 0, 0 }, // case  6: b and c inside
        { 1, 4, 3, 4, 0, 0, 0, 0 }, // case  7: a, b and c inside
        { 1, 4, 3, 4, 0, 0, 0, 0 }, // case  8: d inside (same as case 7)
        { 1, 2, 3, 4, 0, 0, 0, 0 }, // case  9: a and d inside
        { 1, 4, 3, 4, 1, 2, 2, 3 }, // case 10: b and d inside
        { 2, 3, 3, 4, 0, 0, 0, 0 }, // case 11: a, b and d inside
        { 1, 4, 2, 3, 0, 0, 0, 0 }, // case 12: c and d inside
        { 1, 2, 2, 3, 0, 0, 0, 0 }, // case 13: a, c and d inside (same as case 2)
        { 1, 2, 1, 4, 0, 0, 0, 0 }, // case 14: b, c and d inside (same as case 1)
        { 0, 0, 0, 0, 0, 0, 0, 0 }  // case 15: all inside (same as case 0)
    };

    //-----------------------------------------------------------------------------------
    void ScalarField::init()
    {            
        /* The scalar field, located in the corners of each grid cell. The number of cells
         * have to be deduced from the lowest matrix dimension provided (n in an m x n matrix).
         * For the matrix to be evenly distributed in space, however, there has to be a common
         * denominator between viewport x and y.
         * The m_matrixDim holds the number of cells in each the x and y direction.
         */
        glm::vec2 _vp_size = Renderer::getViewportF();
        m_matrixDim.y = (int)_vp_size.y / m_resolution;
        m_matrixDim.x = (int)_vp_size.x / m_resolution;
        m_fieldSz = m_matrixDim.x * m_matrixDim.y;
        // allocate scalar field
        m_field = new float[m_fieldSz];
        memset(m_field, 0, sizeof(float) * m_fieldSz);
        // -------- DEBUG -------- //
        // the scalar filed value is stored in the z coordinate,
        // the x and y corresponds to the corners in the cells
        m_debugFieldVertices = new glm::vec3[m_fieldSz];
        memset(m_debugFieldVertices, 0, sizeof(glm::vec3) * m_fieldSz);
        ShaderLibrary::load("../assets/shaders/marching-squares/marchingSquaresDebug.glsl");
        m_debugShader = ShaderLibrary::get("marchingSquaresDebug");
        // prefill all x and y values of scalar field, later only the z (or noise values) will
        // be updated
        uint32_t index;
        glm::vec2 vp = Renderer::getViewportF();
        float ar = vp.x / vp.y;
        for (int y = 0; y < m_matrixDim.y; y++)
            for (int x = 0; x < m_matrixDim.x; x++)
            {
                index = y * m_matrixDim.x + x;
                glm::vec3 pos = glm::vec3(
                    x * ar * (m_resolution / vp.x) * 2.0f - ar,
                    y * (m_resolution / vp.y) * 2.0f - 1.0f,
                    0.0f
                );
                m_debugFieldVertices[index] = pos;
            }
        // -------- DEBUG -------- //
        // At most, there can be one line per scalar value, or 4 per cell.
        //m_lineVertices = new glm::vec2[m_fieldSz * 2];
        //memset(m_lineVertices, 0, sizeof(glm::vec2) * m_fieldSz * 2);
        m_lineVertices.resize(m_fieldSz * 2);

        SYN_CORE_TRACE("field dim | ", m_matrixDim.x, " x ", m_matrixDim.y, "|");
        SYN_CORE_TRACE("field size = ", m_fieldSz);

        // Using 3 octaves of perlin noise for evolution of field
        NoiseParameters param;
        param.octaveCount = 3;
        param.baseFreq = 0.04f;
        Noise::set_noise_parameters(param);
        Noise::set_noise_type_3D(NoiseType::Perlin);

        // prepare buffers for rendering the scalar field
        Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_DYNAMIC_DRAW);
        vbo->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" }
        });
        vbo->setData(0, sizeof(glm::vec3) * m_fieldSz);
        m_vaoDebug = API::newVertexArray(vbo);

        // prepare buffers for rendering iso lines
        Ref<VertexBuffer> vbo1 = API::newVertexBuffer(GL_DYNAMIC_DRAW);
        vbo1->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" }
        });
        vbo1->setData(0, sizeof(glm::vec2) * m_fieldSz * 2);
        m_vaoLines = API::newVertexArray(vbo1);

    }
    //-----------------------------------------------------------------------------------
    void ScalarField::onUpdate(float _dt)
    {
        SYN_PROFILE_FUNCTION();

        m_zNoise += _dt * 5.5f;

        updateScalarField();
        updateIsoLines();
    }
    //-----------------------------------------------------------------------------------
    void ScalarField::updateScalarField()
    {
        SYN_PROFILE_FUNCTION();
        
        static int index = 0;
        glm::vec2 vp = Renderer::getViewportF();
        float ar = vp.x / vp.y;
        for (int y = 0; y < m_matrixDim.y; y++)
        {
            for (int x = 0; x < m_matrixDim.x; x++)
            {
                index = y * m_matrixDim.x + x;
                float r = Noise::fbm_3D({ x, y, m_zNoise }, true);
                m_field[index] = r;
                m_debugFieldVertices[index].z = r;
            }
        }

        m_vaoDebug->getVertexBuffer()->updateBufferData(m_debugFieldVertices, 
                                                        sizeof(glm::vec3) * m_fieldSz);
    }
    //-----------------------------------------------------------------------------------
    void ScalarField::updateIsoLines()
    {
        SYN_PROFILE_FUNCTION();

        m_lineVertices.clear();
        m_lineVerticesSz = 0;
        for (int y = 0; y < m_matrixDim.y - 1; y++)
        {
            for (int x = 0; x < m_matrixDim.x - 1; x++)
            {
                uint32_t index_r0 =  y      * m_matrixDim.x + x;
                uint32_t index_r1 = (y + 1) * m_matrixDim.x + x;

                glm::vec3 abcd[4] = {
                    m_debugFieldVertices[index_r0    ],
                    m_debugFieldVertices[index_r0 + 1],
                    m_debugFieldVertices[index_r1 + 1],
                    m_debugFieldVertices[index_r1    ]
                };

                uint8_t lookupIndex = 0;
                if (m_field[index_r0    ] < m_isoLevel) lookupIndex |= 1;
                if (m_field[index_r0 + 1] < m_isoLevel) lookupIndex |= 2;
                if (m_field[index_r1 + 1] < m_isoLevel) lookupIndex |= 4;
                if (m_field[index_r1    ] < m_isoLevel) lookupIndex |= 8;
                //uint8_t a = m_field[index_r0    ] < m_isoLevel ? 0 : 1;
                //uint8_t b = m_field[index_r0 + 1] < m_isoLevel ? 0 : 1;
                //uint8_t c = m_field[index_r1 + 1] < m_isoLevel ? 0 : 1;
                //uint8_t d = m_field[index_r1    ] < m_isoLevel ? 0 : 1;
                //uint8_t lookupIndex = d << 3 | c << 2 | b << 1 | a;

                uint8_t* vertexIndices = s_edgeTable[lookupIndex];

                // populate the lines array per cell
                for (int i = 0; i < 4; i++)
                {
                    int i0 = vertexIndices[2*i+0] - 1;
                    int i1 = vertexIndices[2*i+1] - 1;
                    if (i0 < 0 || i1 < 0)
                        break;

                    glm::vec2 v;
                    // midpoint 'interpolation'
                    //v.x = abcd[i0].x + (abcd[i1].x - abcd[i0].x) / 2.0f;
                    //v.y = abcd[i0].y + (abcd[i1].y - abcd[i0].y) / 2.0f;
                    // linearly interpolated
                    if (abs(m_isoLevel - abcd[i0].z) < 0.001)       v = abcd[i0];
                    else if (abs(m_isoLevel - abcd[i1].z) < 0.001)  v = abcd[i1];
                    else if (abs(abcd[i0].z - abcd[i1].z) < 0.001)  v = abcd[i0];
                    else
                    {
                        float t = (m_isoLevel - abcd[i0].z) / (abcd[i1].z - abcd[i0].z);
                        v.x = abcd[i0].x + t * (abcd[i1].x - abcd[i0].x);
                        v.y = abcd[i0].y + t * (abcd[i1].y - abcd[i0].y);
                    }
                    //
                    m_lineVertices.push_back(v);
                }
            }
        }

        m_lineVerticesSz = m_lineVertices.size();
        m_vaoLines->getVertexBuffer()->updateBufferData(m_lineVertices.data(), 
                                                        m_lineVerticesSz*sizeof(glm::vec2));
    }
    //-----------------------------------------------------------------------------------
    void ScalarField::onRender(const Ref<Camera>& _camera)
    {
        Renderer::setLineWidth(2.0f);
        // -------- DEBUG : show scalar field values -------- //
        m_debugShader->enable();
        m_debugShader->setMatrix4fv("u_view_projection_matrix", _camera->getViewProjectionMatrix());
        m_vaoDebug->bind();
        Renderer::drawArraysNoDepth(m_fieldSz, 0, GL_POINTS);

        // -------- INTERPOLATED ISO LINES -------- //
        m_shader->enable();
	    m_shader->setMatrix4fv("u_view_projection_matrix", _camera->getViewProjectionMatrix());
        m_vaoLines->bind();
        Renderer::drawArraysNoDepth(m_lineVerticesSz, 0, GL_LINES);
        
    }

}
