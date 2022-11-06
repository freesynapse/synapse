
#include "ScatterPlot2D.h"

#include "../FigureParameters.h"
#include "../FigureUtils.h"


namespace Syn
{
    namespace mplc
    {
        ScatterPlot2D::ScatterPlot2D(Figure* _parent, 
                                     const std::vector<float>& _X, 
                                     const std::vector<float>& _Y,
                                     const std::string& _scatter_id)
        {
            m_parentRawPtr = _parent;
            // copy figure parameters
            memcpy(&m_canvasParameters, m_parentRawPtr->paramsRawPtr(), sizeof(figure_params_t));
            set_canvas_id(_scatter_id);
            m_dataX = std::vector<float>(_X);
            m_dataY = std::vector<float>(_Y);
            m_OpenGLPrimitive = GL_TRIANGLES;
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::setData()
        {
            // x and y guaranteed to be of equal length
            SYN_CORE_ASSERT(m_dataX.size() == m_dataY.size(), "X and Y data must be of equal size.");

            // limits initialized in Canvas2D constructor
            for (size_t i = 0; i < m_dataX.size(); i++)
            {
                m_dataLimX[0] = std::min(m_dataLimX[0], m_dataX[i]);
                m_dataLimX[1] = std::max(m_dataLimX[1], m_dataX[i]);
                m_dataLimY[0] = std::min(m_dataLimY[0], m_dataY[i]);
                m_dataLimY[1] = std::max(m_dataLimY[1], m_dataY[i]);
            }
            m_parentRawPtr->updateDataLimits();
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::data(const std::vector<float>& _Y)
        {
            m_dataY = std::vector<float>(_Y);
            m_dataX = std::vector<float>(_Y.size());
            // create X vector
            for (size_t i = 0; i < _Y.size(); i++)
                m_dataX.push_back(static_cast<float>(i));
            setData();
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::data(const std::vector<float>& _X, const std::vector<float>& _Y)
        {
            m_dataX = std::vector<float>(_X);
            m_dataY = std::vector<float>(_Y);
            setData();
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::redraw()
        {
            std::vector<glm::vec3> V;                                    // TODO : !!!
            //normalized_params_t params = normalized_params_t(m_parentRawPtr->paramsPtr());    // alternate constructor needed or something -- everything is calculated twice now, 
                                                                                // once here and once in FigureRenderObj.
            normalized_params_t params = normalized_params_t(m_canvasParameters);
            
            std::vector<glm::vec2> vertices;
            size_t marker_vertex_count = figureMarkerVertices(&params, vertices);
            auto& scaler = m_parentRawPtr->axesScalerPtr();
            //
            for (size_t i = 0; i < m_dataX.size(); i++)
            {
                // point center
                float x = scaler->eval_x(m_dataX[i]);
                float y = scaler->eval_y(m_dataY[i]);
                // set vertices
                for (size_t j = 0; j < marker_vertex_count; j++)
                    V.push_back({ x + vertices[j].x, y + vertices[j].y, params.z_value_data });
            }

            m_vertexCount = static_cast<uint32_t>(V.size());

            //
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position "},
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec3) * m_vertexCount);

            m_vaoData = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::render(const Ref<Shader>& _shader)
        {
            static auto& renderer = Renderer::get();

            _shader->setUniform4fv("u_color", m_canvasParameters.stroke_color);
            m_vaoData->bind();
            renderer.drawArrays(m_vertexCount, 0, true, m_OpenGLPrimitive);
        }
    }
}