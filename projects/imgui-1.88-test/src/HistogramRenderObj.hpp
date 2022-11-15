
#pragma once

#include <stdio.h>

#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Renderer/Buffers/Framebuffer.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Renderer/Buffers/VertexBuffer.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/Shader/ShaderLibrary.hpp>

#include <Synapse/Utils/Random/Random.hpp>

#include "HistogramParameters.h"
#include "FigureUtils.h"

using namespace Syn;


template<typename T> class Histogram;   // forward decl

#define FIGURE_REDRAW_DATA    0b0001
#define FIGURE_REDRAW_AXES      0b0010
#define FIGURE_REDRAW_LABELS    0b0100
#define FIGURE_REDRAW_SELECTION 0b1000
#define FIGURE_REDRAW_ALL       0b1111

//
template<typename T>
class HistogramRenderObj
{
public:
    friend class Histogram<T>;

public:
    HistogramRenderObj(Histogram<T>* _parent, const glm::vec2& _canvas_sz)
    {
        m_parentRawPtr = _parent;
        m_canvasSz = _canvas_sz;
        m_framebufferID = "hist_canvas" + std::string(Random::rand_str(16));
        m_framebuffer = API::newFramebuffer(ColorFormat::RGBA16F,
                                            glm::vec2(_canvas_sz.x, _canvas_sz.y),
                                            1, 
                                            true, 
                                            false, 
                                            m_framebufferID);
        //
        setupStaticShaders();

        // use default rendering parameters
        m_histParamsPtr = MakeRef<histogram_params_t>(m_canvasSz);
        m_histParamsPtr->bar_spacing_px = 1;
        m_histParamsPtr->x_ticks_between_bars = true;

        // fonts
        m_tickLabelFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                       m_histParamsPtr->ticks_font_size_px,
                                       m_shaderFont,
                                       m_histParamsPtr->figure_sz_px);        
        m_tickLabelFont->disableUpdateOnResize();
        m_tickLabelFont->setColor(m_histParamsPtr->tick_label_color);

        m_titleFont = API::newFont("../assets/ttf/JetBrains/JetBrainsMono-Medium.ttf", 
                                    m_histParamsPtr->title_size,
                                    m_shaderFont,
                                    m_histParamsPtr->figure_sz_px);
        m_titleFont->disableUpdateOnResize();
        m_titleFont->setColor(m_histParamsPtr->title_color);

        // render to framebuffer at least once
        //m_redrawFlags = FIGURE_REDRAW_DATA | FIGURE_REDRAW_AXES | FIGURE_REDRAW_LABELS;
        m_redrawFlags = FIGURE_REDRAW_ALL;
    }

    /* Sets the canvas in 'interactive' mode, i.e. permitting selections and mouse
     * interactions. For the histogram, selections are enabled.
     */
    void setInteractiveMode(bool _b);

    /* Render the canvas using the private framebuffer. After rendering, the default
     * framebufer is bound (->bind(0)).
     */
    void render();
    
    /* Selects an interval, rendered as overlay. Coordinates is set based on which bins
     * are within (and including) the selected interval.
     */
    void selectInterval(const T& _lo, const T& _hi);
    void updateInterval();

    /* Redraws all figure elements using m_histParams.
     */
    void redraw();


    // Accessors
    T& intervalLo() { return m_currInterval.first; }
    T& intervalHi() { return m_currInterval.second; }


private:
    void redrawBars(normalized_params_t* _hist_params);
    void redrawAxes(normalized_params_t* _hist_params);
    void redrawTickLabels(normalized_params_t* _hist_params);
    void redrawSelection(normalized_params_t* _hist_params);
    void setupStaticShaders();


private:
    Histogram<T>* m_parentRawPtr;
    
    glm::vec2 m_canvasSz = { 0.0f, 0.0f };
    Ref<histogram_params_t> m_histParamsPtr;

    std::pair<std::vector<std::string>, std::vector<std::string>> m_axisLabels;   // first = x, second = y
    
    Ref<Framebuffer> m_framebuffer = nullptr;
    std::string m_framebufferID = "";
    Ref<Shader> m_shaderCanvas = nullptr;
    Ref<Shader> m_shaderFont = nullptr;

    float m_maxBinCount = 0.0f;

    uint32_t m_redrawFlags = 0;

    Ref<VertexArray> m_vaoBars = nullptr;
    uint32_t m_barsVertexCount = 0;
    Ref<VertexArray> m_vaoAxes = nullptr;
    uint32_t m_axesVertexCount = 0;
    
    bool m_interactiveMode = false;         // selections possible for this histogram?
    std::pair<T, T> m_currInterval = { 0, 0 };
    std::pair<T, T> m_prevInterval = { -1, -1 };
    Ref<VertexArray> m_vaoFill = nullptr;
    std::vector<float> m_xTickPositionsX;   // used for selection, populated in redraw()
    float m_xTickInterval = 0.0f;

    std::vector<glm::vec2> m_tickLabelPositionsX;
    std::vector<std::string> m_tickLabelsX;

    std::vector<glm::vec2> m_tickLabelPositionsY;
    std::vector<std::string> m_tickLabelsY;

    Ref<Font> m_tickLabelFont = nullptr;
    Ref<Font> m_titleFont = nullptr;

};


