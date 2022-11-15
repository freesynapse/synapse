
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
            m_framebufferID = "Figure_" + std::string(Random::rand_str(8));
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
            static const Ref<Axes>& axes = m_parentRawPtr->axesPtr();

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
            
            // render grid lines
            #ifdef DEBUG_FIGURE_GRIDLINES
                m_shader2D->setUniform4fv("u_color", glm::vec4(0.3f));
                __debug_vaoGridLines->bind();
                renderer.drawArrays(__debug_gridLinesVertexCount, 0, true, GL_LINES);
            #endif

            // render canvas data
            for (auto& canvas : m_parentRawPtr->m_canvases)
            {
                //Canvas2D* canvas_ptr = canvas.second;
                //m_shader2D->setUniform4fv("u_color", m_figureParamsPtr->stroke_color);
                //m_shader2D->setUniform4fv("u_color", canvas_ptr->m_canvasParameters.stroke_color);
                //canvas_ptr->vao()->bind();
                //renderer.drawArrays(canvas_ptr->vertexCount(), 0, true, canvas_ptr->OpenGLPrimitive());
                canvas.second->render(m_shader2D);
            }

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
            if (m_tickLabelPositionsY.size())// > 0 && m_tickLabelsY.size() > 0)
            {
                auto& y_labels  = axes->y_ticks().tick_labels.labels;
                for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
                    m_tickLabelFont->addString(m_tickLabelPositionsY[i].x, 
                                               m_tickLabelPositionsY[i].y, 
                                               "%s", 
                                               y_labels[i].c_str());
                                               //m_tickLabelsY[i].c_str());
            }
            if (m_tickLabelPositionsX.size())// > 0 && m_tickLabelsX.size() > 0)
            {
                auto& x_labels  = axes->x_ticks().tick_labels.labels;
                for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
                    m_tickLabelFont->addString(m_tickLabelPositionsX[i].x, 
                                               m_tickLabelPositionsX[i].y, 
                                               "%s", 
                                               x_labels[i].c_str());
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
                for (auto& canvas : m_parentRawPtr->m_canvases)
                {
                    //m_parentRawPtr->update_data_limits(canvas);   -- instead, data limits are updated for every new plot added
                    canvas.second->redraw();
                }
            }
            
            // Check new data limits and compare it to the 'local' data limits of 
            // FigureRenderObj. If these limits are the same, ticks and tick labels can
            // stay as they were.
            if (m_localDataLimX != m_parentRawPtr->m_dataLimX || 
                m_localDataLimY != m_parentRawPtr->m_dataLimY)
            {
                m_localDataLimX = m_parentRawPtr->m_dataLimX;
                m_localDataLimY = m_parentRawPtr->m_dataLimY;
                m_redrawFlags |= FIGURE_REDRAW_AUX;
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
                //m_tickLabelsX.clear();                
                //m_tickLabelsY.clear();
                redrawTickLabels(&norm_params);
            }

            //
            m_redrawFlags = 0;
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::redrawAxes(normalized_params_t* _fig_params)
        {
            m_axesVertexCount = 4;
            glm::vec3 V[4] = 
            {
                // X 
                { _fig_params->x_axis_lim[0] - _fig_params->axes_neg_protrusion.x, _fig_params->canvas_origin.y, _fig_params->z_value_aux },
                { _fig_params->x_axis_lim[1], _fig_params->canvas_origin.y, _fig_params->z_value_aux },
                // Y
                { _fig_params->canvas_origin.x, _fig_params->y_axis_lim[0] - _fig_params->axes_neg_protrusion.y, _fig_params->z_value_aux },
                { _fig_params->canvas_origin.x, _fig_params->y_axis_lim[1], _fig_params->z_value_aux },
            };
            //
            
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            });
            vbo->setData((void*)V, sizeof(glm::vec3) * m_axesVertexCount);

            m_vaoAxes = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::redrawTicks(normalized_params_t* _fig_params)
        {
            std::vector<glm::vec3> V;
            static const Ref<Axes>& axes = m_parentRawPtr->axesPtr();
            static float epsilon = 0.01f;

            float font_height = m_tickLabelFont->getFontHeight();
            #ifdef DEBUG_FIGURE_GRIDLINES
                std::vector<glm::vec3> __debug_vertices;
            #endif

            if (_fig_params->render_x_ticks)
            {
                float curr_x_val = axes->x_ticks().lower_bound;
                float x_step = axes->x_ticks().tick_spacing;
                //
                for (size_t i = 0; i < axes->x_ticks().max_ticks + 1; i++)
                {
                    float x = axes->eval_x(curr_x_val);

                    if (x > _fig_params->x_axis_lim[1] + epsilon)
                        break;

                    glm::vec3 v0 = { x, _fig_params->canvas_origin.y, _fig_params->z_value_aux };
                    glm::vec3 v1 = { x, _fig_params->canvas_origin.y - _fig_params->tick_length.y, _fig_params->z_value_aux };
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
                        if (i > 0 && i < axes->x_ticks().max_ticks)
                        {
                            __debug_vertices.push_back({ x, _fig_params->canvas_origin.y, _fig_params->z_value_aux });
                            __debug_vertices.push_back({ x, _fig_params->y_axis_lim[1], _fig_params->z_value_aux });
                        }
                    #endif
                    
                    curr_x_val += x_step;
                }
            }

            // y ticks
            if (_fig_params->render_y_ticks)
            {
                float label_height = m_tickLabelFont->getFontHeight() * 0.5f;
                
                /* TODO : could be automated so that axes->x_tick_start() returns the first normalized x tick
                 * and axes->next_x_tick() gives the next, normalized tick. Counters inside of scaler etc.
                 */
                float curr_y_val = axes->y_ticks().lower_bound;
                float y_step = axes->y_ticks().tick_spacing;
                
                //
                for (size_t i = 0; i < axes->y_ticks().max_ticks + 1; i++)
                {
                    float y = axes->eval_y(curr_y_val);

                    if (y > _fig_params->y_axis_lim[1]+epsilon)
                        break;

                    glm::vec3 v0 = { _fig_params->canvas_origin.x, y, _fig_params->z_value_aux };
                    glm::vec3 v1 = { _fig_params->canvas_origin.x - _fig_params->tick_length.x, y, _fig_params->z_value_aux }; 
                    
                    V.push_back(v0);
                    V.push_back(v1);

                    glm::vec2 y_label_pos =
                    {
                        (v1.x * _fig_params->figure_sz_px.x) - m_figureParamsPtr->tick_labels_offset_px.x, // - label_width, 
                        // font_height is further scaled down w/ 0.75 since the normalized range (0..1) is scaled up for font rendering (-1..1)
                        _fig_params->figure_sz_px.y - (v1.y * _fig_params->figure_sz_px.y) + label_height * 0.75f
                    };
                    m_tickLabelPositionsY.push_back(y_label_pos);

                    #ifdef DEBUG_FIGURE_GRIDLINES
                        if (i > 0 && i < axes->y_ticks().max_ticks - 1)
                        {
                            __debug_vertices.push_back({ _fig_params->x_axis_lim[0], y, _fig_params->z_value_aux });
                            __debug_vertices.push_back({ _fig_params->x_axis_lim[1], y, _fig_params->z_value_aux });
                        }
                    #endif

                    curr_y_val += y_step;
                }
            }

            m_ticksVertexCount = static_cast<uint32_t>(V.size());
            
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec3) * m_ticksVertexCount);

            m_vaoTicks = API::newVertexArray(vbo);

            #ifdef DEBUG_FIGURE_GRIDLINES
                __debug_gridLinesVertexCount = static_cast<uint32_t>(__debug_vertices.size());
                Ref<VertexBuffer> vbo2 = API::newVertexBuffer(GL_STATIC_DRAW);
                vbo2->setBufferLayout({
                    { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
                });
                vbo2->setData((void*)(&__debug_vertices[0]), sizeof(glm::vec3) * __debug_gridLinesVertexCount);
                __debug_vaoGridLines = API::newVertexArray(vbo2);
            #endif

            Renderer::get().executeRenderCommands();

        }
        //-------------------------------------------------------------------------------
        void FigureRenderObj::redrawTickLabels(normalized_params_t* _fig_params)
        {
            // m_(x/y)TickLabelPositions holds the pixel coordinates of tick positions

            static const Ref<Axes>& axes = m_parentRawPtr->axesPtr();

            // x axis
            //
            float data_spacing_px = _fig_params->data_spacing * _fig_params->figure_sz_px.x;
            float histogram_adj = (_fig_params->figure_type == FigureType::Histogram ? data_spacing_px * 0.5f : 0.0f);

            format_tick_labels(&axes->x_ticks(), m_tickLabelPositionsX);
            auto& x_labels = axes->x_ticks().tick_labels.labels;
            //
            for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
                m_tickLabelPositionsX[i].x -= m_tickLabelFont->getStringWidth("%s", x_labels[i].c_str()) * 0.5f;

            // y axis
            //            
            format_tick_labels(&axes->y_ticks(), m_tickLabelPositionsY);
            auto& y_labels = axes->y_ticks().tick_labels.labels;
            //
            for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
                m_tickLabelPositionsY[i].x -= m_tickLabelFont->getStringWidth("%s", y_labels[i].c_str());

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
                layout(location = 0) in vec3 a_position;
                void main()
                {
                    vec2 p = 2.0 * a_position.xy - 1.0;
                    gl_Position = vec4(p.x, p.y, a_position.z, 1.0);
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
        //-------------------------------------------------------------------------------
        void FigureRenderObj::format_tick_labels(NiceScale* _ticks,
                                                 const std::vector<glm::vec2>& _label_positions)
        {
            tick_labels_t labels;
                        
            float range = _ticks->range;
            int log_10 = (int)(ceil(log10(abs(range))));
            int k = log_10 + (log_10 < 0 ? 4 : 0);

            float lo_bound = _ticks->lower_bound;
            for (size_t i = 0; i < _label_positions.size(); i++)
            {
                float tick_label = lo_bound + i * _ticks->tick_spacing;
                std::stringstream ss;
                ss << std::setprecision(k) << tick_label;
                std::string fmt_label = ss.str();
                labels.labels.push_back(fmt_label);
                float label_width = m_tickLabelFont->getStringWidth(fmt_label.c_str());
                labels.max_label_width = std::min(labels.min_label_width, 
                                                  label_width);
                labels.max_label_width = std::max(labels.max_label_width, 
                                                  label_width);
            }

            labels.label_count = _label_positions.size();
            _ticks->tick_labels = labels;
        }

    }
}
