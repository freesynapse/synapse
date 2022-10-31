
#pragma once

#include <Synapse/Memory/MemoryTypes.hpp>

#include "HistogramParameters.h"
#include "HistogramRenderObj.hpp"

using namespace Syn;


//template<typename T> class HistogramRenderObj;  // forward decl

/* Histogram class, for showing distributions. Number of bins is specified, and an
 * extra is added for inclusion of the last interval. The last bin will always be empty.
 * A separate render object will be needed.
 */
template<typename T>
class Histogram
{
public:
    friend class HistogramRenderObj<T>;

public:
    // default constructors -- data has to be set manually using data().
    Histogram(const ImVec2& _fig_size=ImVec2(0, 0))
    {
        glm::vec2 fig_size = check_fig_size(glm::vec2(_fig_size.x, _fig_size.y));
        m_renderObjPtr = std::make_shared<HistogramRenderObj<T>>(this, glm::vec2(fig_size.x, fig_size.y) );
    }
    Histogram(const glm::vec2& _fig_size=glm::vec2(0))
    {
        glm::vec2 fig_size = check_fig_size(_fig_size);
        m_renderObjPtr = std::make_shared<HistogramRenderObj<T>>(this, glm::vec2(fig_size.x, fig_size.y) );
        m_renderObjPtr = MakeRef<HistogramRenderObj<T>>(this, _fig_size);
    }
    /* Constructor for setting data and bins on initialization. If _nbins is set to 0, 
     * number of bins will be deduced from the data (wip for floating-point histograms).
     */
    Histogram(const std::vector<T>& _data, size_t _nbins=0, const ImVec2& _fig_size=ImVec2(0, 0))
    { 
        glm::vec2 fig_size = check_fig_size(glm::vec2(_fig_size.x, _fig_size.y));
        m_renderObjPtr = std::make_shared<HistogramRenderObj<T>>(this, glm::vec2(fig_size.x, fig_size.y)); 
        data(_data, _nbins);
    }
    /* Constructor for setting data and bins on initialization. If _nbins is set to 0, 
     * number of bins will be deduced from the data (wip for floating-point histograms).
     */
    Histogram(const std::vector<T>& _data, size_t _nbins=0, const glm::vec2& _fig_size=glm::vec2(0))
    {
        glm::vec2 fig_size = check_fig_size(_fig_size);
        m_renderObjPtr = MakeRef<HistogramRenderObj<T>>(this, _fig_size); 
        data(_data, _nbins);
    }
    
    // rendering functions -- forwarded to the canvas
    void updateInterval() { m_renderObjPtr->updateInterval(); }
    void selectInterval(const T& _lo, const T& _hi) { m_renderObjPtr->selectInterval(_lo, _hi); }
    void render() { m_renderObjPtr->render(); }
    void updateCanvas() { m_renderObjPtr->redraw(); }
    void interactiveMode(bool _b) { m_renderObjPtr->setInteractiveMode(_b); }
    T& intervalLo() { return m_renderObjPtr->intervalLo(); }
    T& intervalHi() { return m_renderObjPtr->intervalHi(); }

    // python-style accessors
    void data(const std::vector<T>& _data, size_t _nbins);

    const auto& data() { return m_data; }
    void title(const std::string& _title) { m_figureTitle = _title; }
    const std::string& title() { return m_figureTitle; }

    const std::map<T, size_t>& bins() { return m_bins; }
    inline size_t& binCount() { return m_binCount; }
    inline const T& minBin() { return m_bins.begin()->first; }
    inline const T& maxBin() 
    {
        T x;
        if (isFloatingPointType(x))
            return std::prev(m_bins.rbegin())->first;
        return m_bins.rbegin()->first;
    }
    inline GLuint getCanvas() { return m_renderObjPtr->m_framebuffer->getColorAttachmentIDn(0); }
    inline ImVec2 getCanvasSz() { return ImVec2(m_renderObjPtr->m_canvasSz.x, m_renderObjPtr->m_canvasSz.y); }
    Ref<histogram_params_t> params() { return m_renderObjPtr->m_histParamsPtr; }
    inline bool toUpdate() { return m_renderObjPtr->m_redrawFlags; }
    // __debug
    void __debug_set_bins(const std::map<T, size_t>& _bins) 
    { 
        m_bins = _bins; 
        m_binCount = _bins.size(); 
    }


private:
    /* Checks figure size at Histogram construction and uses default if needed.
     */
    glm::vec2 check_fig_size(const glm::vec2& _fig_sz)
    {
        if (_fig_sz.x <= 0 || _fig_sz.y <= 0)
            return DEFAULT_FIG_SIZE;
        return _fig_sz;
    }
    template<typename T_>
    inline bool isIntegralType(T_ _val) { return std::is_integral<T_>::value; }
    template<typename T_>
    inline bool isFloatingPointType(T_ _val) { return std::is_floating_point<T_>::value; }
    //template<typename T>
    auto findClosestBin(const T& _val)
    {
        if (isIntegralType(_val))
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
    //
    void setupBins(T _min, T _max);


//
private:
    size_t m_binCount = 0;
    std::map<T, size_t> m_bins;
    std::vector<T> m_data;

    std::string m_figureTitle = "";

    std::shared_ptr<HistogramRenderObj<T>> m_renderObjPtr = nullptr;

};

