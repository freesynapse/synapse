
#include "LinePlot2D.h"


namespace Syn
{
    namespace mplc
    {
        LinePlot2D::LinePlot2D(Figure* _parent, 
                               const std::vector<std::vector<float>>& _X,
                               const std::vector<std::vector<float>>& _Y,
                               const std::string& _lineplot_id)
        {
            m_parentRawPtr = _parent;
            // copy figure parameters
            memcpy(&m_canvasParameters, m_parentRawPtr->paramsRawPtr(), sizeof(figure_params_t));
            set_canvas_id(_lineplot_id);
            m_dataX = std::vector<std::vector<float>>(_X);
            m_dataY = std::vector<std::vector<float>>(_Y);
            m_OpenGLPrimitive = GL_LINES;
        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::setData()
        {
            // limits
            m_maxShape[0] = static_cast<int>(m_dataX.size());
            SYN_CORE_ASSERT(m_dataX.size() == m_dataY.size(), 
                            "number of rows of X and Y data expected to be of equal length.");
            
            for (int i = 0; i < m_maxShape[0]; i++)
            {
                SYN_CORE_ASSERT(m_dataX[i].size() == m_dataY[i].size(),
                                "length of X and Y vectors expected to match.");
                for (size_t j = 0; j < m_dataX[i].size(); j++)
                {
                    m_dataLimX[0] = std::min(m_dataLimX[0], m_dataX[i][j]);
                    m_dataLimX[1] = std::max(m_dataLimX[1], m_dataX[i][j]);
                    m_dataLimY[0] = std::min(m_dataLimY[0], m_dataY[i][j]);
                    m_dataLimY[1] = std::max(m_dataLimY[1], m_dataY[i][j]);

                    m_maxShape[1] = std::max(m_maxShape[1], static_cast<int>(m_dataX[i].size()));
                }
            }

            m_parentRawPtr->updateDataLimits();

        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::data(const std::vector<float>& _Y)
        {
            std::vector<std::vector<float>> Y;
            Y.push_back(_Y);
            data(Y);
        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::data(const std::vector<float>& _X, const std::vector<float>& _Y)
        {
            std::vector<std::vector<float>> X, Y;
            X.push_back(_X);
            Y.push_back(_Y);
            data(X, Y);
        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::data(const std::vector<std::vector<float>>& _Y)
        {
            m_dataX = std::vector<std::vector<float>>();
            m_dataY = std::vector<std::vector<float>>(_Y);
            for (size_t i = 0; i < _Y.size(); i++)
            {
                std::vector<float> X;
                for (size_t j = 0; j < _Y[i].size(); j++)
                    X.push_back(static_cast<float>(j));
                m_dataX.push_back(X);
            }
            setData();
        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::data(const std::vector<std::vector<float>>& _X, 
                              const std::vector<std::vector<float>>& _Y)
        {
            clear_data(m_dataX, m_dataY);
            m_dataX = std::vector<std::vector<float>>(_X);
            m_dataY = std::vector<std::vector<float>>(_Y);
            setData();
        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::redraw()
        {
            std::vector<glm::vec3> V;
            std::vector<glm::vec3> V_markers;
            normalized_params_t params = normalized_params_t(m_canvasParameters);
            const Ref<Axes>& axes = m_parentRawPtr->axesPtr();

            std::vector<glm::vec2> vertices;
            size_t marker_vertex_count = figureMarkerVertices(&params, vertices);

            //
            for (int m = 0; m < m_maxShape[0]; m++)
            {
                for (size_t n = 1; n < m_dataX[m].size(); n++)
                {
                    // TODO   : to increase efficiency of rendering, GL_LINESTRIP could be 
                    //          used where the 'line' connecting different data vectors
                    //          (axis 1; n) would be transparent (alpha=0.0).
                    //

                    glm::vec3 v0 = { axes->eval_x(m_dataX[m][n-1]), axes->eval_y(m_dataY[m][n-1]), params.z_value_data };
                    glm::vec3 v1 = { axes->eval_x(m_dataX[m][n  ]), axes->eval_y(m_dataY[m][n  ]), params.z_value_data };
                    V.push_back(v0);
                    V.push_back(v1);

                    // marker_vertex_count is 0 of marker == FigureMarker::None
                    for (size_t i = 0; i < marker_vertex_count; i++)
                        V_markers.push_back({ v0.x + vertices[i].x, v0.y + vertices[i].y, params.z_value_data });
                }
            }
            m_vertexCount = static_cast<uint32_t>(V.size());

            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            });
            vbo->setData((void*)&(V[0]), sizeof(glm::vec3) * m_vertexCount);

            m_vaoData = API::newVertexArray(vbo);

            //
            m_markerVertexCount = static_cast<uint32_t>(V_markers.size());
            if (m_markerVertexCount)
            {
                Ref<VertexBuffer> vbo_marker = API::newVertexBuffer(GL_STATIC_DRAW);
                vbo_marker->setBufferLayout({
                    { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
                });
                vbo_marker->setData((void*)&(V_markers[0]), sizeof(glm::vec3) * m_markerVertexCount);

                m_vaoMarkers = API::newVertexArray(vbo_marker);
            }

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void LinePlot2D::render(const Ref<Shader>& _shader)
        {
            static auto& renderer = Renderer::get();

            static float line_width = m_canvasParameters.line_width_px;
            if (line_width != 1.0f)
                renderer.setLineWidth(line_width);

            _shader->setUniform4fv("u_color", m_canvasParameters.stroke_color);
            m_vaoData->bind();
            renderer.drawArrays(m_vertexCount, 0, true, m_OpenGLPrimitive);

            if (line_width != 1.0f)
                renderer.setLineWidth(1.0f);

            if (m_vaoMarkers != nullptr)
            {
                m_vaoMarkers->bind();
                renderer.drawArrays(m_markerVertexCount, 0, true, GL_TRIANGLES);
            }
        }

    }
}