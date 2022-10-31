
#pragma once

#include "../FigureRenderObjBase.h"


namespace Syn
{
    namespace mplc
    {
        class LinePlot; // forward decl

        //
        class LinePlotRenderObj : public FigureRenderObjBase
        {
        public:
            
            LinePlotRenderObj(LinePlot* _parent, const glm::vec2& _fig_sz);
            ~LinePlotRenderObj() = default;


            /*---------------------------------------------------------------------------
             * Pure virtual member functions
             *---------------------------------------------------------------------------
             */

            /* Redraw/update the data part of the canvas.
             */
            virtual void redrawData(normalized_params_t* _fig_params) override;

            /* Exports Figure as a PNG.
             */
            virtual void saveAsPNG(const std::string& _file_name) override { this->saveAsPNGBaseClass(_file_name); }


            /*---------------------------------------------------------------------------
             * Virtual (optional) member functions
             *---------------------------------------------------------------------------
             */

            /* Selects an interval, rendered as overlay. 
             */
            virtual void fillBetweenX(const float& _x0, const float& _x1) override;
            virtual void fillBetweenX() override { fillBetweenX(m_currFillLimX[0], m_currFillLimX[1]); }
            virtual void fillBetweenY(const float& _y0, const float& _y1) override;
            virtual void fillBetweenY() override { fillBetweenY(m_currFillLimY[0], m_currFillLimY[1]); }    


            /*---------------------------------------------------------------------------
             * Pure virtual accessors
             *---------------------------------------------------------------------------
             */

            /* Sets the canvas in 'interactive' mode, i.e. permitting selections and mouse
             * interactions. For the histogram, selections are enabled.
             */
            virtual void setInteractiveMode(bool _b) override { /* TODO : implmentation when needed. */}


        public:
            LinePlot* m_parentRawPtr = nullptr;

        };

    }
}
