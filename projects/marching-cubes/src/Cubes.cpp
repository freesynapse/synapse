
#include "pch.hpp"

#include <glm/gtx/normal.hpp>

#include <Synapse/Core.hpp>
#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Renderer/MeshCreator.hpp>
#include <Synapse/Renderer/Shader/ShaderLibrary.hpp>
#include <Synapse/Utils/Noise/Noise.hpp>
#include <Synapse/Utils/Timer/Timer.hpp>
#include <Synapse/Utils/Thread/ThreadPool.h>
#include <Synapse/Memory/MemoryLog.hpp>
#include <Synapse/Debug/Profiler.hpp>

#include "Cubes.h"
#include "Tables.h"


namespace Syn
{
    //-----------------------------------------------------------------------------------
    void ScalarField3D::init()
    {
        SYN_PROFILE_FUNCTION();

        // number of scalars, and helpers for indexing
        m_nScalars = m_resolution * m_resolution * m_resolution;
        m_resolution2 = m_resolution * m_resolution;
        
        // size of each grid 'cube'
        m_cellSize = m_fieldAxis / m_resolution;
        
        // the origin, corresponding to half the size of the resolution,
        // spanning [-res/2 ... res/2] along each axis.
        m_origin = (float)(m_resolution - 1) / 2.0f;
        
        // storage for scalar field and debug field visualizer
        //m_fieldVertices = new glm::vec4[m_nScalars];
        //memset(m_fieldVertices, 0, sizeof(glm::vec4) * m_nScalars);        
        m_fieldVertices = new vertex_t[m_nScalars];
        memset(m_fieldVertices, 0, sizeof(vertex_t) * m_nScalars);

        // Noise parameters, static and used henceforth
        Noise::set_param_octave_count(m_octaveCount);
        
        // preset all field positions
        float step = 1.0f / m_resolution;
        for (int z = 0; z < m_resolution; z++)
        {
            for (int y = 0; y < m_resolution; y++)
            {
                for (int x = 0; x < m_resolution; x++)
                {
                    int index = z * m_resolution2 + y * m_resolution + x;
                    //float r = Noise::single_4D(x*m_noiseFreq, y*m_noiseFreq, z*m_noiseFreq, m_zOffset);
                    // noise in .x, partial derivatives in .yzw
                    glm::vec4 noise_d = Noise::fbm_perlin3_d(glm::vec3(step * x, step * y, step * z));

                    m_fieldVertices[index].pos = 
                    {
                        (x - m_origin) * m_cellSize,
                        (y - m_origin) * m_cellSize,
                        (z - m_origin) * m_cellSize,
                        noise_d.x
                    };
                    m_fieldVertices[index].normal = glm::vec3(-noise_d.y, 1.0f, -noise_d.w);
                }
            }
        }

        // scalar field visualization objects and (debug) shader
        //
        m_fieldShader = ShaderLibrary::get("scalarFieldShader");

        Ref<VertexBuffer> vbo0 = API::newVertexBuffer(GL_DYNAMIC_DRAW);
        vbo0->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" },
            { VERTEX_ATTRIB_LOCATION_NORMAL,   ShaderDataType::Float3, "a_normal" }
        });
        vbo0->setData(m_fieldVertices, sizeof(vertex_t) * m_nScalars);
        
        m_vaoScalars = API::newVertexArray(vbo0);

        // iso surface array and shaders
        //
        //m_isoSurfaceShader = ShaderLibrary::get("isoSurfaceShader");
        m_isoSurfaceShader = ShaderLibrary::get("PBRShader");
        m_isoSurfaceShader->enable();
        m_isoSurfaceShader->setUniform1f("u_ao", m_materialAO);
        m_isoSurfaceShader->setUniform3fv("point_light_position_0", m_lightPositions[0]);
        m_isoSurfaceShader->setUniform3fv("point_light_position_1", m_lightPositions[1]);
        m_isoSurfaceShader->setUniform3fv("point_light_position_2", m_lightPositions[2]);
        m_isoSurfaceShader->setUniform3fv("point_light_position_3", m_lightPositions[3]);
        
        // --- DEBUG : visalize lights --- //
        for (int i = 0; i < 4; i++)
            MeshCreator::createDebugCube(m_lightPositions[i], 0.1f);
        // --- DEBUG : visalize lights --- //

        m_isoSurface = new triangle_t[m_nScalars * m_maxTrisPerCell];
        memset(m_isoSurface, 0, sizeof(triangle_t) * m_nScalars * m_maxTrisPerCell);

        Ref<VertexBuffer> vbo1 = API::newVertexBuffer(GL_DYNAMIC_DRAW);
        vbo1->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            { VERTEX_ATTRIB_LOCATION_NORMAL,   ShaderDataType::Float3, "a_normal" }
        });
        vbo1->setData(m_isoSurface, sizeof(triangle_t) * m_nScalars * m_maxTrisPerCell);
        
        m_vaoIsoSurface = API::newVertexArray(vbo1);
        
        SYN_CORE_TRACE("iso surface : allocated ", \
                       MemoryLog::_fmt_sz(sizeof(triangle_t) * m_nScalars * m_maxTrisPerCell));

        // put axes around the field (using <MeshDebug> lines)
        setAxes();

    }
    //-----------------------------------------------------------------------------------
    void ScalarField3D::update(float _dt)
    {
        SYN_PROFILE_FUNCTION();

        m_zOffset += _dt * 10.0f;

        updateScalarField();
        updateIsoSurface();
        //updateIsoSurfaceThreaded();
    }
    //-----------------------------------------------------------------------------------
    void ScalarField3D::render(bool _wire_frame, bool _backface)
    {
        SYN_PROFILE_FUNCTION();

        // --- DEBUG : render scalar field as points ---
        //m_fieldShader->enable();
        //m_fieldShader->setMatrix4fv("u_view_projection_matrix", m_cameraPtr->getViewProjectionMatrix());
        //m_vaoScalars->bind();
        //Renderer::drawArraysNoDepth(m_nScalars, 0, GL_POINTS);
        // --- DEBUG : render scalar field as points ---

        // render bounding box and ligts
        //MeshCreator::renderDebugMesh("axes", m_cameraPtr, true);
        MeshCreator::renderDebugMeshes(m_cameraPtr);

        if (_wire_frame) Renderer::enableWireFrame();
        Renderer::setCulling(_backface);

        // render iso surface
        m_isoSurfaceShader->enable();
        m_isoSurfaceShader->setMatrix4fv("u_view_projection_matrix", 
                                         m_cameraPtr->getViewProjectionMatrix());
        m_isoSurfaceShader->setUniform3fv("u_camera_pos", m_cameraPtr->getPosition());
		m_isoSurfaceShader->setUniform3fv("u_albedo", m_materialAlbedo);
		m_isoSurfaceShader->setUniform1f("u_metallic", m_materialMetallic);
		m_isoSurfaceShader->setUniform1f("u_roughness", m_materialRoughness);
        m_vaoIsoSurface->bind();
        Renderer::drawArrays(m_nTriangles * 3, 0, true, GL_TRIANGLES);

        Renderer::setCulling(!_backface);
        if (_wire_frame) Renderer::disableWireFrame();
        
    }
    //-----------------------------------------------------------------------------------
    void ScalarField3D::updateScalarField()
    {
        SYN_PROFILE_FUNCTION();

        static uint32_t index;
        static float step = 1.0f / m_resolution;
        for (int z = 0; z < m_resolution; z++)
        {
            for (int y = 0; y < m_resolution; y++)
            {
                for (int x = 0; x < m_resolution; x++)
                {
                    index = z * m_resolution2 + y * m_resolution + x;
                    glm::vec4 n_d = Noise::fbm_perlin3_d(glm::vec3(x, y, z + m_zOffset));
                    m_fieldVertices[index].pos.w = n_d.x;
                    m_fieldVertices[index].normal = glm::normalize(glm::vec3(-n_d.y, 1.0f, -n_d.w));
                    //m_fieldVertices[index].normal = glm::vec3(n_d.y, n_d.z, n_d.w);
                }
            }
        }
        m_vaoScalars->getVertexBuffer()->updateBufferData(m_fieldVertices, 
                                                          sizeof(glm::vec4) * m_nScalars);
    }
    //-----------------------------------------------------------------------------------
    void ScalarField3D::updateIsoSurface()
    {
        memset(m_isoSurface, 0, sizeof(triangle_t) * m_nScalars * m_maxTrisPerCell);
        glm::vec3 intersectionList[12];
        vertex3_t intersectionListN[12];
        m_nTriangles = 0;
        for (int z = 0; z < m_resolution - 1; z++)
        {
            for (int y = 0; y < m_resolution - 1; y++)
            {
                for (int x = 0; x < m_resolution - 1; x++)
                {
                    // Indices of vertices (in the m_fieldVertices arrays), ordering according to 
                    // reference (http://paulbourke.net/geometry/polygonise/).
                    uint32_t index_r0 = (z    ) * m_resolution2 + (y    ) * m_resolution + (x    );
                    uint32_t index_r2 = (z + 1) * m_resolution2 + (y    ) * m_resolution + (x + 1);
                    uint32_t index_r4 = (z    ) * m_resolution2 + (y + 1) * m_resolution + (x    );
                    uint32_t index_r6 = (z + 1) * m_resolution2 + (y + 1) * m_resolution + (x + 1);
                    uint32_t index_r1 = index_r0 + 1;
                    uint32_t index_r3 = index_r2 - 1;
                    uint32_t index_r5 = index_r4 + 1;
                    uint32_t index_r7 = index_r6 - 1;

                    // vertex positions for this cell
                    glm::vec4 p0 = m_fieldVertices[index_r0].pos;
                    glm::vec4 p1 = m_fieldVertices[index_r1].pos;
                    glm::vec4 p2 = m_fieldVertices[index_r2].pos;
                    glm::vec4 p3 = m_fieldVertices[index_r3].pos;
                    glm::vec4 p4 = m_fieldVertices[index_r4].pos;
                    glm::vec4 p5 = m_fieldVertices[index_r5].pos;
                    glm::vec4 p6 = m_fieldVertices[index_r6].pos;
                    glm::vec4 p7 = m_fieldVertices[index_r7].pos;

                    // vertex normals for this cell
                    glm::vec3 n0 = m_fieldVertices[index_r0].normal;
                    glm::vec3 n1 = m_fieldVertices[index_r1].normal;
                    glm::vec3 n2 = m_fieldVertices[index_r2].normal;
                    glm::vec3 n3 = m_fieldVertices[index_r3].normal;
                    glm::vec3 n4 = m_fieldVertices[index_r4].normal;
                    glm::vec3 n5 = m_fieldVertices[index_r5].normal;
                    glm::vec3 n6 = m_fieldVertices[index_r6].normal;
                    glm::vec3 n7 = m_fieldVertices[index_r7].normal;


                    // check iso values and set lookup value
                    int cubeIndex = 0;
                    if (p0.w < m_isoLevel) cubeIndex |=   1;
                    if (p1.w < m_isoLevel) cubeIndex |=   2;
                    if (p2.w < m_isoLevel) cubeIndex |=   4;
                    if (p3.w < m_isoLevel) cubeIndex |=   8;
                    if (p4.w < m_isoLevel) cubeIndex |=  16;
                    if (p5.w < m_isoLevel) cubeIndex |=  32;
                    if (p6.w < m_isoLevel) cubeIndex |=  64;
                    if (p7.w < m_isoLevel) cubeIndex |= 128;
                    
                    // trivial case, either all inside or all outside surface
                    if (g_edgeTable[cubeIndex] == 0)
                        continue;
                    
                    // find vertices where the surface intercept this cell
                    //if (g_edgeTable[cubeIndex] &    1) intersectionList[ 0] = vertexInterp(p0, p1);
                    //if (g_edgeTable[cubeIndex] &    2) intersectionList[ 1] = vertexInterp(p1, p2);
                    //if (g_edgeTable[cubeIndex] &    4) intersectionList[ 2] = vertexInterp(p2, p3);
                    //if (g_edgeTable[cubeIndex] &    8) intersectionList[ 3] = vertexInterp(p0, p3);
                    //if (g_edgeTable[cubeIndex] &   16) intersectionList[ 4] = vertexInterp(p4, p5);
                    //if (g_edgeTable[cubeIndex] &   32) intersectionList[ 5] = vertexInterp(p5, p6);
                    //if (g_edgeTable[cubeIndex] &   64) intersectionList[ 6] = vertexInterp(p6, p7);
                    //if (g_edgeTable[cubeIndex] &  128) intersectionList[ 7] = vertexInterp(p4, p7);
                    //if (g_edgeTable[cubeIndex] &  256) intersectionList[ 8] = vertexInterp(p0, p4);
                    //if (g_edgeTable[cubeIndex] &  512) intersectionList[ 9] = vertexInterp(p1, p5);
                    //if (g_edgeTable[cubeIndex] & 1024) intersectionList[10] = vertexInterp(p2, p6);
                    //if (g_edgeTable[cubeIndex] & 2048) intersectionList[11] = vertexInterp(p3, p7);
                    if (g_edgeTable[cubeIndex] &    1) intersectionListN[ 0] = vertexInterpN(p0, p1, n0, n1);
                    if (g_edgeTable[cubeIndex] &    2) intersectionListN[ 1] = vertexInterpN(p1, p2, n1, n2);
                    if (g_edgeTable[cubeIndex] &    4) intersectionListN[ 2] = vertexInterpN(p2, p3, n2, n3);
                    if (g_edgeTable[cubeIndex] &    8) intersectionListN[ 3] = vertexInterpN(p0, p3, n0, n3);
                    if (g_edgeTable[cubeIndex] &   16) intersectionListN[ 4] = vertexInterpN(p4, p5, n4, n5);
                    if (g_edgeTable[cubeIndex] &   32) intersectionListN[ 5] = vertexInterpN(p5, p6, n5, n6);
                    if (g_edgeTable[cubeIndex] &   64) intersectionListN[ 6] = vertexInterpN(p6, p7, n6, n7);
                    if (g_edgeTable[cubeIndex] &  128) intersectionListN[ 7] = vertexInterpN(p4, p7, n4, n7);
                    if (g_edgeTable[cubeIndex] &  256) intersectionListN[ 8] = vertexInterpN(p0, p4, n0, n4);
                    if (g_edgeTable[cubeIndex] &  512) intersectionListN[ 9] = vertexInterpN(p1, p5, n1, n5);
                    if (g_edgeTable[cubeIndex] & 1024) intersectionListN[10] = vertexInterpN(p2, p6, n2, n6);
                    if (g_edgeTable[cubeIndex] & 2048) intersectionListN[11] = vertexInterpN(p3, p7, n3, n7);

                    // create the triangles
                    for (int i = 0; g_triTable[cubeIndex][i] != -1; i += 3)
                    {
                        vertex3_t v0 = intersectionListN[g_triTable[cubeIndex][i+2]];
                        vertex3_t v1 = intersectionListN[g_triTable[cubeIndex][i+1]];
                        vertex3_t v2 = intersectionListN[g_triTable[cubeIndex][i  ]];
                        m_isoSurface[m_nTriangles].p0 = v0.pos;
                        m_isoSurface[m_nTriangles].p1 = v1.pos;
                        m_isoSurface[m_nTriangles].p2 = v2.pos;
                        m_isoSurface[m_nTriangles].n0 = glm::normalize(v0.normal);
                        m_isoSurface[m_nTriangles].n1 = glm::normalize(v1.normal);
                        m_isoSurface[m_nTriangles].n2 = glm::normalize(v2.normal);
                        m_nTriangles += 1;
                    }
                }
            }
        }
        m_vaoIsoSurface->getVertexBuffer()->updateBufferData(m_isoSurface, 
            sizeof(triangle_t) * m_nTriangles);
    }
    //-----------------------------------------------------------------------------------
    vertex3_t ScalarField3D::vertexInterpN(const glm::vec4& _p0, const glm::vec4& _p1, 
                                           const glm::vec3& _n0, const glm::vec3& _n1)
    {
        static float m;
        m = (m_isoLevel - _p0.w) / (_p1.w - _p0.w);
        glm::vec3 p = {
            _p0.x + m * (_p1.x - _p0.x),
            _p0.y + m * (_p1.y - _p0.y),
            _p0.z + m * (_p1.z - _p0.z),            
        };
        glm::vec3 n = {
            _n0.x + m * (_n1.x - _n0.x),
            _n0.y + m * (_n1.y - _n0.y),
            _n0.z + m * (_n1.z - _n0.z),            
        };
        return vertex3_t(p, n);
    }
    //-----------------------------------------------------------------------------------
    glm::vec3 ScalarField3D::vertexInterp(const glm::vec4& _p0, const glm::vec4& _p1)
    {
        static float m;
        m = (m_isoLevel - _p0.w) / (_p1.w - _p0.w);
        glm::vec3 p = {
            _p0.x + m * (_p1.x - _p0.x),
            _p0.y + m * (_p1.y - _p0.y),
            _p0.z + m * (_p1.z - _p0.z),            
        };
        return p;
    }
    //-----------------------------------------------------------------------------------
    void ScalarField3D::setAxes()
    {
        std::vector<glm::vec3> lines;
        float s = m_origin * m_cellSize + 0.2f;

        lines.push_back(glm::vec3(-s, -s, -s));
        lines.push_back(glm::vec3( s, -s, -s));
        lines.push_back(glm::vec3(-s,  s, -s));
        lines.push_back(glm::vec3( s,  s, -s));
        lines.push_back(glm::vec3(-s, -s,  s));
        lines.push_back(glm::vec3( s, -s,  s));
        lines.push_back(glm::vec3(-s,  s,  s));
        lines.push_back(glm::vec3( s,  s,  s));

        lines.push_back(glm::vec3(-s, -s, -s));
        lines.push_back(glm::vec3(-s,  s, -s));
        lines.push_back(glm::vec3( s, -s, -s));
        lines.push_back(glm::vec3( s,  s, -s));
        lines.push_back(glm::vec3(-s, -s,  s));
        lines.push_back(glm::vec3(-s,  s,  s));
        lines.push_back(glm::vec3( s,  s,  s));
        lines.push_back(glm::vec3( s, -s,  s));

        lines.push_back(glm::vec3(-s, -s, -s));
        lines.push_back(glm::vec3(-s, -s,  s));
        lines.push_back(glm::vec3(-s,  s, -s));
        lines.push_back(glm::vec3(-s,  s,  s));
        lines.push_back(glm::vec3( s, -s,  s));
        lines.push_back(glm::vec3( s, -s, -s));
        lines.push_back(glm::vec3( s,  s,  s));
        lines.push_back(glm::vec3( s,  s, -s));

        MeshCreator::createDebugLines(lines, "axes");
        MeshCreator::setDebugRenderColor({ 1.0f, 1.0f, 1.0f });
    }
}




    //-----------------------------------------------------------------------------------
    /*
    void ScalarField3D::updateIsoSurfaceThreaded()
    {
        memset(m_isoSurface, 0, sizeof(triangle_t) * m_nScalars * m_maxTrisPerCell);
        
        uint32_t n_threads = ThreadPool::get().threadCount() + 1;
        // thread for every z
        uint32_t total_n = m_resolution - 1;
        uint32_t n_per_thread = total_n / n_threads;
        uint32_t remainder = total_n % n_threads;

        // synchronization
        std::vector<std::future<int>> v_threads_finished(n_threads);
        uint32_t i_from = 0;
        uint32_t i_to = n_per_thread;

        // spin up a new threads handling some z range
        for (uint32_t i = 0; i < n_threads; i++)
        {
            // we need the std::future for later collection of all threads
            std::future<int> n_triangles = ThreadPool::get().submit(std::bind(
                [&](const uint32_t _i_from, uint32_t _i_to, triangle_t* _data)
                {
                    // Lambda function with the desired z range and a pointer to the data.
                    // No modifications of elements not within this range (which are unique to
                    // this thread) will be done, hence no need for a std::lock_guard.
                    // 
                    int n_triangles = 0;
                    glm::vec3 intersectionList[12];
                    for (int z = _i_from; z < _i_to; z++)
                    {
                        for (int y = 0; y < m_resolution - 1; y++)
                        {
                            for (int x = 0; x < m_resolution - 1; x++)
                            {
                                uint32_t index_r0 = (z    ) * m_resolution2 + (y    ) * m_resolution + (x    );
                                uint32_t index_r2 = (z + 1) * m_resolution2 + (y    ) * m_resolution + (x + 1);
                                uint32_t index_r4 = (z    ) * m_resolution2 + (y + 1) * m_resolution + (x    );
                                uint32_t index_r6 = (z + 1) * m_resolution2 + (y + 1) * m_resolution + (x + 1);
                                uint32_t index_r1 = index_r0 + 1;
                                uint32_t index_r3 = index_r2 - 1;
                                uint32_t index_r5 = index_r4 + 1;
                                uint32_t index_r7 = index_r6 - 1;
                                // vertex positions for this cell
                                glm::vec4 p0 = m_fieldVertices[index_r0].pos;
                                glm::vec4 p1 = m_fieldVertices[index_r1].pos;
                                glm::vec4 p2 = m_fieldVertices[index_r2].pos;
                                glm::vec4 p3 = m_fieldVertices[index_r3].pos;
                                glm::vec4 p4 = m_fieldVertices[index_r4].pos;
                                glm::vec4 p5 = m_fieldVertices[index_r5].pos;
                                glm::vec4 p6 = m_fieldVertices[index_r6].pos;
                                glm::vec4 p7 = m_fieldVertices[index_r7].pos;
                                // check iso values and set lookup value
                                int cubeIndex = 0;
                                if (p0.w < m_isoLevel) cubeIndex |=   1;
                                if (p1.w < m_isoLevel) cubeIndex |=   2;
                                if (p2.w < m_isoLevel) cubeIndex |=   4;
                                if (p3.w < m_isoLevel) cubeIndex |=   8;
                                if (p4.w < m_isoLevel) cubeIndex |=  16;
                                if (p5.w < m_isoLevel) cubeIndex |=  32;
                                if (p6.w < m_isoLevel) cubeIndex |=  64;
                                if (p7.w < m_isoLevel) cubeIndex |= 128;
                                // trivial case, either all inside or all outside surface
                                if (g_edgeTable[cubeIndex] == 0) continue;
                                // find vertices where the surface intercept this cell
                                if (g_edgeTable[cubeIndex] &    1) intersectionList[ 0] = vertexInterp(p0, p1);
                                if (g_edgeTable[cubeIndex] &    2) intersectionList[ 1] = vertexInterp(p1, p2);
                                if (g_edgeTable[cubeIndex] &    4) intersectionList[ 2] = vertexInterp(p2, p3);
                                if (g_edgeTable[cubeIndex] &    8) intersectionList[ 3] = vertexInterp(p0, p3);
                                if (g_edgeTable[cubeIndex] &   16) intersectionList[ 4] = vertexInterp(p4, p5);
                                if (g_edgeTable[cubeIndex] &   32) intersectionList[ 5] = vertexInterp(p5, p6);
                                if (g_edgeTable[cubeIndex] &   64) intersectionList[ 6] = vertexInterp(p6, p7);
                                if (g_edgeTable[cubeIndex] &  128) intersectionList[ 7] = vertexInterp(p4, p7);
                                if (g_edgeTable[cubeIndex] &  256) intersectionList[ 8] = vertexInterp(p0, p4);
                                if (g_edgeTable[cubeIndex] &  512) intersectionList[ 9] = vertexInterp(p1, p5);
                                if (g_edgeTable[cubeIndex] & 1024) intersectionList[10] = vertexInterp(p2, p6);
                                if (g_edgeTable[cubeIndex] & 2048) intersectionList[11] = vertexInterp(p3, p7);
                                // create the triangles
                                // This here won't work for threading, since the indexing into the 
                                // m_isoSurface array is similar to std::vector::push_back.
                                // IF I want parallelism, this needs to be fixed! TODO ?
                                // 
                                for (int i = 0; g_triTable[cubeIndex][i] != -1; i += 3)
                                {
                                    glm::vec3 p0 = intersectionList[g_triTable[cubeIndex][i+2]];
                                    glm::vec3 p1 = intersectionList[g_triTable[cubeIndex][i+1]];
                                    glm::vec3 p2 = intersectionList[g_triTable[cubeIndex][i  ]];
                                    m_isoSurface[n_triangles].p0 = p0;
                                    m_isoSurface[n_triangles].p1 = p1;
                                    m_isoSurface[n_triangles].p2 = p2;
                                    glm::vec3 normal = glm::triangleNormal(p0, p1, p2);
                                    m_isoSurface[n_triangles].n0 = normal;
                                    m_isoSurface[n_triangles].n1 = normal;
                                    m_isoSurface[n_triangles].n2 = normal;
                                    n_triangles += 1;
                                }
                            }
                        }
                    }
                    // return true when finished, to be collected before GPU buffer update.
                    return n_triangles;
                }, i_from, i_to, m_isoSurface
            ));

            i_from += n_per_thread;
            if (i == n_threads - 1)
                i_to = i_from + n_per_thread + remainder;
            else
                i_to += n_per_thread;

            v_threads_finished[i] = std::move(n_triangles);
        }
  
        // collect futures
        m_nTriangles = 0;
        for (uint32_t i = 0; i < n_threads; i++)
            m_nTriangles += v_threads_finished[i].get();

        //printf("number of tris for surface = %d\n", index_tri);
        m_vaoIsoSurface->getVertexBuffer()->updateBufferData(m_isoSurface, 
            sizeof(triangle_t) * m_nTriangles);
    }
*/