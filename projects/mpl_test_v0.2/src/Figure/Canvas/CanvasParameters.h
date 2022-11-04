
#include "../FigureParameters.h"


namespace Syn
{
    namespace mplc
    {
        struct scatter_params_t
        {
            FigureMarker marker;
            float marker_size;
            size_t x_tick_count;
            size_t y_tick_count;
            
            scatter_params_t(FigureMarker _marker=rcParams.scatter_marker,
                             float _marker_size=rcParams.scatter_marker_sz_px,
                             size_t _x_tick_count=rcParams.x_tick_count,
                             size_t _y_tick_count=rcParams.y_tick_count) :
                marker(_marker),
                marker_size(_marker_size),
                x_tick_count(_x_tick_count),
                y_tick_count(_y_tick_count)
            {}
        };
        //
        struct LinePlotParams
        {

        };
        //
        struct HistParams
        {

        };

    }
}


