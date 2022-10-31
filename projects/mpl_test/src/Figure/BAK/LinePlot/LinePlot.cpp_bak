
#include "LinePlot.h"

namespace Syn
{
    namespace mplc
    {
        //-------------------------------------------------------------------------------
        // CONSTRUCTORS
        //-------------------------------------------------------------------------------
        //
        LinePlot::LinePlot(const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
        }
        //-------------------------------------------------------------------------------
        LinePlot::LinePlot(const std::vector<float>& _data, const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
            data(_data);
        }
        //-------------------------------------------------------------------------------
        LinePlot::LinePlot(const std::vector<int>& _data, const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
            std::vector<float> fdata(_data.begin(), _data.end());
        }
        //-------------------------------------------------------------------------------
        LinePlot::LinePlot(const std::vector<float>& _X, const std::vector<float>& _Y, const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
            data(_X, _Y);
        }
        //-------------------------------------------------------------------------------
        LinePlot::LinePlot(const std::vector<int>& _X, const std::vector<int>& _Y, const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
            std::vector<float> fx(_X.begin(), _X.end());
            std::vector<float> fy(_Y.begin(), _Y.end());
            data(fx, fy);
        }
        //-------------------------------------------------------------------------------
        LinePlot::LinePlot(const std::vector<std::vector<float>>& _data, const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
            data(_data);
        }
        //-------------------------------------------------------------------------------
        LinePlot::LinePlot(const std::vector<std::vector<int>>& _data, const glm::vec2& _fig_size)
        {
            setLinePlotRenderObjPtr(_fig_size);
            std:vector<std::vector<float>> ffdata;
            for (size_t i = 0; i < _data.size(); i++)
            {
                std::vector<float> fdata(_data[i].begin(), _data[i].end());
                ffdata.push_back(fdata);
            }
            data(ffdata);
        }
        //-------------------------------------------------------------------------------
        // LINEPLOT-SPECIFIC MEMBER FUNCTIONS
        //-------------------------------------------------------------------------------
        //
        void LinePlot::data(const std::vector<float>& _data)
        {
            m_dataY.clear();
            m_dataY.push_back(std::vector<float>(_data));

            m_dataYDim0 = m_dataY.size();
            m_maxDataYDim1 = m_dataY[0].size(); 

            // no X data array provided, the X values will be np.arange(_data.shape[0])
            m_dataX.clear();
            for (size_t i = 0; i < m_maxDataYDim1; i++)
                m_dataX.push_back(i);
            m_renderObjPtr->m_dataLimX = { 0, m_maxDataYDim1 };

            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_ALL;
            m_renderObjPtr->m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
            if (m_renderObjPtr->m_interactiveMode)
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        }
        //-------------------------------------------------------------------------------
        void LinePlot::data(const std::vector<float>& _X, const std::vector<float>& _Y)
        {
            m_dataX = std::vector<float>(_X);
            m_renderObjPtr->m_dataLimX[0] = *std::min_element(m_dataX.begin(), m_dataX.end());
            m_renderObjPtr->m_dataLimX[1] = *std::max_element(m_dataX.begin(), m_dataX.end());

            m_dataY.clear();
            m_dataY.push_back(std::vector<float>(_Y));
            
            m_dataYDim0 = m_dataY.size();
            m_maxDataYDim1 = m_dataY[0].size();

            SYN_CORE_ASSERT(m_dataX.size() == m_maxDataYDim1, "X and Y data are of different dimensionality.");

            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_ALL;
            m_renderObjPtr->m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
            if (m_renderObjPtr->m_interactiveMode)
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        }
        //-------------------------------------------------------------------------------
        void LinePlot::data(const std::vector<std::vector<float>>& _data)
        {
            m_dataY = _data;

            // no X data array provided, the X values will be np.arange(_data.shape[0]) of longest
            // data row
            m_dataYDim0 = m_dataY.size();
            m_maxDataYDim1 = std::numeric_limits<size_t>::min();
            for (const auto& data_row : m_dataY)
                m_maxDataYDim1 = std::max(m_maxDataYDim1, data_row.size());
            
            m_dataX.clear();
            for (size_t i = 0; i < m_maxDataYDim1; i++)
                m_dataX.push_back(i);
            m_renderObjPtr->m_dataLimX = { 0, m_maxDataYDim1 };

            SYN_CORE_ASSERT(m_dataX.size() == m_maxDataYDim1, "X and Y data are of different dimensionality.");

            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_ALL;
            m_renderObjPtr->m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
            if (m_renderObjPtr->m_interactiveMode)
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        }
    }
}
