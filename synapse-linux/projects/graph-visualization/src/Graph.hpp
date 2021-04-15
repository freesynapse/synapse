#ifndef __GRAPH_H
#define __GRAPH_H

#include <iostream>
#include <vector>
#include <fstream>

#include <Synapse/Core.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Renderer/Camera/Camera.hpp>
#include <Synapse/Renderer/Buffers/Framebuffer.hpp>
#include <Synapse/Renderer/Mesh/MeshShape.hpp>


namespace Syn 
{

    typedef struct node_vertex_
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        node_vertex_() {}
        node_vertex_(const glm::vec3& _p, const glm::vec3& _n, const glm::vec2& _uv) : 
            pos(_p), normal(_n), uv(_uv) {}
    } node_vertex;

    constexpr float NR = 0.5f;
    constexpr glm::vec3 NORMAL { 0.0f, 0.0f, -1.0f };

    class Node
    {
    public:
        Node() {}
        node_vertex getVertex_n(int _n) { return node_vertex(pos[_n], normal[_n], uv[_n]); }
        void setCenter(const glm::vec3& _c)
        {   
            center = _c;
            // set positions and all coordinates
            pos[0] = _c + glm::vec3(-NR, -NR,  0.0f);   normal[0] = NORMAL;     uv[0] = glm::vec2(0.0f, 0.0f);
            pos[1] = _c + glm::vec3( NR, -NR,  0.0f);   normal[1] = NORMAL;     uv[1] = glm::vec2(1.0f, 0.0f);
            pos[2] = _c + glm::vec3( NR,  NR,  0.0f);   normal[2] = NORMAL;     uv[2] = glm::vec2(1.0f, 1.0f);
            pos[3] = _c + glm::vec3(-NR,  NR,  0.0f);   normal[3] = NORMAL;     uv[3] = glm::vec2(0.0f, 1.0f);
        }
    public:
        glm::vec3 center;
        glm::vec3 pos[4];
        glm::vec3 normal[4];
        glm::vec2 uv[4];
        float cameraDistance;
        bool operator<(Node& that) { return this->cameraDistance > that.cameraDistance; }
    };


    class Graph
    {
    public:
        Graph(const Ref<Framebuffer>& _render_buffer) :
            m_fboRender(_render_buffer) {}
        ~Graph() {}
    
        void loadGraphFromFile(const std::string& _filename);
        void render(const Ref<Camera>& _camera);

    private:
        void prepareRendering();
        void prepareRenderingDebug();

    public:
        uint32_t getNodeCount() { return m_nNodes; }
        uint32_t getEdgeCount() { return m_nEdges; }

    public:
        std::vector<float> nodes;
        std::vector<float> edges;
        std::vector<Node> m_nodes;

    private:
        uint32_t m_nNodes, m_nEdges, m_nDimensions;
        
        Ref<Framebuffer> m_fboRender = nullptr;
        glm::vec2 m_resolution;
        glm::vec2 m_invResolution;


        Ref<VertexArray> m_vaoNodes, m_vaoEdges;
        Ref<VertexBuffer> m_vboNodes, m_vboEdges;
        Ref<Shader> m_nodeShader, m_edgeShader;
        
    };

}


#endif //__GRAPH_H
