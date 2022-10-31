
#include "HistogramTemplates.hpp"
#include "Histogram.hpp"
#include "HistogramRenderObj.hpp"

template<typename T>
void HistogramRenderObj<T>::render()
{
    if (!m_redrawFlags)
        return;
    
    redraw();

    static auto& renderer = Renderer::get();
    m_framebuffer->bind();
    
    renderer.setClearColor(m_histParamsPtr->canvas_background);
    renderer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    m_shaderCanvas->enable();
    
    // render selection
    m_shaderCanvas->setUniform4fv("u_color", glm::vec4(1.0f, 1.0f, 1.0f, 0.25f));
    m_vaoSelection->bind();
    renderer.drawArrays(6, 0, true, GL_TRIANGLES);

    // render data
    m_shaderCanvas->setUniform4fv("u_color", m_histParamsPtr->bar_color);
    m_vaoBars->bind();
    renderer.drawArrays(m_barsVertexCount, 0, true, GL_TRIANGLES);

    // render axes and ticks
    m_shaderCanvas->setUniform4fv("u_color", m_histParamsPtr->axis_color);
    m_vaoAxes->bind();
    renderer.drawArrays(m_axesVertexCount, 0, true, GL_LINES);

    // render tick labels
    m_tickLabelFont->setColor(m_histParamsPtr->tick_label_color);
    float x_step = 0.0f;
    float x = 0.0f;

    m_tickLabelFont->beginRenderBlock();
    for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
        m_tickLabelFont->addString(m_tickLabelPositionsY[i].x, m_tickLabelPositionsY[i].y, "%s", m_tickLabelsY[i].c_str());
    for (size_t i = 0; i < m_tickLabelPositionsX.size(); i++)
        m_tickLabelFont->addString(m_tickLabelPositionsX[i].x, m_tickLabelPositionsX[i].y, "%s", m_tickLabelsX[i].c_str());
    m_tickLabelFont->endRenderBlock();
    
    const char* title = m_parentRawPtr->m_figureTitle.c_str();
    float title_height = m_titleFont->getFontHeight();
    float title_width = m_titleFont->getStringWidth("%s", title);
    glm::vec2 title_pos =
    {
        (m_histParamsPtr->figure_sz_px.x * 0.5f) - (title_width * 0.5f),
        10.0f + title_height
    };
    m_titleFont->beginRenderBlock();
    m_titleFont->addString(title_pos.x, title_pos.y, "%s", title);
    m_titleFont->endRenderBlock();

    m_framebuffer->bindDefaultFramebuffer();
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::setInteractiveMode(bool _b)
{
    m_interactiveMode = _b;
    
    if (m_interactiveMode)
    {
        m_currInterval = { m_parentRawPtr->minBin(), m_parentRawPtr->maxBin() };
        m_prevInterval = m_currInterval;

        m_redrawFlags |= FIGURE_REDRAW_SELECTION;
    }
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::updateInterval()
{
    selectInterval(m_currInterval.first, m_currInterval.second);
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::selectInterval(const T& _lo, const T& _hi)
{
    T lo = m_parentRawPtr->findClosestBin(_lo)->first;
    T hi = m_parentRawPtr->findClosestBin(_hi)->first;

    if (!m_interactiveMode)
        return;

    if (lo < m_parentRawPtr->minBin())
    {
        SYN_CORE_WARNING("invalid histogram interval selected : ", lo, ", ", hi, ".");
        m_currInterval.first = m_parentRawPtr->minBin();
        return;
    }
    else if (hi > m_parentRawPtr->maxBin())
    {
        SYN_CORE_WARNING("invalid histogram interval selected : ", lo, ", ", hi, ".");
        m_currInterval.second = m_parentRawPtr->maxBin();
        return;
    }

    // auto adjust entered values within bounds
    if (lo > hi)
    {
        if (hi < m_parentRawPtr->maxBin())
        {
            auto next_hi = std::next(m_parentRawPtr->m_bins.find(hi));
            hi = next_hi->first;
            lo = std::prev(next_hi)->first;
            //hi++;
            //lo = hi - 1;
        }
        else if (hi == m_parentRawPtr->maxBin())
        {
            lo = std::prev(m_parentRawPtr->m_bins.find(hi))->first;
            //lo--;
        }
    }

    m_currInterval = { lo, hi };

    if (m_currInterval != m_prevInterval)
    {
        m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        m_prevInterval = m_currInterval;
    }
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::redraw()
{
    
    // conversion of pixel parameters to normalized coordinates [0..1]
    normalized_params_t norm_params = normalized_params_t(m_histParamsPtr);
    //
    if (m_redrawFlags & FIGURE_REDRAW_DATA)
        redrawBars(&norm_params);
    
    if (m_redrawFlags & FIGURE_REDRAW_AXES || m_redrawFlags & FIGURE_REDRAW_LABELS)
    {
        m_xTickPositionsX.clear();
        m_tickLabelPositionsX.clear();
        m_tickLabelsX.clear();    
        m_tickLabelPositionsY.clear();
        m_tickLabelsY.clear();

        redrawAxes(&norm_params);
        redrawTickLabels(&norm_params);
    }

    if (m_redrawFlags & FIGURE_REDRAW_SELECTION)
        redrawSelection(&norm_params);

    //
    m_redrawFlags = 0;
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::redrawBars(normalized_params_t* _hist_params)
{
    // scaling -- find bin with highest counts
    m_maxBinCount = 0.0f;
    for (const auto& it : m_parentRawPtr->m_bins)
        m_maxBinCount = std::max(m_maxBinCount, static_cast<float>(it.second));
    float max_n_inv = (1.0f - _hist_params->canvas_origin.y - _hist_params->bar_axis_offset.y) / m_maxBinCount;

    // vertex and index data
    m_barsVertexCount = 6 * (m_parentRawPtr->m_binCount + 1);
    glm::vec2* V = new glm::vec2[m_barsVertexCount];
    
    uint32_t i = 0;
    uint32_t v = 0;
    float x_step = (_hist_params->x_axis_length - 2 * (_hist_params->bar_axis_offset.x)) / static_cast<float>(m_parentRawPtr->m_binCount);
    float x = _hist_params->canvas_origin.x + _hist_params->bar_axis_offset.x;
    //
    for (const auto& it : m_parentRawPtr->m_bins)
    {
        float x0 = x + (_hist_params->bar_spacing * 0.5f);
        float x1 = x + x_step - (_hist_params->bar_spacing * 0.5f);
        float y0 = _hist_params->canvas_origin.y + _hist_params->bar_axis_offset.y;
        float y1 = _hist_params->canvas_origin.y + \
                    static_cast<float>(it.second) * max_n_inv * _hist_params->bar_height + \
                    _hist_params->bar_axis_offset.y;

        V[i+0] = glm::vec2(x0, y0); V[i+1] = glm::vec2(x1, y0); V[i+2] = glm::vec2(x1, y1);
        V[i+3] = glm::vec2(x1, y1); V[i+4] = glm::vec2(x0, y1); V[i+5] = glm::vec2(x0, y0);

        i += 6;
        x += x_step;
    }
    m_barsVertexCount = i;

    //
    Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
    vbo->setBufferLayout({
        { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
    });
    vbo->setData((void*)V, sizeof(glm::vec2) * m_barsVertexCount);

    m_vaoBars = API::newVertexArray(vbo);
    Renderer::get().executeRenderCommands();

    delete[] V;
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::redrawAxes(normalized_params_t* _hist_params)
{
    m_axesVertexCount = 0;
    
    if (_hist_params->render_x_axis) m_axesVertexCount += 2;
    if (_hist_params->render_y_axis) m_axesVertexCount += 2;

    uint32_t x_tick_count = 0;
    if (_hist_params->render_x_ticks)
    {
        x_tick_count = m_parentRawPtr->m_binCount;
        if (_hist_params->x_ticks_between_bars)
            x_tick_count++;
        m_axesVertexCount += (x_tick_count * 2);
    }

    if (_hist_params->render_y_ticks)
        // -1 since first tick will always be at 0
        m_axesVertexCount += ((_hist_params->y_tick_count + 1) * 2);

    //        
    std::vector<glm::vec2> V;

    // x axis
    if (_hist_params->render_x_axis)
    {
        V.push_back({ _hist_params->x_axis_lim[0] - _hist_params->axes_neg_protrusion.x, _hist_params->canvas_origin.y });
        V.push_back({ _hist_params->x_axis_lim[1], _hist_params->canvas_origin.y });
    }
    // y axis
    if (_hist_params->render_y_axis)
    {
        V.push_back({ _hist_params->canvas_origin.x, _hist_params->y_axis_lim[0] - _hist_params->axes_neg_protrusion.y });
        V.push_back({ _hist_params->canvas_origin.x, _hist_params->y_axis_lim[1] });
    }

    // x ticks
    float font_height = m_tickLabelFont->getFontHeight();
    if (_hist_params->render_x_ticks)
    {
        float bar_width = (_hist_params->x_axis_length - 2 * (_hist_params->bar_axis_offset.x)) / static_cast<float>(m_parentRawPtr->m_binCount);
        float x = _hist_params->canvas_origin.x + _hist_params->bar_axis_offset.x + (bar_width * 0.5f);
        if (_hist_params->x_ticks_between_bars)
            x -= (bar_width * 0.5f);

        for (size_t i = 0; i < m_parentRawPtr->m_bins.size(); i++)
        {
            glm::vec2 v0 = { x, _hist_params->canvas_origin.y };
            glm::vec2 v1 = { x, _hist_params->canvas_origin.y - _hist_params->tick_length.y };
            V.push_back(v0);
            V.push_back(v1);
            m_xTickPositionsX.push_back(x);                
            x += bar_width;

            // set tick label positions -- n.b.: y axis is inverted for font rendering in pixel space
            glm::vec2 x_tick_pos = 
            {
                v1.x * _hist_params->figure_sz_px.x,
                _hist_params->figure_sz_px.y - ((v1.y - _hist_params->tick_labels_offset.y) * _hist_params->figure_sz_px.y) + font_height
            };
            m_tickLabelPositionsX.push_back(x_tick_pos);
        }
        
        // store x tick interval for selection
        if (m_xTickPositionsX.size() > 1)
            m_xTickInterval = m_xTickPositionsX[1] - m_xTickPositionsX[0];
        
        // add last tick position if between bars ticks (i.e. one more tick than bars/labels)
        if (_hist_params->x_ticks_between_bars)    // if between bars the last tick is 'unlabeled'
        {
            V.push_back(glm::vec2(x, _hist_params->canvas_origin.y));
            V.push_back(glm::vec2(x, _hist_params->canvas_origin.y - _hist_params->tick_length.y));
        }
    }

    // y ticks
    if (_hist_params->render_y_ticks)
    {
        nice_scale y_tick_params(0, m_maxBinCount, _hist_params->y_tick_count);
        
        float y_min = _hist_params->canvas_origin.y + _hist_params->bar_axis_offset.y;
        float y_max = _hist_params->canvas_origin.y + \
                        m_maxBinCount * \
                        ((1.0f - _hist_params->canvas_origin.y - _hist_params->bar_axis_offset.y) / m_maxBinCount) * \
                        _hist_params->bar_height + \
                        _hist_params->bar_axis_offset.y;

        // the length of one (1) unit in normalized, canvas-corrected, coordinates
        float unit_length = (y_max - y_min) / m_maxBinCount;
        float y = _hist_params->canvas_origin.y;
        float y_step = y_tick_params.m_tick_spacing * unit_length;
        float label_width = m_tickLabelFont->getStringWidth("111111"); // TODO : replace this with something dynamic
        float label_height = m_tickLabelFont->getFontHeight() * 0.5f;
        //
        for (size_t i = 0; i < y_tick_params.m_max_ticks + 1; i++)
        {
            if (y > _hist_params->y_axis_lim[1])
                break;

            glm::vec2 v0 = { _hist_params->canvas_origin.x, y };
            glm::vec2 v1 = { _hist_params->canvas_origin.x - _hist_params->tick_length.x, y }; 
            V.push_back(v0);
            V.push_back(v1);

            glm::vec2 y_label_pos =
            {
                (v1.x - _hist_params->tick_labels_offset.x) * _hist_params->figure_sz_px.x - label_width, 
                // font_height is further scaled down w/ 0.5 since the normalized range (0..1) is scaled up for font rendering (-1..1)
                _hist_params->figure_sz_px.y - (v1.y * _hist_params->figure_sz_px.y) + label_height * 0.5f
            };
            m_tickLabelPositionsY.push_back(y_label_pos);

            // tick label 
            y += y_step;
        }
    }

    //m_axesVertexCount = v_idx;
    m_axesVertexCount = V.size();
    
    Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
    vbo->setBufferLayout({
        { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
    });
    //vbo->setData((void*)V, sizeof(glm::vec2) * m_axesVertexCount);
    vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * V.size());

    m_vaoAxes = API::newVertexArray(vbo);

    Renderer::get().executeRenderCommands();

    //delete[] V;
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::redrawTickLabels(normalized_params_t* _hist_params)
{
    // m_(x/y)TickLabelPositions holds the pixel coordinates of tick positions

    // x axis
    //
    float bar_width_px = ((1.0f - 2 * (_hist_params->canvas_origin.x + _hist_params->bar_axis_offset.x)) / \
                            static_cast<float>(m_parentRawPtr->m_binCount)) * _hist_params->figure_sz_px.x;
    size_t i = 0;
    for (const auto& it : m_parentRawPtr->m_bins)
    {
        std::stringstream ss;
                    
        T val = it.first;
        
        if (std::is_floating_point<T>::value)
            ss << std::setprecision(1) << std::fixed << val;
        else
            ss << val;
        std::string label = ss.str();
        float label_width = m_tickLabelFont->getStringWidth(label.c_str());

        // TODO : rotate labels?
        if (label_width > bar_width_px)
        {
            // quick fix, space tick labels so at least som eare readable
            static bool reported = false;
            if (!reported)
            {
                SYN_CORE_WARNING("FIX ME: x tick labels wider than bars.");
                reported = true;
            }
            if (i % 3 != 0)
                label = "";
        }

        m_tickLabelsX.push_back(label);

        m_tickLabelPositionsX[i].x = m_tickLabelPositionsX[i].x - (label_width * 0.5f);
        if (_hist_params->x_ticks_between_bars)
                m_tickLabelPositionsX[i].x += (bar_width_px * 0.5f);

        i++;
    }

    SYN_CORE_ASSERT(m_tickLabelPositionsX.size() == m_tickLabelsX.size(), "m_tickLabelPositionsX.size() != m_tickLabelsX.size()");

    // y axis
    //
    nice_scale y_tick_params(0, m_maxBinCount, _hist_params->y_tick_count);
    
    // initialize empty vector        
    m_tickLabelsY = std::vector<std::string>(m_tickLabelPositionsY.size());
    
    for (size_t i = 0; i < m_tickLabelPositionsY.size(); i++)
    {
        float tick_label = i * y_tick_params.m_tick_spacing;
        std::stringstream ss;
        std::setiosflags(std::ios::right);
        ss << std::setw(6) << std::setprecision(0) << std::fixed << tick_label;
        m_tickLabelsY[i] = ss.str();
    }
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::redrawSelection(normalized_params_t* _hist_params)
{
    // calculate corrdinates based on which bins are selected.
    int offset = (int)m_parentRawPtr->minBin();
    glm::vec2 min_xy = { m_xTickPositionsX[(int)m_currInterval.first - offset], _hist_params->y_axis_lim[0] };
    float x_max = m_xTickPositionsX[0] + (((int)m_currInterval.second - offset + 1) * m_xTickInterval);
    glm::vec2 max_xy = { x_max, _hist_params->y_axis_lim[1] };

    glm::vec2 vertices[6] =
    {
        glm::vec2(min_xy.x, min_xy.y),  // 0
        glm::vec2(max_xy.x, min_xy.y),  // 1
        glm::vec2(max_xy.x, max_xy.y),  // 2

        glm::vec2(max_xy.x, max_xy.y),  // 2
        glm::vec2(min_xy.x, max_xy.y),  // 3
        glm::vec2(min_xy.x, min_xy.y),  // 0
    };

    Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
    vbo->setBufferLayout({
        { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position "},
    });
    vbo->setData((void*)vertices, sizeof(glm::vec2) * 6);

    m_vaoSelection = API::newVertexArray(vbo);
    Renderer::get().executeRenderCommands();
}
//---------------------------------------------------------------------------------------
template<typename T>
void HistogramRenderObj<T>::setupStaticShaders()
{
    // set static shader (for now)
    std::string hist_shader_src = R"(
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
    FileIOHandler::write_buffer_to_file("./hist_canvas_shader.glsl", hist_shader_src);
    m_shaderCanvas = API::newShader("./hist_canvas_shader.glsl");
    ShaderLibrary::add(m_shaderCanvas);

    std::string hist_font_src = R"(
        #type VERTEX_SHADER
        #version 330 core

        layout(location = 0) in vec4 a_position;
        
        // u_rotate is set to 1 if rotation is performed
        uniform int u_do_rotate = 0;
        uniform mat2 u_rotation_matrix;

        out vec2 f_tex_pos;

        void main()
        {
            vec2 xy = a_position.xy;
            if (u_do_rotate == 1)
                xy = u_rotation_matrix * xy;
            gl_Position = vec4(xy, 0.0f, 1.0f);
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
    FileIOHandler::write_buffer_to_file("./hist_font_shader.glsl", hist_font_src);
    m_shaderFont = API::newShader("./hist_font_shader.glsl");
    ShaderLibrary::add(m_shaderFont);

}



