
#include <Synapse/Core.hpp>

#include "FigureRenderObj.h"
#include "FigureUtils.h"
#include "FigureBase.h"
#include "Canvas/Canvas2D.h"


namespace Syn
{
    namespace mplc
    {
        FigureRenderObj::FigureRenderObj(Figure* _parent)
        {
            SYN_CORE_TRACE("new FigureRenderObj object created.");
            
            m_parentRawPtr = _parent;
            m_figureTitle = _parent->m_figureTitle;
            m_figureSizePx = _parent->m_figureSizePx;

            setup_static_shaders();
            initialize();
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::initialize()
        {
            m_framebufferID = "canvas" + std::string(Random::rand_str(16));
            m_framebuffer = API::newFramebuffer(ColorFormat::RGBA16F,
                                                glm::vec2(m_figureSizePx.x, m_figureSizePx.y), // m_figureSizePx,
                                                1, 
                                                true, 
                                                false, 
                                                m_framebufferID);

            // default rendering parameters
            m_figureParamsPtr = m_parentRawPtr->m_figureParamsPtr.get();

            // title font
            m_titleFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                        m_figureParamsPtr->title_font_size_px,
                                        m_shaderFont,
                                        m_figureParamsPtr->figure_sz_px);
            m_titleFont->disableUpdateOnResize();
            m_titleFont->setColor(m_figureParamsPtr->title_color);

            // axis label font
            m_axisLabelFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                           m_figureParamsPtr->axis_label_font_size_px,
                                           m_shaderFont,
                                           m_figureParamsPtr->figure_sz_px);
            m_axisLabelFont->disableUpdateOnResize();
            m_axisLabelFont->setColor(m_figureParamsPtr->axis_label_color);

            // tick label font
            m_tickLabelFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                            m_figureParamsPtr->ticks_font_size_px,
                                            m_shaderFont,
                                            m_figureParamsPtr->figure_sz_px);        
            m_tickLabelFont->disableUpdateOnResize();
            m_tickLabelFont->setColor(m_figureParamsPtr->tick_label_color);


