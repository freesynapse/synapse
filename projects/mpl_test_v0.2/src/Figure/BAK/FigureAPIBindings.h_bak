
#pragma once

#include "Histogram/Histogram.h"
#include "LinePlot/LinePlot.h"
#include "ScatterPlot/ScatterPlot.h"

namespace Syn
{
    namespace mplc
    {
        // Histogram
        static inline std::shared_ptr<Histogram> newHistogram(const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<Histogram>(_fig_size); }
        static inline std::shared_ptr<Histogram> newHistogram(const std::vector<float>& _data, size_t _nbins=0, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<Histogram>(_data, _nbins, _fig_size); }
        static inline std::shared_ptr<Histogram> newHistogram(const std::vector<int>& _data, size_t _nbins=0, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<Histogram>(_data, _nbins, _fig_size); }

        // LinePlot
        static inline std::shared_ptr<LinePlot> newLinePlot(const glm::vec2& _fig_size)
        { return std::make_shared<LinePlot>(_fig_size); }
        static inline std::shared_ptr<LinePlot> newLinePlot(const std::vector<float>& _data, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<LinePlot>(_data, _fig_size); }
        static inline std::shared_ptr<LinePlot> newLinePlot(const std::vector<int>& _data, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<LinePlot>(_data, _fig_size); }
        static inline std::shared_ptr<LinePlot> newLinePlot(const std::vector<float>& _X, const std::vector<float>& _Y, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<LinePlot>(_X, _Y, _fig_size); }
        static inline std::shared_ptr<LinePlot> newLinePlot(const std::vector<int>& _X, const std::vector<int>& _Y, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<LinePlot>(_X, _Y, _fig_size); }
        static inline std::shared_ptr<LinePlot> newLinePlot(const std::vector<std::vector<float>>& _data, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<LinePlot>(_data, _fig_size); }
        static inline std::shared_ptr<LinePlot> newLinePlot(const std::vector<std::vector<int>>& _data, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<LinePlot>(_data, _fig_size); }

        // ScatterPlot
        static inline std::shared_ptr<ScatterPlot> newScatterPlot(const glm::vec2& _fig_size)
        { return std::make_shared<ScatterPlot>(_fig_size); }
        static inline std::shared_ptr<ScatterPlot> newScatterPlot(const std::vector<float>& _data, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<ScatterPlot>(_data, _fig_size); }
        static inline std::shared_ptr<ScatterPlot> newScatterPlot(const std::vector<int>& _data, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<ScatterPlot>(_data, _fig_size); }
        static inline std::shared_ptr<ScatterPlot> newScatterPlot(const std::vector<float>& _X, const std::vector<float>& _Y, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<ScatterPlot>(_X, _Y, _fig_size); }
        static inline std::shared_ptr<ScatterPlot> newScatterPlot(const std::vector<int>& _X, const std::vector<int>& _Y, const glm::vec2& _fig_size=glm::vec2(0))
        { return std::make_shared<ScatterPlot>(_X, _Y, _fig_size); }
    }
}
