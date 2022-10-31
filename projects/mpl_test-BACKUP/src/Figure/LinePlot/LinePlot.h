
#pragma once


#include "../FigureBase.h"
#include "LinePlotRenderObj.h"


namespace Syn
{
    namespace mplc
    {
        class LinePlot : public FigureBase
        {
        public:
            friend class LinePlotRenderObj;

        public:
            /*---------------------------------------------------------------------------
             * Constructors / destructors
             *---------------------------------------------------------------------------
             */
            
            /* Default constructors -- data has to be set manually using data()
             */
            LinePlot(const glm::vec2& _fig_size=glm::vec2(0));

            /* Constructors for setting Y data on initialization.
             */
            LinePlot(const std::vector<float>& _data, const glm::vec2& _fig_size=glm::vec2(0));
            LinePlot(const std::vector<int>& _data, const glm::vec2& _fig_size=glm::vec2(0));

            /* Constructors for setting X and Y data on initialization.
             */
            LinePlot(const std::vector<float>& _X, const std::vector<float>& _Y, const glm::vec2& _fig_size=glm::vec2(0));
            LinePlot(const std::vector<int>& _X, const std::vector<int>& _Y, const glm::vec2& _fig_size=glm::vec2(0));

            /* Constructors for setting multiple Y sets on initialization.
             */
            LinePlot(const std::vector<std::vector<float>>& _data, const glm::vec2& _fig_size=glm::vec2(0));
            LinePlot(const std::vector<std::vector<int>>& _data, const glm::vec2& _fig_size=glm::vec2(0));

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
             * LinePlot-specific member functions
             *---------------------------------------------------------------------------
             */

            /* Data functions. Data (Y values) are provided either on its own, in which 
             * case the X values are deduced (as a linear array with the same 
             * dimensionality), or together with preset X values. Values (both X and Y) 
             * may be provided as integers (int) and if so, will be converted to floats.
             * Additionally, data may be entered as multiple sets of data, as an array of 
             * arrays. Here, X is deduced. If integers are provided, conversion to float 
             * will be done.
             */
            void data(const std::vector<float>& _data);
            void data(const std::vector<int>& _data);
            void data(const std::vector<float>& _X, const std::vector<float>& _Y);
            void data(const std::vector<int>& _X, const std::vector<int>& _Y);
            void data(const std::vector<std::vector<float>>& _data);
            void data(const std::vector<std::vector<int>>& _data);


            /*---------------------------------------------------------------------------
             * LinePlot-specific accessors
             *---------------------------------------------------------------------------
             */
            const std::vector<float>& dataX()               { return m_dataX; }
            const std::vector<std::vector<float>>& dataY()  { return m_dataY; }


            /*---------------------------------------------------------------------------
             * LinePlot-specific private member functions
             *---------------------------------------------------------------------------
             */
        private:
            inline void setLinePlotRenderObjPtr(const glm::vec2& _fig_sz)
            {
                glm::vec2 fig_sz = check_fig_size(_fig_sz);
                m_renderObjPtr = std::make_shared<LinePlotRenderObj>(this, _fig_sz);
            }


            /*---------------------------------------------------------------------------
             * LinePlot-specific member variables
             *---------------------------------------------------------------------------
             */
        private:
            std::shared_ptr<LinePlotRenderObj> m_renderObjPtr = nullptr;

            std::vector<float> m_dataX;
            std::vector<std::vector<float>> m_dataY;

            size_t m_dataYDim0 = 0;
            size_t m_maxDataYDim1 = 0;
        };

    }
}