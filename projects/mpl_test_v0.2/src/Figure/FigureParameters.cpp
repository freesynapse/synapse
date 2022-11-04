
#include "FigureParameters.h"

#include "Canvas/CanvasParameters.h"


namespace Syn
{
    namespace mplc
    {
        void figure_params_t::setFromScatterParams(const scatter_params_t& _params)
        {
            scatter_marker = _params.marker;
            scatter_marker_sz_px = _params.marker_size;
            x_tick_count = _params.x_tick_count;
            y_tick_count = _params.y_tick_count;
        }
    }
}