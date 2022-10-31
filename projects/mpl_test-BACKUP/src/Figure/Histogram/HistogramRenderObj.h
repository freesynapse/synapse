
#pragma once

#include "../FigureRenderObjBase.h"
#include "../FigureParameters.h"
#include "../FigureUtils.h"


namespace Syn
{
    namespace mplc
    {

        class Histogram;   // forward decl

        class HistogramRenderObj : public FigureRenderObjBase
        {
        public:
            friend class Histogram;

        public:

            HistogramRenderObj(Histogram* _parent, const glm::vec2& _fig_sz);
            ~HistogramRenderObj() = default;


            /*---------------------------------------------------------------------------
             * Pure virtual member functions
             *---------------------------------------------------------------------------
             */

            /* Redraw/update the data part of the canvas.
             */
            virtual void redrawData(normalized_params_t* _params) override;

            /* Exports Figure as a PNG.
             */
            virtual void saveAsPNG(const std::string& _file_name) override { this->saveAsPNGBaseClass(_file_name); }


            /*---------------------------------------------------------------------------
             * Virtual (optional) member functions
             *---------------------------------------------------------------------------
             */

            /* Selects an interval, rendered as overlay. 
             */
            virtual void fillBetweenX(const float& _lo, const float& _hi) override;
            virtual void fillBetweenX() override { fillBetweenX(m_currFillLimX[0], m_currFillLimX[1]); }


            /*---------------------------------------------------------------------------
             * Pure virtual accessors
             *---------------------------------------------------------------------------
             */

            /* Sets the canvas in 'interactive' mode, i.e. permitting selections and mouse
             * interactions. For the histogram, selections are enabled.
             */
            virtual void setInteractiveMode(bool _b) override;


        public:
            Histogram* m_parentRawPtr = nullptr;

            // in FigureRenderObjBase
            //glm::vec2 m_canvasSz = { 0.0f, 0.0f };
            //std::shared_ptr<histogram_params_t> m_histParamsPtr;  --> changed to figure_params_t

            // in FigureRenderObjBase
            //Ref<Framebuffer> m_framebuffer = nullptr;     
            //std::string m_framebufferID = "";
            //Ref<Shader> m_shaderCanvas = nullptr;
            //Ref<Shader> m_shaderFont = nullptr;


            // in FigureRenderObjBase
            //uint32_t m_redrawFlags = 0;

            //Ref<VertexArray> m_vaoBars = nullptr;
            //uint32_t m_barsVertexCount = 0;
            //Ref<VertexArray> m_vaoAxes = nullptr;
            //uint32_t m_axesVertexCount = 0;
            
            //bool m_interactiveMode = false;         // selections possible for this histogram?
            //std::pair<float, float> m_currSelectionX = { 0, 0 };
            //std::pair<float, float> m_prevSelectionX = { -1, -1 };
            //Ref<VertexArray> m_vaoSelection = nullptr;
            //std::vector<float> m_xTickPositionsX;   // used for selection, populated in redraw()
            //float m_tickIntervalX = 0.0f;

            //std::vector<glm::vec2> m_tickLabelPositionsX;
            //std::vector<std::string> m_tickLabelsX;

            //std::vector<glm::vec2> m_tickLabelPositionsY;
            //std::vector<std::string> m_tickLabelsY;

            //std::shared_ptr<Font> m_tickLabelFont = nullptr;
            //std::shared_ptr<Font> m_titleFont = nullptr;

        };
    }
}

