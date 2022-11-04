
#pragma once

#include <glm/glm.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Memory/MemoryTypes.hpp>
#include "../FigureBase.h"


namespace Syn
{
    namespace mplc
    {
        class Canvas2D
        {
        public:
            friend class Figure;
            
        public:
            Canvas2D() {}
            virtual ~Canvas2D() {}

            //
            virtual void setData() = 0;
            virtual void redraw() = 0;

            virtual const Ref<VertexArray>& vao() const { return m_vaoData; }
            virtual const uint32_t& vertexCount() const { return m_vertexCount; }
            virtual const GLenum& OpenGLPrimitive() const { return m_OpenGLPrimitive; }

            //
            virtual const glm::vec2& dataLimX() const { return m_dataLimX; }
            virtual const glm::vec2& dataLimY() const { return m_dataLimY; }

        protected:
            Figure* m_parentRawPtr = nullptr;
            
            glm::vec2 m_dataLimX = { 0.0f, 1.0f };
            glm::vec2 m_dataLimY = { 0.0f, 1.0f };

            uint32_t m_vertexCount = 0;
            Ref<VertexArray> m_vaoData = nullptr;
            GLenum m_OpenGLPrimitive = GL_TRIANGLES;

        };
    }
}
