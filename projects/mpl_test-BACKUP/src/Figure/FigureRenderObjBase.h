
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
        class FigureBase;   // forward decl

        class FigureRenderObjBase
        {
        public:

            FigureRenderObjBase();
            ~FigureRenderObjBase() = default;
            
            /* Render the canvas using the private framebuffer. After rendering, the 
             * default framebufer is bound (->bind(0)). Do not override.
             */
            virtual void render();

            /* Redraw Figure elements (data, axes, ticks, ticklabels) depending on set 
             * m_redrawFlags.
             */
            virtual void redraw();

        protected:
            /* Constructs the frambuffer; requires figure size to be set, i.e. best 
             * called from derived member.
             */
            void initializeBaseClass();

            /* Exports the framebuffer texture to PNG.
             */
            void saveAsPNGBaseClass(const std::string& _file_name) { printf("HEHEHEHEHE!"); };


        private:
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
            void setupStaticShaders();


            /*---------------------------------------------------------------------------
             * Pure virtual member functions
             *---------------------------------------------------------------------------
             */
        public:
            /* Redraw/update the data part of the Figure.
             */
            virtual void redrawData(normalized_params_t* _params) = 0;

            /* Exports the framebuffer texture to PNG.
             */
            virtual void saveAsPNG(const std::string& _file_name) = 0;


            /*---------------------------------------------------------------------------
             * Virtual (optional) member functions
             *---------------------------------------------------------------------------
             */
            
            /* Selects an interval, rendered as overlay. 
             */
            virtual void fillBetweenX(const float& _x0, const float& _x1) {}
            virtual void fillBetweenX() {} // --> calls fillBetweenX(float, float) with current limits
            virtual void fillBetweenY(const float& _y0, const float& _y1) {}
            virtual void fillBetweenY() {} // --> calls fillBetweenY(float, float) with current limits


            /*---------------------------------------------------------------------------
             * Pure virtual accessors
             *---------------------------------------------------------------------------
             */

            /* Sets the canvas in 'interactive' mode, i.e. permitting selections and mouse
             * interactions. For the histogram, selections are enabled.
             */
            virtual void setInteractiveMode(bool _b) = 0;


            /*---------------------------------------------------------------------------
             * Virtual (optional) accessors
             *---------------------------------------------------------------------------
             */
            virtual float& fillLimX0()      { return m_currFillLimX[0]; };
            virtual float& fillLimX1()      { return m_currFillLimX[1]; };
            virtual glm::vec2& fillLimX()   { return m_currFillLimX; }
            virtual float& fillLimY0()      { return m_currFillLimY[0]; };
            virtual float& fillLimY1()      { return m_currFillLimY[1]; };
            virtual glm::vec2& fillLimY()   { return m_currFillLimY; }


        public:
            // Set by FigureRenderObjBase
            Ref<Shader> m_shader2D = nullptr;
            //Ref<Shader> m_shaderLinePlot2D = nullptr;
            Ref<Shader> m_shaderFont = nullptr;
            bool m_shaderInitialized = false;

            //FigureBase* m_parentRawPtr = nullptr; --> needs to be explicitly set by derived
            glm::vec2 m_figureSz = { 0.0f, 0.0f };
            std::shared_ptr<figure_params_t> m_figureParamsPtr;    
            
            Ref<Framebuffer> m_framebuffer = nullptr;
            std::string m_framebufferID = "";
            std::string m_figureTitle = "";

            uint32_t m_redrawFlags = FIGURE_REDRAW_ALL;

            Ref<VertexArray> m_vaoData = nullptr;
            uint32_t m_dataVertexCount = 0;
            uint32_t m_dataOpenGLPrimitive = GL_TRIANGLES;
            Ref<VertexArray> m_vaoAxes = nullptr;
            uint32_t m_axesVertexCount = 0;
            Ref<VertexArray> m_vaoTicks = nullptr;
            uint32_t m_ticksVertexCount = 0;
            std::vector<glm::vec2> m_dataPositions; // set in derived redrawData(), used to find tick positions

            glm::vec2 m_dataLimX = { 0, 0 };        // min and max X value
            glm::vec2 m_dataLimY = { 0, 0 };        // min and max Y value

            bool m_interactiveMode = false;         // selections possible for this histogram?
            glm::vec2 m_currFillLimX = {  0,  0 };
            glm::vec2 m_prevFillLimX = { -1, -1 };
            glm::vec2 m_currFillLimY = {  0,  0 };
            glm::vec2 m_prevFillLimY = { -1, -1 };
            glm::vec2 m_fillPositionsSelX[2];       // The positions (in normalized coordinates) of min/max
            glm::vec2 m_fillPositionsSelY[2];       // X and Y values. Set in derived->fillBetweenX/Y, 
                                                    // vertex geometry set in this->redrawSelection().
            uint32_t m_selectionVertexCount = 0;
            Ref<VertexArray> m_vaoSelection = nullptr;

            std::vector<glm::vec2> m_tickLabelPositionsX;
            std::vector<glm::vec2> m_tickLabelPositionsY;
            std::vector<std::string> m_tickLabelsX;
            std::vector<std::string> m_tickLabelsY;

            std::shared_ptr<Font> m_tickLabelFont = nullptr;
            std::shared_ptr<Font> m_titleFont = nullptr;

        };


    }

}
