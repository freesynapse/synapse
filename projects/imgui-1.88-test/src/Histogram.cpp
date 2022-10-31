
#include "HistogramTemplates.hpp"
#include "HistogramRenderObj.hpp"
#include "Histogram.hpp"


template<typename T>
void Histogram<T>::data(const std::vector<T>& _data, size_t _nbins)
{
    // copy of the data
    m_data = std::vector<T>(_data);

    //std::pair<T, T> lim = std::minmax(_data.begin(), _data.end());
    T max = std::numeric_limits<T>::min();
    T min = std::numeric_limits<T>::max();
    for (const auto& val : _data)
    {
        min = std::min(min, val);
        max = std::max(max, val);
    }
    std::pair<T, T> lim = { min, max };
    
    m_binCount = _nbins;
    setupBins(lim.first, lim.second);
    
    for (auto& val : _data)
    {
        if (val < m_bins.begin()->first || val > m_bins.rbegin()->first)
            continue;

        std::prev(m_bins.upper_bound(val))->second++;
    }

    // redraw geometry
    m_renderObjPtr->m_currInterval = { minBin(), maxBin() };
    m_renderObjPtr->m_prevInterval = { minBin(), maxBin() };
    m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW_DATA | FIGURE_REDRAW_AXES | FIGURE_REDRAW_LABELS;
    if (m_renderObjPtr->m_interactiveMode)
        m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_SELECTION;
}
//---------------------------------------------------------------------------------------
template<typename T>
void Histogram<T>::setupBins(T _min, T _max)
{
    m_bins.clear();

    T x = _min;
    // integer types are special cases
    if ((_max - _min) < m_binCount && isIntegralType(x))
        m_binCount = static_cast<size_t>(_max) - static_cast<size_t>(_min);
    else if (m_binCount == 0)
    {
        //m_binCount = static_cast<size_t>(_max) - static_cast<size_t>(_min);
        if (isIntegralType(x))
            m_binCount = static_cast<size_t>(_max) - static_cast<size_t>(_min);
        else
            m_binCount = static_cast<size_t>(_max) - static_cast<size_t>(_min) + 1;
    }
    
    T dx = (_max - _min) / (m_binCount - 1);

    while (x <= _max)
    {
        m_bins[x] = 0;
        x += dx;
    }
    
    //if (isIntegralType(x))
    //    m_bins[x] = 0;
    
    m_binCount = m_bins.size();
}

