
#include "FigureRenderObjBase.h"

#include "FigureUtils.h"


namespace Syn
{
    namespace mplc
    {
        FigureRenderObjBase::FigureRenderObjBase()
        {
            setupStaticShaders();
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::initializeBaseClass()
        {
            m_framebufferID = "figure_" + std::string(Random::rand_str(16));
            m_framebuffer = API::newFramebuffer(ColorFormat::RGBA16F,
                                                glm::vec2(m_figureSz.x, m_figureSz.y),
                                                1, 
                                                true, 
                                                false, 
                                                m_framebufferID);

            // use default histogram rendering parameters
            m_figureParamsPtr = std::make_shared<figure_params_t>(m_figureSz);

            // fonts
            m_tickLabelFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                            m_figureParamsPtr->ticks_font_size_px,
                                            m_shaderFont,
                                            m_figureParamsPtr->figure_sz_px);        
            m_tickLabelFont->disableUpdateOnResize();
            m_tickLabelFont->setColor(m_figureParamsPtr->tick_label_color);

            m_titleFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                        m_figureParamsPtr->title_size,
                                        m_shaderFont,
                                        m_figureParamsPtr->figure_sz_px);
            m_titleFont->disableUpdateOnResize();
            m_titleFont->setColor(m_figureParamsPtr->title_color);