            //
            m_redrawFlags = FIGURE_REDRAW_ALL;
            m_redrawFlags |= (~ FIGURE_REDRAW_SELECTION);
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::render()
        {
            static auto& renderer = Renderer::get();

            if (!m_redrawFlags)
                return;
            
            //SYN_CORE_ASSERT(m_figureParamsPtr->figure_type != FigureType::None, 
            //                "Figure type must be set by Figure class instance.");
            
            //
            redraw();

            m_framebuffer->bind();
            
            renderer.setClearColor(m_figureParamsPtr->figure_background);
            renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            //m_framebuffer->bindDefaultFramebuffer();
            //return;
            
            m_shader2D->enable();
            
            // render canvas data
            for (auto* canvas : m_parentRawPtr->m_canvases)
            {
                m_shader2D->setUniform4fv("u_color", m_figureParamsPtr->stroke_color);
                canvas->vao()->bind();
                renderer.drawArrays(canvas->vertexCount(), 0, true, canvas->OpenGLPrimitive());
            }

            // render grid lines
            #ifdef DEBUG_FIGURE_GRIDLINES
                m_shader2D->setUniform4fv("u_color", glm::vec4(0.3f));
                __debug_vaoGridLines->bind();
                renderer.drawArrays(__debug_gridLinesVertexCount, 0, true, GL_LINES);
            #endif

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
            m_tickLabelFont->beginRenderBlock();
            if (m_tickLabelPositionsY.size() > 0 && m_tickLabelsY.size() > 0)
            {
                for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
                    m_tickLabelFont->addString(m_tickLabelPositionsY[i].x, m_tickLabelPositionsY[i].y, "%s", m_tickLabelsY[i].c_str());
            }
            if (m_tickLabelPositionsX.size() > 0 && m_tickLabelsX.size() > 0)
            {
                for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
                    m_tickLabelFont->addString(m_tickLabelPositionsX[i].x, m_tickLabelPositionsX[i].y, "%s", m_tickLabelsX[i].c_str());
            }
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
        void FigureRenderObj::redraw()
        {
            // conversion of pixel parameters to normalized coordinates [0..1]
            normalized_params_t norm_params = normalized_params_t(m_figureParamsPtr);
            
            //
            if (m_redrawFlags & FIGURE_REDRAW_DATA) // or FIGURE_REDRAW
            {
                for (auto* canvas : m_parentRawPtr->m_canvases)
                {
                    //m_parentRawPtr->update_data_limits(canvas);   -- instead, data limits are updated for every new plot added
                    canvas->redraw();
                }
            }
            
            // Check new data limits and compare it to the 'local' data limits of 
            // FigureRenderObj. If these limits are the same, ticks and tick labels can
            // stay as they were.
            if (m_localDataLimX != m_parentRawPtr->m_dataLimX || 
                m_localDataLimY != m_parentRawPtr->m_dataLimY)
                m_redrawFlags |= FIGURE_REDRAW_AUX;

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
                m_tickLabelsX.clear();                
                m_tickLabelsY.clear();
                redrawTickLabels(&norm_params);
            }

            //
            m_redrawFlags = 0;
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::redrawAxes(normalized_params_t* _fig_params)
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
        void FigureRenderObj::redrawTicks(normalized_params_t* _fig_params)
        {
            std::vector<glm::vec2> V;
            auto scaler = m_parentRawPtr->axesScalerPtr();

            float font_height = m_tickLabelFont->getFontHeight();
            #ifdef DEBUG_FIGURE_GRIDLINES
                std::vector<glm::vec2> __debug_vertices;
            #endif

            if (_fig_params->render_x_ticks)
            {
                // x ticks for Histograms
                if (_fig_params->figure_type == FigureType::Histogram)
                {
                    /*
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
                    */
                }
                // x ticks for 'linear' data
                else
                {
                    //float x_plot_min = _fig_params->canvas_origin.x + _fig_params->data_axis_offset.x;
                    //float x_plot_max = _fig_params->canvas_origin.x + _fig_params->x_axis_length - _fig_params->data_axis_offset.x;
                    //nice_scale x_tick_params(m_parentRawPtr->m_dataLimX, _fig_params->x_tick_count);
                    //range_converter x_converter({ x_tick_params.lower_bound, x_tick_params.upper_bound },
                    //                            { x_plot_min, x_plot_max });
                    //float curr_x_val = x_tick_params.lower_bound;
                    //NiceScale x_tick_params(m_parentRawPtr->dataLimX());
                    //x_tick_params.__debug_print("x tick params");
                    //range_converter x_converter({ x_tick_params.nice_lim[0], x_tick_params.nice_lim[1] },
                    //                            { x_plot_min, x_plot_max });
                    //float curr_x_val = x_tick_params.lower_bound;
                    //float x_step = x_tick_params.tick_spacing;
                    float curr_x_val = scaler->x_ticks().lower_bound;
                    float x_step = scaler->x_ticks().tick_spacing;
                    //
                    //for (size_t i = 0; i < x_tick_params.max_ticks; i++)
                    for (size_t i = 0; i < scaler->x_ticks().max_ticks + 1; i++)
                    {
                        //float x = x_converter.eval(curr_x_val);
                        float x = scaler->eval_x(curr_x_val);

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

                        #ifdef DEBUG_FIGURE_GRIDLINES
                            __debug_vertices.push_back({ x, _fig_params->canvas_origin.y });
                            __debug_vertices.push_back({ x, _fig_params->y_axis_lim[1] });
                        #endif
                        
                        curr_x_val += x_step;
                    }
                }
            }

            // y ticks
            if (_fig_params->render_y_ticks)
            {
                //float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
                //float y_plot_max = _fig_params->data_height;
                //NiceScale y_tick_params(m_parentRawPtr->m_dataLimY);
                //range_converter converter({ y_tick_params.lower_bound, y_tick_params.upper_bound },
                //                          { y_plot_min, y_plot_max });

                static float char_width_px = m_tickLabelFont->getStringWidth("1");
                float label_width = (num_digits_float(scaler->y_ticks().upper_bound) + 1) * char_width_px;
                float label_height = m_tickLabelFont->getFontHeight() * 0.5f;
                //float curr_y_val = y_tick_params.lower_bound;
                //float y_step = y_tick_params.tick_spacing;
                float curr_y_val = scaler->y_ticks().lower_bound;
                float y_step = scaler->y_ticks().tick_spacing;
                //
                //for (size_t i = 0; i < y_tick_params.max_ticks + 1; i++)
                for (size_t i = 0; i < scaler->y_ticks().max_ticks + 1; i++)
                {
                    //float y = converter.eval(curr_y_val);
                    float y = scaler->eval_y(curr_y_val);

                    glm::vec2 v0 = { _fig_params->canvas_origin.x, y };
                    glm::vec2 v1 = { _fig_params->canvas_origin.x - _fig_params->tick_length.x, y }; 
                    
                    V.push_back(v0);
                    V.push_back(v1);

                    glm::vec2 y_label_pos =
                    {
                        (v1.x * _fig_params->figure_sz_px.x) - m_figureParamsPtr->tick_labels_offset_px.x - label_width, 
                        // font_height is further scaled down w/ 0.75 since the normalized range (0..1) is scaled up for font rendering (-1..1)
                        _fig_params->figure_sz_px.y - (v1.y * _fig_params->figure_sz_px.y) + label_height * 0.75f
                    };
                    m_tickLabelPositionsY.push_back(y_label_pos);

                    #ifdef DEBUG_FIGURE_GRIDLINES
                        __debug_vertices.push_back({ _fig_params->x_axis_lim[0], y });
                        __debug_vertices.push_back({ _fig_params->x_axis_lim[1], y });
                    #endif

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

            #ifdef DEBUG_FIGURE_GRIDLINES
                __debug_gridLinesVertexCount = static_cast<uint32_t>(__debug_vertices.size());
                Ref<VertexBuffer> vbo2 = API::newVertexBuffer(GL_STATIC_DRAW);
                vbo2->setBufferLayout({
                    { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
                });
                vbo2->setData((void*)(&__debug_vertices[0]), sizeof(glm::vec2) * __debug_gridLinesVertexCount);
                __debug_vaoGridLines = API::newVertexArray(vbo2);
            #endif

            Renderer::get().executeRenderCommands();

        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::redrawTickLabels(normalized_params_t* _fig_params)
        {
            // m_(x/y)TickLabelPositions holds the pixel coordinates of tick positions

            auto scaler = m_parentRawPtr->axesScalerPtr();

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
            //else if (_fig_params->figure_type == FigureType::LinePlot || 
            //         _fig_params->figure_type == FigureType::ScatterPlot)
            else
            {
                //NiceScale x_tick_params(m_parentRawPtr->m_dataLimX);
                m_tickLabelsX = std::vector<std::string>(m_tickLabelPositionsX.size());

                //float start_val = x_tick_params.lower_bound;
                float start_val = scaler->x_ticks().lower_bound;
                for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
                {
                    //float tick_label = start_val + i * x_tick_params.tick_spacing;
                    float tick_label = start_val + i * scaler->x_ticks().tick_spacing;
                    std::stringstream ss;
                    ss << std::setprecision(2) /*<< std::fixed*/ << tick_label;
                    std::string label = ss.str();
                    m_tickLabelPositionsX[i].x -= (m_tickLabelFont->getStringWidth("%s", label.c_str()) * 0.5f);
                    m_tickLabelsX[i] = ss.str();
                }
            }

            // y axis
            //
            //NiceScale y_tick_params(m_parentRawPtr->m_dataLimY);
            
            // initialize empty vector        
            m_tickLabelsY = std::vector<std::string>(m_tickLabelPositionsY.size());
            
            //float start_val = y_tick_params.lower_bound;
            float start_val = scaler->y_ticks().lower_bound;
            for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
            {
                //float tick_label = start_val + i * y_tick_params.tick_spacing;
                float tick_label = start_val + i * scaler->y_ticks().tick_spacing;
                std::stringstream ss;
                std::setiosflags(std::ios::right);
                //ss << std::setw(6) << std::setprecision(0) << std::fixed << tick_label;
                ss << std::setw(3) << tick_label;
                m_tickLabelsY[i] = ss.str();
            }
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::setup_static_shaders()
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
            FileIOHandler::write_buffer_to_file("./figure_font_shader.glsl", figure_font_src);
            m_shaderFont = API::newShader("./figure_font_shader.glsl");
            ShaderLibrary::add(m_shaderFont);

            m_shaderInitialized = true;
        }
        //-------------------------------------------------------------------------------
        int FigureRenderObj::num_digits_float(float _f)
        {
            int digits = 0;
            double ori = _f;    //copy of original number
            long num2  = _f;
            
            //count no of digits before floating point
            while (num2 > 0)
            {
                digits++;
                num2 = num2 / 10;
            }
            if(ori == 0)
                digits = 1;
            
            float no_float;
            no_float = ori * (pow(10, (8 - digits)));
            long long int total=(long long int)no_float;
            int no_of_digits, extrazeroes=0;
            
            for(int i = 0; i < 8; i++)
            {
                int dig;
                dig = total%10;
                total = total/10;
                if(dig != 0)
                    break;
                else
                    extrazeroes++;
            }
            no_of_digits = 8 - extrazeroes;
            return no_of_digits;
        }
    }
}
