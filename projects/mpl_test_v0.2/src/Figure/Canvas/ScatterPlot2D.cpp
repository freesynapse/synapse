
#include "ScatterPlot2D.h"

#include "../FigureParameters.h"
#include "../FigureUtils.h"


namespace Syn
{
    namespace mplc
    {
        ScatterPlot2D::ScatterPlot2D(Figure* _parent, 
                                     const std::vector<float>& _X, 
                                     const std::vector<float>& _Y)
        {
            m_parentRawPtr = _parent;

            m_dataX = std::vector<float>(_X);
            m_dataY = std::vector<float>(_Y);
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::setData()
        {
            // limits
            m_dataLimX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            m_dataLimY = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            
            // x and y guaranteed to be of equal length
            SYN_CORE_ASSERT(m_dataX.size() == m_dataY.size(), "X and Y data must be of equal size.");

            for (size_t i = 0; i < m_dataX.size(); i++)
            {
                m_dataLimX[0] = std::min(m_dataLimX[0], m_dataX[i]);
                m_dataLimX[1] = std::max(m_dataLimX[1], m_dataX[i]);
                m_dataLimY[0] = std::min(m_dataLimY[0], m_dataY[i]);
                m_dataLimY[1] = std::max(m_dataLimY[1], m_dataY[i]);
            }
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot2D::redraw()
        {
            std::vector<glm::vec2> V;                                    // TODO : !!!
            normalized_params_t params = normalized_params_t(m_parentRawPtr->paramsPtr());    // alternate constructor needed or something -- everything is calculated twice now, 
                                                                                // once here and once in FigureRenderObj.
            
            //float x_plot_min = params.canvas_origin.x + params.data_axis_offset.x;
            //float x_plot_max = params.canvas_origin.x + params.x_axis_length - params.data_axis_offset.x;
            //nice_scale x_tick_params(m_parentRawPtr->dataLimX(), params.x_tick_count);
            //range_converter x_converter({ x_tick_params.lower_bound, x_tick_params.upper_bound },
            //                            { x_plot_min, x_plot_max });
            //NiceScale x_tick_params(m_parentRawPtr->dataLimX());
            //RangeConverter x_converter({ x_tick_params.nice_lim[0], x_tick_params.nice_lim[1] },
            //                           { x_plot_min, x_plot_max });

            //float y_plot_min = params.canvas_origin.y + params.data_axis_offset.y;
            //float y_plot_max = params.data_height;
            //NiceScale y_tick_params(m_parentRawPtr->dataLimY());
            //RangeConverter y_converter({ y_tick_params.lower_bound, y_tick_params.upper_bound },
            //                           { y_plot_min, y_plot_max });
            printf("dataLimX : %f, %f\n", m_parentRawPtr->dataLimX().x, m_parentRawPtr->dataLimX().y);
            printf("dataLimY : %f, %f\n", m_parentRawPtr->dataLimY().x, m_parentRawPtr->dataLimY().y);

            std::vector<glm::vec2> vertices;
            size_t marker_vertex_count = figureMarkerVertices(&params, vertices);
            auto scaler = m_parentRawPtr->axesScalerPtr();
            scaler->x_ticks().__debug_print("X ticks");
            scaler->y_ticks().__debug_print("Y ticks");
            //
            for (size_t i = 0; i < m_dataX.size(); i++)
            {
                // point center
                //float x = x_converter.eval(m_dataX[i]);
                //float y = y_converter.eval(m_dataY[i]);
                float x = scaler->eval_x(m_dataX[i]);
                float y = scaler->eval_y(m_dataY[i]);
                // set vertices
                for (size_t j = 0; j < marker_vertex_count; j++)
                    V.push_back({ x + vertices[j].x, y + vertices[j].y });
            }

            m_vertexCount = static_cast<uint32_t>(V.size());

            //
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position "},
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * m_vertexCount);

            m_vaoData = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
    }
}