            //
            m_redrawFlags = FIGURE_REDRAW_ALL;
            m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::render()
        {
            if (!m_redrawFlags)
                return;
            
            SYN_CORE_ASSERT(m_figureParamsPtr->figure_type != FigureType::None, 
                            "Figure type must be set by class derived from FigureRenderObjBase");
            //
            redraw();

            static auto& renderer = Renderer::get();
            m_framebuffer->bind();
            
            renderer.setClearColor(m_figureParamsPtr->canvas_background);
            renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            //m_framebuffer->bindDefaultFramebuffer();
            //return;
            m_shader2D->enable();
            
            // render selection (optional)
            if (m_interactiveMode ||
                m_figureParamsPtr->fill_between_x || 
                m_figureParamsPtr->fill_between_y)
            {
                m_shader2D->setUniform4fv("u_color", glm::vec4(1.0f, 1.0f, 1.0f, 0.25f));
                m_vaoSelection->bind();
                renderer.drawArrays(m_selectionVertexCount, 0, true, GL_TRIANGLES);
            }

            // render data
            //if (m_figureParamsPtr->figure_type == FIGURE_TYPE_HISTOGRAM)
            //{
            m_shader2D->setUniform4fv("u_color", m_figureParamsPtr->bar_color);
            m_vaoData->bind();
            renderer.drawArrays(m_dataVertexCount, 0, true, m_dataOpenGLPrimitive);
            //}
            //else if (m_figureParamsPtr->figure_type == FIGURE_TYPE_LINEPLOT)
            //{
            //    m_shaderLinePlot2D->enable();
            //    m_shaderLinePlot2D->setUniform4fv("u_color", m_figureParamsPtr->bar_color);
            //    m_vaoData->bind();
            //    renderer.drawArrays(m_dataVertexCount, 0, true, m_dataOpenGLPrimitive);
            //    m_shader2D->enable();
            //}

            // render axes
            m_shader2D->setUniform4fv("u_color", m_figureParamsPtr->axis_color);
            m_vaoAxes->bind();
            renderer.drawArrays(m_axesVertexCount, 0, true, GL_LINES);

            // render ticks
            m_shader2D->setUniform4fv("u_color", m_figureParamsPtr->tick_color);
            m_vaoTicks->bind();
            renderer.drawArrays(m_ticksVertexCount, 0, true, GL_LINES);

            // render tick labels
            m_tickLabelFont->setColor(m_figureParamsPtr->tick_label_color);
            //float x_step = 0.0f;
            //float x = 0.0f;
            
            m_tickLabelFont->beginRenderBlock();
            for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
                m_tickLabelFont->addString(m_tickLabelPositionsY[i].x, m_tickLabelPositionsY[i].y, "%s", m_tickLabelsY[i].c_str());
            for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
                m_tickLabelFont->addString(m_tickLabelPositionsX[i].x, m_tickLabelPositionsX[i].y, "%s", m_tickLabelsX[i].c_str());
            m_tickLabelFont->endRenderBlock();
            
            // figure title
            const char* title = m_figureTitle.c_str();
            float title_height = m_titleFont->getFontHeight();
            float title_width = m_titleFont->getStringWidth("%s", title);
            glm::vec2 title_pos =
            {
                (m_figureParamsPtr->figure_sz_px.x * 0.5f) - (title_width * 0.5f),
                10.0f + title_height
            };
            m_titleFont->beginRenderBlock();
            m_titleFont->addString(title_pos.x, title_pos.y, "%s", title);
            m_titleFont->endRenderBlock();

            m_framebuffer->bindDefaultFramebuffer();
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::redraw()
        {
            // conversion of pixel parameters to normalized coordinates [0..1]
            normalized_params_t norm_params = normalized_params_t(m_figureParamsPtr);
            //
            if (m_redrawFlags & FIGURE_REDRAW_DATA)
            {
                m_dataPositions.clear();    // set by redrawData() by derived
                m_tickLabelsX.clear();
                redrawData(&norm_params);
            }
            
            if (m_redrawFlags & FIGURE_REDRAW_AXES)
                redrawAxes(&norm_params);

            if (m_redrawFlags & FIGURE_REDRAW_TICKS)
            {
                m_tickLabelPositionsX.clear();
                m_tickLabelPositionsY.clear();
                redrawTicks(&norm_params);
            }

            if (m_redrawFlags &  FIGURE_REDRAW_TICKLABELS || m_redrawFlags & FIGURE_REDRAW_DATA)
            {
                m_tickLabelsY.clear();  // X tick labels set in derived class redrawData()
                redrawTickLabels(&norm_params);
            }

            if (m_redrawFlags & FIGURE_REDRAW_SELECTION)
                redrawSelection(&norm_params);

            //
            m_redrawFlags = 0;
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::redrawAxes(normalized_params_t* _fig_params)
        {
            m_axesVertexCount = 4;
            glm::vec2 V[4] = 
            {
                // X 
                { _fig_params->x_axis_lim[0] - _fig_params->axes_neg_protrusion.x, _fig_params->canvas_origin.y },
                { _fig_params->x_axis_lim[1], _fig_params->canvas_origin.y },
                // Y
                { _fig_params->canvas_origin.x, _fig_params->y_axis_lim[0] - _fig_params->axes_neg_protrusion.y },
                { _fig_params->canvas_origin.x, _fig_params->y_axis_lim[1] },
            };
            //
            
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
            });
            vbo->setData((void*)V, sizeof(glm::vec2) * m_axesVertexCount);

            m_vaoAxes = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::redrawTicks(normalized_params_t* _fig_params)
        {
            std::vector<glm::vec2> V;

            float font_height = m_tickLabelFont->getFontHeight();

            if (_fig_params->render_x_ticks)
            {
                // x ticks for Histograms
                if (_fig_params->figure_type == FigureType::Histogram)
                {
                    float x = m_dataPositions[0].x + _fig_params->canvas_origin.x + _fig_params->data_axis_offset.x;

                    for (size_t i = 0; i < m_dataPositions.size(); i++)
                    {
                        glm::vec2 v0 = { x, _fig_params->canvas_origin.y };
                        glm::vec2 v1 = { x, _fig_params->canvas_origin.y - _fig_params->tick_length.y };
                        V.push_back(v0);
                        V.push_back(v1);            

                        // set tick label positions -- n.b.: y axis is inverted for font rendering in pixel space
                        glm::vec2 x_tick_pos = 
                        {
                            v1.x * _fig_params->figure_sz_px.x,
                            _fig_params->figure_sz_px.y - ((v1.y - _fig_params->tick_labels_offset.y) * _fig_params->figure_sz_px.y) + font_height
                        };
                        m_tickLabelPositionsX.push_back(x_tick_pos);

                        x += _fig_params->data_spacing;
                    }
                    // For histograms, the last tick should be removed since ticks are drawn in between
                    // the data, and otherwise tick label positions will be off by 1.
                    m_tickLabelPositionsX.pop_back();
                }
                // x ticks for LinePlots
                if (_fig_params->figure_type == FigureType::LinePlot)
                {
                    float x_plot_min = _fig_params->canvas_origin.x + _fig_params->data_axis_offset.x;
                    float x_plot_max = _fig_params->canvas_origin.x + _fig_params->x_axis_length - _fig_params->data_axis_offset.x;

                    nice_scale x_tick_params(m_dataLimX, _fig_params->x_tick_count);
                    range_converter x_converter({ x_tick_params.lower_bound, m_dataLimX[1] },
                                                { x_plot_min, x_plot_max });
                    float curr_x_val = x_tick_params.lower_bound;
                    float x_step = x_tick_params.tick_spacing;

                    //
                    for (size_t i = 0; i < x_tick_params.max_ticks; i++)
                    {
                        float x = x_converter.eval(curr_x_val);

                        if (x > x_plot_max)
                            break;
                        
                        glm::vec2 v0 = { x, _fig_params->canvas_origin.y };
                        glm::vec2 v1 = { x, _fig_params->canvas_origin.y - _fig_params->tick_length.y };
                        V.push_back(v0);
                        V.push_back(v1);

                        // set tick label positions -- n.b.: y axis is inverted for font rendering in pixel space
                        glm::vec2 x_tick_pos = 
                        {
                            v1.x * _fig_params->figure_sz_px.x,
                            _fig_params->figure_sz_px.y - ((v1.y - _fig_params->tick_labels_offset.y) * _fig_params->figure_sz_px.y) + font_height
                        };
                        m_tickLabelPositionsX.push_back(x_tick_pos);

                        curr_x_val += x_step;
                    }
                }
            }

            // y ticks
            if (_fig_params->render_y_ticks)
            {
                float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
                float y_plot_max = _fig_params->data_height;

                nice_scale y_tick_params(m_dataLimY, _fig_params->y_tick_count);
                //nice_scale2 y_tick_params2(m_dataLimY[0], m_dataLimY[1], _fig_params->y_tick_count);
                
                //range_converter converter({ y_tick_params.lower_bound, y_tick_params.upper_bound }, 
                //                          { y_plot_min, y_plot_max });
                range_converter converter({ y_tick_params.lower_bound, y_tick_params.upper_bound },
                                          { y_plot_min, y_plot_max });

                float label_width = m_tickLabelFont->getStringWidth("111111"); // TODO : replace this with something dynamic
                float label_height = m_tickLabelFont->getFontHeight() * 0.5f;
                float curr_y_val = y_tick_params.lower_bound;
                float y_step = y_tick_params.tick_spacing;
                //
                for (size_t i = 0; i < y_tick_params.max_ticks + 1; i++)
                {
                    float y = converter.eval(curr_y_val);
                    if (y > _fig_params->y_axis_lim[1])
                        break;

                    glm::vec2 v0 = { _fig_params->canvas_origin.x, y };
                    glm::vec2 v1 = { _fig_params->canvas_origin.x - _fig_params->tick_length.x, y }; 
                    V.push_back(v0);
                    V.push_back(v1);

                    glm::vec2 y_label_pos =
                    {
                        (v1.x - _fig_params->tick_labels_offset.x) * _fig_params->figure_sz_px.x - label_width, 
                        // font_height is further scaled down w/ 0.5 since the normalized range (0..1) is scaled up for font rendering (-1..1)
                        _fig_params->figure_sz_px.y - (v1.y * _fig_params->figure_sz_px.y) + label_height * 0.5f
                    };
                    m_tickLabelPositionsY.push_back(y_label_pos);

                    curr_y_val += y_step;
                }
            }

            m_ticksVertexCount = static_cast<uint32_t>(V.size());
            
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * m_ticksVertexCount);

            m_vaoTicks = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();

        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::redrawTickLabels(normalized_params_t* _fig_params)
        {
            // m_(x/y)TickLabelPositions holds the pixel coordinates of tick positions

            // x axis
            //
            float data_spacing_px = _fig_params->data_spacing * _fig_params->figure_sz_px.x;
            float histogram_adj = (_fig_params->figure_type == FigureType::Histogram ? data_spacing_px * 0.5f : 0.0f);

            if (_fig_params->figure_type == FigureType::Histogram)
            {
                // Not strictly needed now, but perhaps if the API in the future permits manually
                // setting the tick positions and labels.
                SYN_CORE_ASSERT((m_tickLabelPositionsX.size()) == m_tickLabelsX.size(), 
                                "m_tickLabelPositionsX and m_tickLabelsX of different sizes.");

                // check and cache width of labels, offset positions, so that if labels are to wide
                // they can be omitted.
                bool sparse_labelling = false;
                bool reported = false;
                for (size_t i = 0; i < m_tickLabelsX.size(); i++)
                {
                    std::string label = m_tickLabelsX[i];
                    float label_width = m_tickLabelFont->getStringWidth("%s", label.c_str());
                    m_tickLabelPositionsX[i].x = m_tickLabelPositionsX[i].x - (label_width * 0.5f) + histogram_adj; 
                    if (label_width + 6 > data_spacing_px)
                    {
                        if (!reported) { SYN_CORE_WARNING("FIX ME: x tick labels wider than bars."); reported = true; }
                        sparse_labelling = true;
                    }
                }
                // label omission
                for (size_t i = 0; i < m_tickLabelsX.size(); i++)
                {
                    if (sparse_labelling && (i % 3 != 0))
                        m_tickLabelsX[i] = "";
                }
            }
            else if (_fig_params->figure_type == FigureType::LinePlot || 
                     _fig_params->figure_type == FigureType::ScatterPlot)
            {
                nice_scale x_tick_params(m_dataLimX, _fig_params->x_tick_count);
                m_tickLabelsX = std::vector<std::string>(m_tickLabelPositionsX.size());

                float start_val = x_tick_params.lower_bound;
                for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
                {
                    float tick_label = start_val + i * x_tick_params.tick_spacing;
                    std::stringstream ss;
                    ss << std::setprecision(0) << std::fixed << tick_label;
                    std::string label = ss.str();
                    m_tickLabelPositionsX[i].x -= (m_tickLabelFont->getStringWidth("%s", label.c_str()) * 0.5f);
                    m_tickLabelsX[i] = ss.str();
                }
            }

            // y axis
            //
            nice_scale y_tick_params(m_dataLimY, _fig_params->y_tick_count);
            
            // initialize empty vector        
            m_tickLabelsY = std::vector<std::string>(m_tickLabelPositionsY.size());
            
            float start_val = y_tick_params.lower_bound;
            for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
            {
                float tick_label = start_val + i * y_tick_params.tick_spacing;
                std::stringstream ss;
                std::setiosflags(std::ios::right);
                ss << std::setw(6) << std::setprecision(0) << std::fixed << tick_label;
                m_tickLabelsY[i] = ss.str();
            }
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::redrawSelection(normalized_params_t* _fig_params)
        {
            std::vector<glm::vec2> V;

            // fillBetweenX()
            if (_fig_params->fill_between_x)
            {
                V.push_back({ m_fillPositionsSelX[0].x, m_fillPositionsSelX[0].y });  // 0
                V.push_back({ m_fillPositionsSelX[1].x, m_fillPositionsSelX[0].y });  // 1
                V.push_back({ m_fillPositionsSelX[1].x, m_fillPositionsSelX[1].y });  // 2
                V.push_back({ m_fillPositionsSelX[1].x, m_fillPositionsSelX[1].y });  // 2
                V.push_back({ m_fillPositionsSelX[0].x, m_fillPositionsSelX[1].y });  // 3
                V.push_back({ m_fillPositionsSelX[0].x, m_fillPositionsSelX[0].y });  // 0
            }
            // fillBetweenY()
            if (_fig_params->fill_between_y)
            {
                V.push_back({ m_fillPositionsSelY[0].x, m_fillPositionsSelY[0].y });  // 0
                V.push_back({ m_fillPositionsSelY[1].x, m_fillPositionsSelY[0].y });  // 1
                V.push_back({ m_fillPositionsSelY[1].x, m_fillPositionsSelY[1].y });  // 2
                V.push_back({ m_fillPositionsSelY[1].x, m_fillPositionsSelY[1].y });  // 2
                V.push_back({ m_fillPositionsSelY[0].x, m_fillPositionsSelY[1].y });  // 3
                V.push_back({ m_fillPositionsSelY[0].x, m_fillPositionsSelY[0].y });  // 0
            }

            m_selectionVertexCount = static_cast<uint32_t>(V.size());

            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position "},
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * m_selectionVertexCount);

            m_vaoSelection = API::newVertexArray(vbo);
            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObjBase::setupStaticShaders()
        {
            if (m_shaderInitialized)
                return;

            // set static shader (for now)
            std::string shader_2D_src = R"(
                #type VERTEX_SHADER
                #version 430 core
                layout(location = 0) in vec2 a_position;
                void main()
                {
                    vec2 p = 2.0 * a_position - 1.0;
                    gl_Position = vec4(p, 0.0, 1.0);
                }
                #type FRAGMENT_SHADER
                #version 430 core
                uniform vec4 u_color;
                layout(location = 0) out vec4 f_color;
                void main() 
                { 
                    f_color = u_color;
                }
            )";
            FileIOHandler::write_buffer_to_file("./shader_2D.glsl", shader_2D_src);
            m_shader2D = API::newShader("./shader_2D.glsl");
            ShaderLibrary::add(m_shader2D);

            //std::string shader_lineplot_2D_src = R"(
            //    #type VERTEX_SHADER
            //    #version 430 core
            //    layout(location = 0) in vec3 a_position;
            //    out float v_alpha;
            //    void main()
            //    {
            //        v_alpha = a_position.z;
            //        vec2 p = 2.0 * a_position.xy - 1.0;
            //        gl_Position = vec4(p, 0.0, 1.0);
            //    }
            //    #type FRAGMENT_SHADER
            //    #version 430 core
            //    uniform vec4 u_color;
            //    in float v_alpha;
            //    layout(location = 0) out vec4 f_color;
            //    void main() 
            //    { 
            //        f_color = vec4(u_color.xyz, v_alpha);
            //    }
            //)";
            //FileIOHandler::write_buffer_to_file("./shader_lineplot_2D.glsl", shader_lineplot_2D_src);
            //m_shaderLinePlot2D = API::newShader("./shader_lineplot_2D.glsl");
            //ShaderLibrary::add(m_shaderLinePlot2D);

            std::string figure_font_src = R"(
                #type VERTEX_SHADER
                #version 330 core
                layout(location = 0) in vec4 a_position;
                out vec2 f_tex_pos;
                void main()
                {
                    gl_Position = vec4(a_position.xy, 0.0f, 1.0f);
                    f_tex_pos = a_position.zw;
                }
                #type FRAGMENT_SHADER
                #version 330 core
                in vec2 f_tex_pos;
                out vec4 out_color;
                uniform sampler2D u_texture_sampler;
                uniform vec4 u_color;
                void main()
                {
                    float a = texture2D(u_texture_sampler, f_tex_pos).r;
                    //float alpha = a;
                    //if (a > 0.5)
                    //    alpha = 1.0;
                    //out_color = vec4(u_color.rgb, alpha);
                    out_color = vec4(u_color.rgb, a * 1.5);
                }
            )";
            FileIOHandler::write_buffer_to_file("./hist_font_shader.glsl", figure_font_src);
            m_shaderFont = API::newShader("./hist_font_shader.glsl");
            ShaderLibrary::add(m_shaderFont);

            m_shaderInitialized = true;
        }

    }

}
