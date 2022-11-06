
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
            stroke_color = _params.marker_color;
        }
        //-------------------------------------------------------------------------------
        void figure_params_t::setFromLinePlotParams(const lineplot_params_t& _params)
        {
            scatter_marker = _params.marker;
            scatter_marker_sz_px = _params.marker_size;
            line_width_px = _params.line_width_px;
            stroke_color = _params.line_color;
            x_tick_count = _params.x_tick_count;
            y_tick_count = _params.y_tick_count;
        }
    }
}