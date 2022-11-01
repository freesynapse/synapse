
#pragma once

#include <vector>

#include "FigureRenderObjBase.h"


namespace Syn
{
    namespace mplc
    {
        /* The Figure base class, which all plots inherits from. The Figure is the main, 
         * top hierarchical object. Each Figure object contains the different parts of 
         * the plot; the axes, the canvas (where the data is shown) etc. The different 
         * plots will have constructors for integral types, but underneath all data is
         * represented as floats. 
         */
        class FigureBase
        {
        public:
            FigureBase() {}
            ~FigureBase() = default;

            /*---------------------------------------------------------------------------
             * Pure virtual member functions
             *---------------------------------------------------------------------------
             */
            virtual void render() = 0;      // --> FigureRenderObjBase->render()
            virtual void update() = 0;      // --> FigureRenderObjBase->redraw()
            virtual void redraw() = 0;      // --> FigureRenderObjBase->redraw()
            virtual void saveFigure(const std::string& _file_name) = 0;  // --> FigureRenderObjBase->saveAsPNG()
            virtual void interactiveMode(bool _b) = 0; // --> FigureRenderObjBase->interactiveMode(_b)


            /*---------------------------------------------------------------------------
             * Virtual (optional) member functions
             *---------------------------------------------------------------------------
             */
            virtual void fillBetweenX(const float& _x0=0, const float& _x1=0) {}
            virtual void fillBetweenX() {}
            virtual void fillBetweenY(const float& _y0=0, const float& _y1=0) {}
            virtual void fillBetweenY() {}


            /*---------------------------------------------------------------------------
             * Accessors
             *---------------------------------------------------------------------------
             */
            virtual const std::vector<float>& data() { return m_data; }
            virtual void title(const std::string& _title) { m_figureTitle = _title; }
            virtual const std::string& title() { return m_figureTitle; }
            virtual bool isIntegralType() { return m_integralType; }
            virtual bool isFloatingPointType() { return !m_integralType; }


            /*---------------------------------------------------------------------------
             * Pure virtual accessors (depending on the type of FigureRenderObj)
             *---------------------------------------------------------------------------
             */
            virtual inline GLuint figurePtrID() = 0;
            virtual inline ImVec2 figureSz() = 0;
            virtual Ref<figure_params_t> params() = 0;
            virtual inline bool toUpdate() = 0;


            /*---------------------------------------------------------------------------
             * Pure virtual accessors (depending on the type of FigureRenderObj)
             *---------------------------------------------------------------------------
             */
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wreturn-type"
            virtual float& fillLimX0() {}
            virtual float& fillLimX1() {}
            virtual glm::vec2& fillLimX() {}
            virtual float& fillLimY0() {}
            virtual float& fillLimY1() {}
            virtual glm::vec2& fillLimY() {}
            #pragma clang diagnostic pop
            

        protected:
            /* Checks figure size at FigureObj and uses default if needed.
             */
            glm::vec2 check_fig_size(const glm::vec2& _fig_sz)
            {
                if (_fig_sz.x <= 0 || _fig_sz.y <= 0)
                    return FIGURE_DEFUALT_SIZE; // in FigureParameters.h
                return _fig_sz;
            }


        public:
            std::vector<float> m_data;
            
            std::string m_figureTitle = "";
            bool m_integralType = false;
            
            //std::shared_ptr<FigureRenderObjBase> m_renderObjPtr;
        };
    }
}

