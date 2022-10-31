
#include "ScatterPlot.h"

namespace Syn
{
    namespace mplc
    {
        //-------------------------------------------------------------------------------
        // CONSTRUCTORS
        //-------------------------------------------------------------------------------
        //
        ScatterPlot::ScatterPlot(const glm::vec2& _fig_size)
        {
            setScatterPlotRenderObjPtr(_fig_size);
        }
        //-------------------------------------------------------------------------------
        ScatterPlot::ScatterPlot(const std::vector<float>& _data, const glm::vec2& _fig_size)
        {
            setScatterPlotRenderObjPtr(_fig_size);
            data(_data);
        }
        //-------------------------------------------------------------------------------
        ScatterPlot::ScatterPlot(const std::vector<int>& _data, const glm::vec2& _fig_size)
        {
            setScatterPlotRenderObjPtr(_fig_size);
            std::vector<float> fdata(_data.begin(), _data.end());
        }
        //-------------------------------------------------------------------------------
        ScatterPlot::ScatterPlot(const std::vector<float>& _X, const std::vector<float>& _Y, const glm::vec2& _fig_size)
        {
            setScatterPlotRenderObjPtr(_fig_size);
            data(_X, _Y);
        }
        //-------------------------------------------------------------------------------
        ScatterPlot::ScatterPlot(const std::vector<int>& _X, const std::vector<int>& _Y, const glm::vec2& _fig_size)
        {
            setScatterPlotRenderObjPtr(_fig_size);
            std::vector<float> fx(_X.begin(), _X.end());
            std::vector<float> fy(_Y.begin(), _Y.end());
            data(fx, fy);
        }
        //-------------------------------------------------------------------------------
        // ScatterPlot-SPECIFIC MEMBER FUNCTIONS
        //-------------------------------------------------------------------------------
        //
        void ScatterPlot::data(const std::vector<float>& _data)
        {
            m_dataY.clear();
            m_dataY = std::vector<float>(_data);

            m_dataDim0 = m_dataY.size();

            // no X data array provided, the X values will be np.arange(_data.shape[0])
            m_dataX.clear();
            for (size_t i = 0; i < m_dataDim0; i++)
                m_dataX.push_back(i);
            m_renderObjPtr->m_dataLimX = { 0, m_dataDim0 };

            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_ALL;
            m_renderObjPtr->m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
            if (m_renderObjPtr->m_interactiveMode)
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        }
        //-------------------------------------------------------------------------------
        void ScatterPlot::data(const std::vector<float>& _X, const std::vector<float>& _Y)
        {
            m_dataX = std::vector<float>(_X);
            m_renderObjPtr->m_dataLimX[0] = *std::min_element(m_dataX.begin(), m_dataX.end());
            m_renderObjPtr->m_dataLimX[1] = *std::max_element(m_dataX.begin(), m_dataX.end());

            m_dataY.clear();
            m_dataY = std::vector<float>(_Y);
            
            m_dataDim0 = m_dataY.size();

            SYN_CORE_ASSERT(m_dataX.size() == m_dataDim0, "X and Y data are of different dimensionality.");

            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_ALL;
            m_renderObjPtr->m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
            if (m_renderObjPtr->m_interactiveMode)
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        }
    }
}
