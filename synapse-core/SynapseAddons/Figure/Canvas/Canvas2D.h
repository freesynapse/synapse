
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
            friend class FigureRenderObj;
            
        public:
            Canvas2D()
            {
                m_dataLimX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
                m_dataLimY = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            }
            virtual ~Canvas2D() {}

            // Virtual accessors -- to be populated by children
            virtual void data(const std::vector<float>&) {}
            virtual void data(const std::vector<float>&, const std::vector<float>&) {}
            virtual void data(const std::vector<std::vector<float>>&) {}
            virtual void data(const std::vector<std::vector<float>>&, const std::vector<std::vector<float>>&) {}
            virtual std::pair<std::vector<float>, std::vector<float>> data() { return {}; }
            virtual void __debug_print() {}

            //
        protected:
            virtual void setData() = 0;
            virtual void redraw() = 0;
            virtual void render(const Ref<Shader>&) = 0;    // assumes enabled shader

            //
        public:
            virtual const Ref<VertexArray>& vao() const { return m_vaoData; }
            virtual const uint32_t& vertexCount() const { return m_vertexCount; }
            virtual const GLenum& OpenGLPrimitive() const { return m_OpenGLPrimitive; }

            //
            virtual const glm::vec2& dataLimX() const { return m_dataLimX; }
            virtual const glm::vec2& dataLimY() const { return m_dataLimY; }
            virtual const std::string& canvasID() const { return m_canvasID; }
            virtual const size_t size() const { return 0; };

            //
            virtual const void printCanvasParameters() { m_canvasParameters.printParameters(); }

        protected:
            /* Function for checking the validity of a canvas ID
             */
            void set_canvas_id(const std::string& _id)
            {
                // check if already present
                if (m_parentRawPtr->canvases().find(_id) != m_parentRawPtr->canvases().end())
                {
                    m_canvasID = _id + "_" + std::string(Random::rand_str(8));
                    SYN_CORE_WARNING("Canvas ID '", _id, "' already present, new ID is '", m_canvasID, "'.");
                }
                else if (_id == "")
                {
                    m_canvasID = "canvas_" + std::string(Random::rand_str(8));
                    SYN_CORE_WARNING("No Canvas ID provided, new ID is '", m_canvasID, "'.");
                }
                else
                    m_canvasID = _id;
            }
            /* Clears the X/Y data vectors
             */
            void clear_data(std::vector<float> _d0) { _d0.clear(); }
            void clear_data(std::vector<float> _d0, std::vector<float>& _d1) { _d0.clear(); _d1.clear(); }
            void clear_data(std::vector<std::vector<float>> _d0) { _d0.clear(); }
            void clear_data(std::vector<std::vector<float>> _d0, std::vector<std::vector<float>>& _d1) { _d0.clear(); _d1.clear(); }
            

        protected:
            Figure* m_parentRawPtr = nullptr;
            figure_params_t m_canvasParameters;
            std::string m_canvasID = "";
            glm::vec2 m_dataLimX = { 0.0f, 1.0f };
            glm::vec2 m_dataLimY = { 0.0f, 1.0f };

            size_t m_dataSize = 0;
            
            uint32_t m_vertexCount = 0;
            Ref<VertexArray> m_vaoData = nullptr;
            GLenum m_OpenGLPrimitive = GL_TRIANGLES;

        };
    }
}
