
#pragma once


#pragma once


#include "../FigureBase.h"
#include "ScatterPlotRenderObj.h"


namespace Syn
{
    namespace mplc
    {
        class ScatterPlot : public FigureBase
        {
        public:
            friend class ScatterPlotRenderObj;

        public:
            /*---------------------------------------------------------------------------
             * Constructors / destructors
             *---------------------------------------------------------------------------
             */
            
            /* Default constructors -- data has to be set manually using data()
             */
            ScatterPlot(const glm::vec2& _fig_size=glm::vec2(0));

            /* Constructors for setting Y data on initialization.
             */
            ScatterPlot(const std::vector<float>& _data, const glm::vec2& _fig_size=glm::vec2(0));
            ScatterPlot(const std::vector<int>& _data, const glm::vec2& _fig_size=glm::vec2(0));

            /* Constructors for setting X and Y data on initialization.
             */
            ScatterPlot(const std::vector<float>& _X, const std::vector<float>& _Y, const glm::vec2& _fig_size=glm::vec2(0));
            ScatterPlot(const std::vector<int>& _X, const std::vector<int>& _Y, const glm::vec2& _fig_size=glm::vec2(0));

            /*---------------------------------------------------------------------------
             * Pure virtual member functions
             *---------------------------------------------------------------------------
             */
            virtual void render() override { m_renderObjPtr->render(); }
            virtual void update() override { m_renderObjPtr->redraw(); }
            virtual void redraw() override { m_renderObjPtr->redraw(); }
            virtual void interactiveMode(bool _b) override { m_renderObjPtr->setInteractiveMode(_b); }
            virtual void saveFigure(const std::string& _file_name) override { m_renderObjPtr->saveAsPNG(_file_name); }


            /*---------------------------------------------------------------------------
             * Pure virtual accessors (depending on the type of FigureRenderObj)
             *---------------------------------------------------------------------------
             */
            virtual inline GLuint figurePtrID() override { return m_renderObjPtr->m_framebuffer->getColorAttachmentIDn(0); }
            virtual inline ImVec2 figureSz() override { return ImVec2(m_renderObjPtr->m_figureSz.x, m_renderObjPtr->m_figureSz.y); }
            virtual Ref<figure_params_t> params() override { return m_renderObjPtr->m_figureParamsPtr; }
            virtual inline bool toUpdate() override { return m_renderObjPtr->m_redrawFlags; }


            /*---------------------------------------------------------------------------
             * ScatterPlot-specific member functions
             *---------------------------------------------------------------------------
             */

            /* Data functions. If no X coordinates are entered, the coordinates are set
             * to a linear scale from 0 to the dimensionality of the data.
             */
            void data(const std::vector<float>& _data);
            void data(const std::vector<int>& _data);
            void data(const std::vector<float>& _X, const std::vector<float>& _Y);
            void data(const std::vector<int>& _X, const std::vector<int>& _Y);


            /*---------------------------------------------------------------------------
             * LinePlot-specific accessors
             *---------------------------------------------------------------------------
             */
            const std::vector<float>& dataX()  { return m_dataX; }
            const std::vector<float>& dataY()  { return m_dataY; }


            /*---------------------------------------------------------------------------
             * LinePlot-specific private member functions
             *---------------------------------------------------------------------------
             */
        private:
            inline void setScatterPlotRenderObjPtr(const glm::vec2& _fig_sz)
            {
                glm::vec2 fig_sz = check_fig_size(_fig_sz);
                m_renderObjPtr = std::make_shared<ScatterPlotRenderObj>(this, _fig_sz);
            }


            /*---------------------------------------------------------------------------
             * LinePlot-specific member variables
             *---------------------------------------------------------------------------
             */
        private:
            std::shared_ptr<ScatterPlotRenderObj> m_renderObjPtr = nullptr;

            std::vector<float> m_dataX;
            std::vector<float> m_dataY;

            size_t m_dataDim0 = 0;
        };
    }
}