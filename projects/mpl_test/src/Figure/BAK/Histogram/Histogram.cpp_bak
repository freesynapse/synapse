
#include "HistogramRenderObj.h"
#include "Histogram.h"

namespace Syn
{
    namespace mplc
    {
        //-------------------------------------------------------------------------------
        // CONSTRUCTORS
        //-------------------------------------------------------------------------------
        //
        Histogram::Histogram(const glm::vec2& _fig_size)
        {
            setHistogramRenderObjPtr(_fig_size);
        }
        Histogram::Histogram(const std::vector<float>& _data, size_t _nbins, const glm::vec2& _fig_size)
        {
            setHistogramRenderObjPtr(_fig_size);
            data(_data, _nbins);
        }
        Histogram::Histogram(const std::vector<int>& _data, size_t _nbins, const glm::vec2& _fig_size)
        {
            setHistogramRenderObjPtr(_fig_size);
            m_integralType = true;
            std::vector<float> fdata(_data.begin(), _data.end());
            data(fdata, _nbins);
        }
        //-------------------------------------------------------------------------------
        // PUBLIC MEMBER FUNCTIONS
        //-------------------------------------------------------------------------------
        //
        void Histogram::data(const std::vector<int>& _data, size_t _nbins)
        {
            m_integralType = true;
            std::vector<float> fdata(_data.begin(), _data.end());
            data(fdata, _nbins);
        }
        //-------------------------------------------------------------------------------
        void Histogram::data(const std::vector<float>& _data, size_t _nbins)
        {
            // copy of the data
            m_data = std::vector<float>(_data);

            //std::pair<T, T> lim = std::minmax(_data.begin(), _data.end());
            float max = std::numeric_limits<float>::min();
            float min = std::numeric_limits<float>::max();
            for (const auto& val : _data)
            {
                min = std::min(min, val);
                max = std::max(max, val);
            }
            std::pair<float, float> lim = { min, max };
            
            m_binCount = _nbins;
            createBins(lim.first, lim.second);
            
            for (auto& val : _data)
            {
                if (val < m_bins.begin()->first || val > m_bins.rbegin()->first)
                    continue;

                std::prev(m_bins.upper_bound(val))->second++;
            }

            // redraw geometry
            m_renderObjPtr->m_currFillLimX = { minBin(), maxBin() };
            m_renderObjPtr->m_prevFillLimX = { 0, 0 };
            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_ALL;
            m_renderObjPtr->m_redrawFlags &= (~ FIGURE_REDRAW_SELECTION);
            if (m_renderObjPtr->m_interactiveMode)
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
        }
        //-------------------------------------------------------------------------------
        // PRIVATE MEMBER FUNCTIONS
        //-------------------------------------------------------------------------------
        //
        std::map<float, size_t>::iterator Histogram::findClosestBin(const float& _val)
        {
            if (m_integralType)
                return m_bins.find(_val);
            
            auto low = m_bins.lower_bound(_val);
            if (low == m_bins.begin())
                return m_bins.begin();
            else
            {
                auto prev = std::prev(low);
                if ((_val - prev->first) < (low->first - _val))
                    return prev;
                else
                    return low;
            }
        }
        //-------------------------------------------------------------------------------
        void Histogram::createBins(float _min, float _max)
        {
            m_bins.clear();
            float x = _min;

            // integer types are special cases
            if ((_max - _min) < m_binCount && m_integralType)
                m_binCount = static_cast<size_t>(_max) - static_cast<size_t>(_min);
            else if (m_binCount == 0)
                m_binCount = static_cast<size_t>(_max) - static_cast<size_t>(_min) + 1;
            
            float dx = (_max - _min) / (m_binCount - 1);

            while (x <= _max)
            {
                m_bins[x] = 0;
                x += dx;
            }
            
            m_binCount = m_bins.size();
        }

    }

}
