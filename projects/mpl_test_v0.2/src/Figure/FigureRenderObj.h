
#pragma once

#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Renderer/Buffers/Framebuffer.hpp>
#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include <Synapse/Renderer/Buffers/VertexBuffer.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/Shader/ShaderLibrary.hpp>

#include <Synapse/Utils/Random/Random.hpp>

#include "FigureParameters.h"


#define FIGURE_REDRAW_DATA          0b00000001
#define FIGURE_REDRAW_AXES          0b00000010
#define FIGURE_REDRAW_TICKS         0b00000100
#define FIGURE_REDRAW_TICKLABELS    0b00001000
#define FIGURE_REDRAW_SELECTION     0b00010000
#define FIGURE_REDRAW_ALL           0b00011111


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

        protected:
            /* Constructs the frambuffer; requires figure size to be set, i.e. best 
             * called from derived member.
             */
            void initialize();

        public:
            /* Redraws all figure elements except the data. Since tick positions and 
             * tick labels are dependent on the data, redrawData() of the derived class 
             * must be called beforehand.
             */
            void redrawAxes(normalized_params_t* _fig_params);
            void redrawTicks(normalized_params_t* _fig_params);
            void redrawTickLabels(normalized_params_t* _fig_params);
            void redrawSelection(normalized_params_t* _fig_params);

            
            /* Creates temporary shaders for rendering.
             */
        private:
            void setup_static_shaders();
            int num_digits_float(float _f);


        public:
            // Set by FigureRenderObjBase
            Ref<Shader> m_shader2D = nullptr;
            //Ref<Shader> m_shaderLinePlot2D = nullptr;
            Ref<Shader> m_shaderFont = nullptr;
            bool m_shaderInitialized = false;

            Figure* m_parentRawPtr = nullptr;
            glm::vec2 m_figureSizePx = { 0.0f, 0.0f };
            figure_params_t* m_figureParamsPtr = nullptr;
            
            Ref<Framebuffer> m_framebuffer = nullptr;
            std::string m_framebufferID = "";
            std::string m_figureTitle = "";

            uint32_t m_redrawFlags = FIGURE_REDRAW_ALL;

            Ref<VertexArray> m_vaoAxes = nullptr;
            uint32_t m_axesVertexCount = 0;
            Ref<VertexArray> m_vaoTicks = nullptr;
            uint32_t m_ticksVertexCount = 0;

            std::vector<glm::vec2> m_tickLabelPositionsX;
            std::vector<glm::vec2> m_tickLabelPositionsY;
            std::vector<std::string> m_tickLabelsX;
            std::vector<std::string> m_tickLabelsY;

            Ref<Font> m_titleFont = nullptr;
            Ref<Font> m_axisLabelFont = nullptr;
            Ref<Font> m_tickLabelFont = nullptr;
        };


    }

}
