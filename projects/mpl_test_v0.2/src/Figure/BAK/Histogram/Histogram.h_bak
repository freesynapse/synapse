
#pragma once

#include "../FigureBase.h"
#include "../FigureParameters.h"
#include "HistogramRenderObj.h"


namespace Syn
{
    namespace mplc
    {
        class Histogram : public FigureBase
        {
        public:
            friend class HistogramRenderObj;

        public:

            /*---------------------------------------------------------------------------
             * Constructors / destructors
             *---------------------------------------------------------------------------
             */
            
            /* Default constructor -- data has to be set manually using data()
             */
            Histogram(const glm::vec2& _fig_size=glm::vec2(0));

            /* Constructor for setting data and bins on initialization. If _nbins is set 
             * to 0, number of bins will be deduced from the data.
             */
            Histogram(const std::vector<float>& _data, size_t _nbins=0, const glm::vec2& _fig_size=glm::vec2(0));
            Histogram(const std::vector<int>& _data, size_t _nbins=0, const glm::vec2& _fig_size=glm::vec2(0));
            

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
             * Virtual (optional) member functions
             *---------------------------------------------------------------------------
             */
            virtual void fillBetweenX(const float& _x0, const float& _x1) override { m_renderObjPtr->fillBetweenX(_x0, _x1); }
            virtual void fillBetweenX() override { m_renderObjPtr->fillBetweenX(); }


            /*---------------------------------------------------------------------------
             * Pure virtual accessors (depending on the type of FigureRenderObj)
             *---------------------------------------------------------------------------
             */
            virtual inline GLuint figurePtrID() override { return m_renderObjPtr->m_framebuffer->getColorAttachmentIDn(0); }
            virtual inline ImVec2 figureSz() override { return ImVec2(m_renderObjPtr->m_figureSz.x, m_renderObjPtr->m_figureSz.y); }
            virtual Ref<figure_params_t> params() override { return m_renderObjPtr->m_figureParamsPtr; }
            virtual inline bool toUpdate() override { return m_renderObjPtr->m_redrawFlags; }


            /*---------------------------------------------------------------------------
             * Virtual (optional) accessors (depending on the type of FigureRenderObj)
             *---------------------------------------------------------------------------
             */    
            virtual float& fillLimX0() override { return m_renderObjPtr->fillLimX0(); }
            virtual float& fillLimX1() override { return m_renderObjPtr->fillLimX1(); }
            virtual glm::vec2& fillLimX() override { return m_renderObjPtr->fillLimX(); }


            /*---------------------------------------------------------------------------
             * Histogram-specific member functions
             *---------------------------------------------------------------------------
             */
            void data(const std::vector<float>& _data, size_t _nbins=0);
            // overload for int data
            void data(const std::vector<int>& _data, size_t _nbins=0);

            const std::map<float, size_t>& bins() { return m_bins; }
            inline size_t& binCount() { return m_binCount; }
            inline const float& minBin() { return m_bins.begin()->first; }
            inline const float& maxBin() { return (m_integralType ? m_bins.rbegin()->first : std::prev(m_bins.rbegin())->first); }


            /*---------------------------------------------------------------------------
             * LinePlot-specific private member functions
             *---------------------------------------------------------------------------
             */
        private:
            /* Finds the bin closest to a value and returns an iterator. 
             */
            std::map<float, size_t>::iterator findClosestBin(const float& _val);

            /* Creates bins for the data. If number of bins are unspecified when data is 
             * entered, an appropriate number of bins will be created.
             */
            void createBins(float _min, float _max);

            /* Initializer used for all constructors.
             */
            inline void setHistogramRenderObjPtr(const glm::vec2& _fig_sz)
            {
                glm::vec2 fig_sz = check_fig_size(_fig_sz);
                m_renderObjPtr = std::make_shared<HistogramRenderObj>(this, _fig_sz);
            }


            /*---------------------------------------------------------------------------
            * Histogram member variables
            *---------------------------------------------------------------------------
            */
        public:
            size_t m_binCount = 0;
            std::map<float, size_t> m_bins;
            
            std::shared_ptr<HistogramRenderObj> m_renderObjPtr = nullptr;

            // in FigureBase
            //std::vector<float> m_data;
            //std::string m_figureTitle = "";
            //bool m_integralType = false;

        };

    }

}
