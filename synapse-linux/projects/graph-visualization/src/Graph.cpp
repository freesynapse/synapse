
#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Renderer/Renderer.hpp>
#include <Synapse/Renderer/Shader/ShaderLibrary.hpp>
#include <Synapse/Debug/Profiler.hpp>
#include <Synapse/Renderer/MeshCreator.hpp>

#include "Graph.hpp"


namespace Syn
{
    void Graph::loadGraphFromFile(const std::string& _filename)
    {
        SYN_PROFILE_FUNCTION();

        std::ifstream fs(_filename, std::ios::in | std::ios::binary);
        if (!fs)
        {
            SYN_CORE_FATAL_ERROR("File '", _filename, "' does not exist.");
            return;
        }

        // read number of nodes and edges and allocate mem
        int nnodes, nedges;
        fs.read((char*)&m_nDimensions, sizeof(uint32_t));
        fs.read((char*)&nnodes, sizeof(uint32_t));
        fs.read((char*)&nedges, sizeof(uint32_t));

        // assume 3 dimensions
        SYN_CORE_TRACE("dimensions = ", m_nDimensions);
        if (m_nDimensions != 3)
        {
            SYN_CORE_WARNING("invalid dimensionality.")
            m_nNodes = m_nEdges = 0;
        }
        m_nNodes = nnodes / 3;
        m_nEdges = nedges / 3;

        // clean slate
        nodes = std::vector<float>(nnodes);
        edges = std::vector<float>(nedges);

        SYN_CORE_TRACE("loading ", m_nNodes, " vertices and ", m_nEdges, " edges from file.");

        fs.read((char*)&(nodes[0]), sizeof(float) * nnodes);
        fs.read((char*)&(edges[0]), sizeof(float) * nedges);

        fs.close();

        // scale input
        float scale = 30.0f;
        for (size_t i = 0; i < nnodes; i++)
            nodes[i] *= scale;
        for (size_t i = 0; i < nedges; i++)
            edges[i] *= scale;

        //
        SYN_CORE_TRACE("prearing graph layout for rendering.");

        // repack into objects and prepare GPU upload
        std::vector<node_vertex> vertices;
        //std::vector<uint32_t> indices;
        int idx;
        for (auto i = 0; i < m_nNodes; i++)
        {
            idx = i * 3;
            glm::vec3 p { nodes[idx], nodes[idx+1], nodes[idx+2] };
            
            Node node;
            node.setCenter(p);
            m_nodes.push_back(node);            
        }

        std::sort(m_nodes.begin(), m_nodes.end());

        for (auto& node : m_nodes)
        {
            // repack for GPU
            vertices.push_back(node.getVertex_n(0));
            vertices.push_back(node.getVertex_n(1));
            vertices.push_back(node.getVertex_n(2));

            vertices.push_back(node.getVertex_n(2));
            vertices.push_back(node.getVertex_n(3));
            vertices.push_back(node.getVertex_n(0));
        }

        m_vboNodes = API::newVertexBuffer(GL_DYNAMIC_DRAW);
        m_vboNodes->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            { VERTEX_ATTRIB_LOCATION_NORMAL, ShaderDataType::Float3, "a_normal" },
            { VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" }
        });
        m_vboNodes->setData((void*)vertices.data(), sizeof(node_vertex) * vertices.size());

        m_vaoNodes = API::newVertexArray(m_vboNodes);
        //std::cout << "m_node.size = " << m_nodes.size() << "\n";
        //std::cout << "vertices.size = " << vertices.size() << "\n";
        
        // shaders
        ShaderLibrary::load("../assets/shaders/graph-visualization/graphPointShader2.glsl");
        m_nodeShader = ShaderLibrary::get("graphPointShader2");
        
        //this->prepareRendering();
        //prepareRenderingDebug();

