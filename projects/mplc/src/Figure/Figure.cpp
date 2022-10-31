
#include "Figure.h"
#include "ScatterPlot.h"

namespace Syn
{
    namespace mplc
    {
        void Figure::scatter()
        {
            ScatterPlot* scatter_plot = new ScatterPlot(this);
            m_canvases.push_back(scatter_plot);
        }
    }
}