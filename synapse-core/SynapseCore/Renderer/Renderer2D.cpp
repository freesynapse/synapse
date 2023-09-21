
#include "../../pch.hpp"

#include "Renderer2D.hpp"
#include "Renderer.hpp"
#include "../Debug/Profiler.hpp"
#include "./Buffers/VertexArray.hpp"
#include "./Buffers/VertexBuffer.hpp"
#include "./Buffers/IndexBuffer.hpp"


namespace Syn {

    struct SpriteVertex
    {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec4 color;

        //float texIndex;
        //float tilingFactor;
    };

    // a sprite batch
    struct Renderer2DData
    {
        static const uint32_t maxSprites = 10000;
        static const uint32_t maxVertices = maxSprites * 4;
        static const uint32_t maxIndices = maxSprites * 6;
        // TODO: get render capabilities from OpenGL
        static const uint32_t maxTexSlots = 32;

        Ref<VertexArray> spriteVertexArray;
        Ref<VertexBuffer> spriteVertexBuffer;
        Ref<IndexBuffer> spriteIndexBuffer;
        Ref<Shader> textureShader;
        Ref<Texture2D> defaultTexture;

        uint32_t spriteIndexCount = 0;
        SpriteVertex* spriteVertexBufferBase = nullptr;
        SpriteVertex* spriteVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, maxTexSlots> textureSlots;
        uint32_t textureSlotIndex = 1;  // 0 = default texture

        glm::vec4 spriteVertexPositions[4];
        
        Renderer2D::Statistics statistics;

        // ------ TEMPORARY -------
        SpriteVertex spriteVertices[4] = 
        {
            glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
            glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
            glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
            glm::vec3(-0.5, 0.5f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 
        };
        uint32_t spriteIndices[6] = { 0, 1, 2, 2, 3, 0};

        Ref<Shader> shader;
        // ------ TEMPORARY -------


        // copy of VP-matrix from camera on beginScene().
        glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);


    };

    // create a Renderer2DData instatiation
    static Renderer2DData s_data;
    bool Renderer2D::m_initalized = false;

	//-----------------------------------------------------------------------------------
    void Renderer2D::init()
    {
        SYN_PROFILE_FUNCTION();

        // vertex buffer
        s_data.spriteVertexBuffer = MakeRef<VertexBuffer>(GL_DYNAMIC_DRAW);
        s_data.spriteVertexBuffer->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
                { VERTEX_ATTRIB_LOCATION_UV, ShaderDataType::Float2, "a_uv" },
                { VERTEX_ATTRIB_LOCATION_COLOR, ShaderDataType::Float4, "a_color" }
            });
        s_data.spriteVertexBuffer->setData((void*)s_data.spriteVertices, sizeof(SpriteVertex) * 4);

        // index buffer
        s_data.spriteIndexBuffer = MakeRef<IndexBuffer>(GL_TRIANGLES, GL_DYNAMIC_DRAW);
        s_data.spriteIndexBuffer->setData((void*)s_data.spriteIndices, 6);

        // vertex array
        s_data.spriteVertexArray = MakeRef<VertexArray>(s_data.spriteVertexBuffer, s_data.spriteIndexBuffer);

        //
        Renderer::executeRenderCommands();

        m_initalized = true;
        SYN_CORE_TRACE("Renderer2D ready.");

    }


	//-----------------------------------------------------------------------------------
	void Renderer2D::shutdown()
    {
        SYN_PROFILE_FUNCTION();

    }


    //-----------------------------------------------------------------------------------
    void Renderer2D::beginScene(const Syn::Ref<Syn::OrthographicCamera>& _camera)
    {
        SYN_PROFILE_FUNCTION();
        #ifdef DEBUG_RENDERER_2D
            if (!m_initalized) { SYN_CORE_FATAL_ERROR("2D renderer not initialized"); }
        #endif

        s_data.viewProjectionMatrix = _camera->getViewProjectionMatrix();

    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::endScene()
    {
        SYN_PROFILE_FUNCTION();

    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::setShader(const Ref<Shader>& _shader)
    {
        SYN_PROFILE_FUNCTION();
        #ifdef DEBUG_RENDERER_2D
            if (!m_initalized) { SYN_CORE_FATAL_ERROR("2D renderer not initialized"); }
        #endif

        s_data.shader = _shader;

        s_data.shader->enable();
        
        // the view-projection matrix is set in Renderer2D::beginScene(_camera_ref)
        s_data.shader->setMatrix4fv("u_view_projection_matrix", s_data.viewProjectionMatrix);
        
    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::renderSprite(const glm::vec3 &_pos, const glm::vec2& _size, const glm::vec4& _color)
    {
        SYN_PROFILE_FUNCTION();

        // compute model transform matrix and upload to GPU
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), _pos) *
            glm::scale(glm::mat4(1.0f), {_size.x, _size.y, 0.0f});
        
        s_data.shader->setMatrix4fv("u_model_matrix", transform);

        // issue the draw call
        s_data.spriteVertexArray->bind();
        Renderer::drawIndexed(s_data.spriteVertexArray->getIndexCount(), false, GL_TRIANGLES);
        
        #ifdef DEBUG_RENDERER_2D
            s_data.statistics.drawCalls++;
        #endif

    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::renderSprite(const glm::vec2 &_pos, const glm::vec2& _size, const glm::vec4& _color)
    {
        renderSprite(glm::vec3(_pos, 0.0f), _size, _color);
    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::renderSprite(const glm::vec3& _pos, const glm::vec2& _size, const Ref<Texture2D>& _texture)
    {
        SYN_PROFILE_FUNCTION();

        // compute model transform
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), _pos) *
            glm::scale(glm::mat4(1.0f), {_size.x, _size.y, 0.0f});
        
        s_data.shader->setMatrix4fv("u_model_matrix", transform);

        // bind texture
        _texture->bind();

        // issue the draw call
        s_data.spriteVertexArray->bind();
        Renderer::drawIndexed(s_data.spriteVertexArray->getIndexCount(), false, GL_TRIANGLES);
        
        #ifdef DEBUG_RENDERER_2D
            s_data.statistics.drawCalls++;
            s_data.statistics.spriteCount++;
        #endif

    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::renderSprite(const glm::vec2& _pos, const glm::vec2& _size, const Ref<Texture2D>& _texture)
    {
        renderSprite(_pos, _size, _texture);
    }


	//-----------------------------------------------------------------------------------
    void Renderer2D::resetStatistics()
    {
        memset(&s_data.statistics, 0, sizeof(Statistics));
    }


	//-----------------------------------------------------------------------------------
    Renderer2D::Statistics Renderer2D::getStatistics()
    {
        return s_data.statistics;
    }


}




