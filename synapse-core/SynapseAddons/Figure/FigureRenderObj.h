
#pragma once

#include "../../SynapseCore/API/OpenGLBindings.hpp"
#include "../../SynapseCore/Renderer/Buffers/Framebuffer.hpp"
#include "../../SynapseCore/Renderer/Buffers/VertexArray.hpp"
#include "../../SynapseCore/Renderer/Buffers/VertexBuffer.hpp"
#include "../../SynapseCore/Renderer/Shader/Shader.hpp"
#include "../../SynapseCore/Renderer/Shader/ShaderLibrary.hpp"

#include "../../SynapseCore/Utils/Random/Random.hpp"

#include "FigureParameters.h"


#define FIGURE_REDRAW_DATA          0b00000001
#define FIGURE_REDRAW_AXES          0b00000010
#define FIGURE_REDRAW_TICKS         0b00000100
#define FIGURE_REDRAW_TICKLABELS    0b00001000
#define FIGURE_REDRAW_FILL          0b00010000
#define FIGURE_REDRAW_AUX           0b00001110  // axes, tick, tick labels
#define FIGURE_REDRAW_ALL           0b00011111  // all elements
#define FIGURE_REDRAW               FIGURE_REDRAW_ALL

#define FIGURE_RENDER_GRIDLINES     0b00000001
#define FIGURE_RENDER_FILL          0b00000110
#define FIGURE_RENDER_FILL_X        0b00000010
#define FIGURE_RENDER_FILL_Y        0b00000100


namespace Syn
{
    namespace mplc
    {
        class Figure;   // forward decl

        class FigureRenderObj
        {
        public:
            friend class Figure;

        public:
            FigureRenderObj(Figure* _parent);
            ~FigureRenderObj() = default;
            
            /* Render the canvas using the private framebuffer. After rendering, the 
             * default framebufer is bound (->bind(0)). Do not override.
             */
            void render();

            /* Redraw Figure elements (data, axes, ticks, ticklabels) depending on set 
             * m_redrawFlags.
             */
            void redraw();

        private:
            /* Constructs the frambuffer; requires figure size to be set. Called from 
             * parent Figure object.
             */
            void init();

            /* Redraws all figure elements except the data. Since tick positions and 
             * tick labels are dependent on the data, redrawData() of the derived class 
             * must be called beforehand.
             */
            void redrawAxes(normalized_params_t* _fig_params);
            void redrawTicks(normalized_params_t* _fig_params);
            void redrawTickLabels(normalized_params_t* _fig_params);
            void redrawFill(normalized_params_t* _fig_params);

        protected:
            const Ref<Framebuffer>& getFramebuffer() { return m_framebuffer; }
            
        private:
            /* Creates temporary shaders for rendering.
             */
            void setup_static_shaders();

            /* Determine number of decimals for tick labels
             */
            void format_tick_labels(NiceScale* _ticks,
                                    const std::vector<glm::vec2>& _label_positions);


        public:
            // Set by FigureRenderObjBase
            Ref<Shader> m_shader2D = nullptr;
            //Ref<Shader> m_shaderLinePlot2D = nullptr;
            Ref<Shader> m_shaderFont = nullptr;
            bool m_shaderInitialized = false;

            Figure* m_parentRawPtr = nullptr;
            glm::vec2 m_figureSizePx = { 0.0f, 0.0f };
            figure_params_t* m_figureParamsPtr = nullptr;
            glm::vec2 m_localDataLimX = { 0.0f, 1.0f };
            glm::vec2 m_localDataLimY = { 0.0f, 1.0f };
            
            Ref<Framebuffer> m_framebuffer = nullptr;
            std::string m_framebufferID = "";
            //std::string m_figureTitle = ""; -- retrieved from parent Figure obj

            uint32_t m_redrawFlags = FIGURE_REDRAW_ALL;
            uint32_t m_auxRenderFlags = FIGURE_RENDER_GRIDLINES;

            Ref<VertexArray> m_vaoAxes = nullptr;
            uint32_t m_axesVertexCount = 0;
            Ref<VertexArray> m_vaoTicks = nullptr;
            uint32_t m_ticksVertexCount = 0;

            std::vector<glm::vec2> m_tickLabelPositionsX;
            std::vector<glm::vec2> m_tickLabelPositionsY;

            Ref<Font> m_titleFont = nullptr;
            Ref<Font> m_axisLabelFont = nullptr;
            Ref<Font> m_tickLabelFont = nullptr;

            // auxilliary objects
            Ref<VertexArray> m_vaoGridLines = nullptr;
            uint32_t m_gridLinesVertexCount = 0;
            glm::vec2 m_fillLimX = { 1.0f, -1.0f };
            glm::vec2 m_fillLimY = { 1.0f, -1.0f };
            Ref<VertexArray> m_vaoFill = nullptr;
            uint32_t m_fillVertexCount = 0;
        };


    }

}