        return;
    }

    //-----------------------------------------------------------------------------------
    void Graph::render(const Ref<Camera>& _camera)
    {
        glm::vec3 cameraPos = _camera->getPosition();

        // update distance to camera and sort
        for (auto& node : m_nodes)
            node.cameraDistance = glm::length2(node.center - cameraPos);
        std::sort(m_nodes.begin(), m_nodes.end());

        // TODO --> instancing!
        //std::vector<node_vertex> vertices(m_nNodes * 6);
        //static uint32_t size = sizeof(node_vertex) * m_nNodes * 6;
        //for (auto& node : m_nodes)
        //{
        //    vertices.push_back(node.getVertex_n(0));
        //    vertices.push_back(node.getVertex_n(1));
        //    vertices.push_back(node.getVertex_n(2));
        //    vertices.push_back(node.getVertex_n(2));
        //    vertices.push_back(node.getVertex_n(3));
        //    vertices.push_back(node.getVertex_n(0));
        //}
        //// reupload
        //m_vaoNodes->bind();
        //m_vboNodes->bind();
        //m_vboNodes->setData((void*)vertices.data(), size);
        
        // bind render buffer and render all nodes
        m_fboRender->bind();
        Renderer::setClearColor(0.045f, 0.045f, 0.31f, 1.0f);
        Renderer::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_nodeShader->enable();
        m_nodeShader->setMatrix4fv("u_view_projection_matrix", _camera->getViewProjectionMatrix());
        m_vaoNodes->bind();
        Renderer::drawArrays(m_nodes.size(), 0, true, GL_TRIANGLES);

        //m_edgeShader->enable();
        //m_edgeShader->setMatrix4fv("u_view_projection_matrix", _camera->getViewProjectionMatrix());
        //m_vaoEdges->bind();
        //Renderer::drawArrays(m_nEdges, 0, true, GL_LINES);

    }

    //-----------------------------------------------------------------------------------
    void Graph::prepareRenderingDebug()
    {
        SYN_PROFILE_FUNCTION();

        std::vector<node_vertex> vdata;
        std::vector<uint32_t> indices;
        glm::vec3 normal{ 0.0f, 0.0f, -1.0f };
        vdata.push_back(node_vertex(glm::vec3(-1.0f, -1.0f, -2.0f), normal, glm::vec2(0.0f, 0.0f)));
        vdata.push_back(node_vertex(glm::vec3( 1.0f, -1.0f, -2.0f), normal, glm::vec2(1.0f, 0.0f)));
        vdata.push_back(node_vertex(glm::vec3( 1.0f,  1.0f, -2.0f), normal, glm::vec2(1.0f, 1.0f)));
        vdata.push_back(node_vertex(glm::vec3(-1.0f,  1.0f, -2.0f), normal, glm::vec2(0.0f, 1.0f)));
        indices.push_back(0); indices.push_back(1); indices.push_back(2);
        indices.push_back(2); indices.push_back(3); indices.push_back(0);

        vdata.push_back(node_vertex(glm::vec3(-1.0f, -1.0f,  2.0f), normal, glm::vec2(0.0f, 0.0f)));
        vdata.push_back(node_vertex(glm::vec3( 1.0f, -1.0f,  2.0f), normal, glm::vec2(1.0f, 0.0f)));
        vdata.push_back(node_vertex(glm::vec3( 1.0f,  1.0f,  2.0f), normal, glm::vec2(1.0f, 1.0f)));
        vdata.push_back(node_vertex(glm::vec3(-1.0f,  1.0f,  2.0f), normal, glm::vec2(0.0f, 1.0f)));
        indices.push_back(4); indices.push_back(5); indices.push_back(6);
        indices.push_back(6); indices.push_back(7); indices.push_back(4);

        Ref<VertexBuffer> vbo0 = API::newVertexBuffer(GL_STATIC_DRAW);
        vbo0->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            { VERTEX_ATTRIB_LOCATION_NORMAL, ShaderDataType::Float3, "a_normal" },
            { VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" }
        });
        vbo0->setData((void*)vdata.data(), sizeof(node_vertex) * vdata.size());

        Ref<IndexBuffer> ibo0 = API::newIndexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
        ibo0->setData((void*)indices.data(), indices.size());
        m_vaoNodes = API::newVertexArray(vbo0, ibo0);
        
        glm::ivec2 xy = m_fboRender->getSize();
        m_resolution = glm::vec2((float)xy.x, (float)xy.y);
        m_invResolution = 1.0f / m_resolution;

        ShaderLibrary::load("../assets/shaders/graph-visualization/graphPointShader2.glsl");
        m_nodeShader = ShaderLibrary::get("graphPointShader2");

    }


    //-----------------------------------------------------------------------------------
    void Graph::prepareRendering()
    {
        SYN_PROFILE_FUNCTION();

        typedef struct vert_
        {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv;
            vert_() {}
            vert_(const glm::vec3& _p, const glm::vec3& _n, const glm::vec2& _uv) : 
                pos(_p), normal(_n), uv(_uv) {}
        } vertex;

        std::vector<vertex> vdata;
        std::vector<uint32_t> indices;
        glm::vec3 normal{ 0.0f, 0.0f, -1.0f };
        for (uint32_t i = 0; i < m_nNodes; i++)
        {
            int ix = i*3;
            glm::vec3 node = glm::vec3(nodes[ix+0], nodes[ix+1], nodes[ix+2]);
            vdata.push_back(vertex(node + glm::vec3(-0.5f, -0.5f, 0.0f), normal, glm::vec2(0.0f, 0.0f)));
            vdata.push_back(vertex(node + glm::vec3( 0.5f, -0.5f, 0.0f), normal, glm::vec2(1.0f, 0.0f)));
            vdata.push_back(vertex(node + glm::vec3( 0.5f,  0.5f, 0.0f), normal, glm::vec2(1.0f, 1.0f)));
            vdata.push_back(vertex(node + glm::vec3(-0.5f,  0.5f, 0.0f), normal, glm::vec2(0.0f, 1.0f)));
            ix = i*4;
            indices.push_back(ix+0); indices.push_back(ix+1); indices.push_back(ix+2);
            indices.push_back(ix+2); indices.push_back(ix+3); indices.push_back(ix+0);
        }

        Ref<VertexBuffer> vbo0 = API::newVertexBuffer(GL_STATIC_DRAW);
        vbo0->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            { VERTEX_ATTRIB_LOCATION_NORMAL, ShaderDataType::Float3, "a_normal" },
            { VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" }
        });
        vbo0->setData((void*)vdata.data(), sizeof(vertex) * vdata.size());

        Ref<IndexBuffer> ibo0 = API::newIndexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
        ibo0->setData((void*)indices.data(), indices.size());
        m_vaoNodes = API::newVertexArray(vbo0, ibo0);
        
        /*
        Ref<VertexBuffer> vbo1 = API::newVertexBuffer(GL_STATIC_DRAW);
        vbo1->setBufferLayout({
            { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position"},
        });
        vbo1->setData((void*)edges.data(), sizeof(float) * edges.size());
        m_vaoEdges = API::newVertexArray(vbo1);
        */
        //
        glm::ivec2 xy = m_fboRender->getSize();
        m_resolution = glm::vec2((float)xy.x, (float)xy.y);
        m_invResolution = 1.0f / m_resolution;

        ShaderLibrary::load("../assets/shaders/graph-visualization/graphPointShader.glsl");
        ShaderLibrary::load("../assets/shaders/graph-visualization/graphPointShader2.glsl");
        ShaderLibrary::load("../assets/shaders/graph-visualization/graphLineShader.glsl");
        m_nodeShader = ShaderLibrary::get("graphPointShader2");
        m_edgeShader = ShaderLibrary::get("graphLineShader");

    }

}

