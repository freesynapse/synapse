
#include "ScatterPlotRenderObj.h"
#include "ScatterPlot.h"

#include "../FigureUtils.h"


namespace Syn
{
    namespace mplc
    {
        ScatterPlotRenderObj::ScatterPlotRenderObj(ScatterPlot* _parent, const glm::vec2& _fig_sz)
        {
            m_parentRawPtr = _parent;
            m_figureSz = _fig_sz;

            // create the frambuffer
            this->initializeBaseClass();

            // set title, this requires access to the parent class
            this->m_figureTitle = m_parentRawPtr->m_figureTitle;
            this->m_figureParamsPtr->figure_type = FigureType::ScatterPlot;

            // Scatterplot-specific
            m_dataOpenGLPrimitive = GL_TRIANGLES;
        }
        //-------------------------------------------------------------------------------
        void ScatterPlotRenderObj::redrawData(normalized_params_t* _fig_params)
        {
            std::vector<glm::vec2> V;
            
            // find limits of value of all Y values
            m_dataLimY = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            m_dataLimY[0] = *std::min_element(m_parentRawPtr->m_dataY.begin(), m_parentRawPtr->m_dataY.end());
            m_dataLimY[1] = *std::max_element(m_parentRawPtr->m_dataY.begin(), m_parentRawPtr->m_dataY.end());

            float x_plot_min = _fig_params->canvas_origin.x + _fig_params->data_axis_offset.x;
            float x_plot_max = _fig_params->canvas_origin.x + _fig_params->x_axis_length - _fig_params->data_axis_offset.x;
            range_converter x_converter(m_dataLimX, { x_plot_min, x_plot_max });

            float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
            float y_plot_max = _fig_params->data_height;
            range_converter y_converter(m_dataLimY, { y_plot_min, y_plot_max });

            auto& x_data = m_parentRawPtr->m_dataX;
            auto& y_data = m_parentRawPtr->m_dataY;

            /*
            float theta = M_PI * 0.13f;
            glm::mat2x2 rot_mat = 
            {
                cos(theta), sin(theta),
               -sin(theta), cos(theta)
            };

            printf(" cos(theta) = %f\n", cos(theta)); 
            printf("-sin(theta) = %f\n", -sin(theta)); 
            printf(" sin(theta) = %f\n", sin(theta)); 
            printf(" cos(theta) = %f\n", cos(theta)); 

            glm::vec2 pos[4] = 
            {
                {-1,  0},
                { 0, -1},
                { 1,  0},
                { 0,  1},
            };
            for (int i = 0; i < 4; i++)
            {
                glm::vec2 r = rot_mat * pos[i];
                printf("( %.3f, %.3f ) -> ( %.3f, %.3f )\n", pos[i].x, pos[i].y, r.x, r.y);
            }
            */
            std::vector<glm::vec2> vertices;
            _fig_params->scatter_marker = ScatterPlotMarker::Diamond;
            size_t marker_vertex_count = scatterPlotMarkerVertices(_fig_params, vertices);
            for (size_t i = 0; i < y_data.size(); i++)
            {
                float x = x_converter.eval(x_data[i]);
                float y = y_converter.eval(y_data[i]);

                for (size_t j = 0; j < marker_vertex_count; j++)
                {
                    V.push_back({ x+vertices[j].x, y+vertices[j].y });

                }
            }

            m_dataVertexCount = static_cast<uint32_t>(V.size());

            //
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * m_dataVertexCount);

            m_vaoData = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void ScatterPlotRenderObj::fillBetweenX(const float& _x0, const float& _x1)
        {

        }
        //-------------------------------------------------------------------------------
        void ScatterPlotRenderObj::fillBetweenY(const float& _y0, const float& _y1)
        {

        }

    }
}
