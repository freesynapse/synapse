
#include "LinePlotRenderObj.h"
#include "LinePlot.h"

#include "../FigureUtils.h"


namespace Syn
{
    namespace mplc
    {
        LinePlotRenderObj::LinePlotRenderObj(LinePlot* _parent, const glm::vec2& _fig_sz)
        {
            m_parentRawPtr = _parent;
            m_figureSz = _fig_sz;

            // create the frambuffer
            this->initializeBaseClass();

            // set title, this requires access to the parent class
            this->m_figureTitle = m_parentRawPtr->m_figureTitle;
            this->m_figureParamsPtr->figure_type = FigureType::LinePlot;

            // lineplot-specific
            m_dataOpenGLPrimitive = GL_LINES;
        }
        //-------------------------------------------------------------------------------
        void LinePlotRenderObj::redrawData(normalized_params_t* _fig_params)
        {
            //std::vector<glm::vec3> V;
            std::vector<glm::vec2> V;
            
            // find limits of value of all Y values
            m_dataLimY = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            for (size_t i = 0; i < m_parentRawPtr->m_dataYDim0; i++)
            {
                auto& data = m_parentRawPtr->m_dataY[i];
                m_dataLimY[0] = std::min(m_dataLimY[0], *std::min_element(data.begin(), data.end()));
                m_dataLimY[1] = std::max(m_dataLimY[1], *std::max_element(data.begin(), data.end()));
            }

            float x_plot_min = _fig_params->canvas_origin.x + _fig_params->data_axis_offset.x;
            float x_plot_max = _fig_params->canvas_origin.x + _fig_params->x_axis_length - _fig_params->data_axis_offset.x;
            range_converter x_converter(m_dataLimX, { x_plot_min, x_plot_max });

            float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
            float y_plot_max = _fig_params->data_height;
            range_converter y_converter(m_dataLimY, { y_plot_min, y_plot_max });

            for (size_t i = 0; i < m_parentRawPtr->m_dataYDim0; i++)
            {
                auto& y_data = m_parentRawPtr->m_dataY[i];
                auto& x_data = m_parentRawPtr->m_dataX;

                for (size_t j = 1; j < y_data.size(); j++)
                {
                    float x0 = x_converter.eval(x_data[j-1]);
                    float x1 = x_converter.eval(x_data[j]);
                    float y0 = y_converter.eval(y_data[j-1]);
                    float y1 = y_converter.eval(y_data[j]);
                    //glm::vec3 v0 = { x0, y0, 1.0f };      // alpha as .z : 1 for lines from the same data 'row'
                    //glm::vec3 v1 = { x1, y1, 1.0f };      // nice idea, maybe we'll come back to this...
                    glm::vec2 v0 = { x0, y0 };
                    glm::vec2 v1 = { x1, y1 };                    
                    V.push_back(v0);
                    V.push_back(v1);
                }
            }

            m_dataVertexCount = static_cast<uint32_t>(V.size());

            //
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                //{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
            });
            //vbo->setData((void*)(&V[0]), sizeof(glm::vec3) * m_dataVertexCount);
            vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * m_dataVertexCount);

            m_vaoData = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void LinePlotRenderObj::fillBetweenX(const float& _x0, const float& _x1)
        {

        }
        //-------------------------------------------------------------------------------
        void LinePlotRenderObj::fillBetweenY(const float& _y0, const float& _y1)
        {

        }

    }
}
