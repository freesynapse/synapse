
#include "../FigureParameters.h"


namespace Syn
{
    namespace mplc
    {
        struct scatter_params_t
        {
            FigureMarker marker;
            float marker_size;
            glm::vec4 marker_color;
            size_t x_tick_count;
            size_t y_tick_count;
            
            scatter_params_t(FigureMarker _marker=rcParams.scatter_marker,
                             float _marker_size=rcParams.scatter_marker_sz_px,
                             const glm::vec4& _marker_color=rcParams.stroke_color,
                             size_t _x_tick_count=rcParams.x_tick_count,
                             size_t _y_tick_count=rcParams.y_tick_count) :
                marker(_marker),
                marker_size(_marker_size),
                marker_color(_marker_color),
                x_tick_count(_x_tick_count),
                y_tick_count(_y_tick_count)
            {}
        };
        //
        struct lineplot_params_t
        {
            float line_width_px;
            FigureMarker marker;
            float marker_size;
            glm::vec4 line_color;
            size_t x_tick_count;
            size_t y_tick_count;

            lineplot_params_t(float _line_width_px=rcParams.line_width_px,
                              FigureMarker _marker=FigureMarker::None,
                              float _marker_size=rcParams.scatter_marker_sz_px,
                              const glm::vec4& _line_color=rcParams.stroke_color,
                              size_t _x_tick_count=rcParams.x_tick_count,
                              size_t _y_tick_count=rcParams.y_tick_count) :
                line_width_px(_line_width_px),
                marker(_marker),
                marker_size(_marker_size),
                line_color(_line_color),
                x_tick_count(_x_tick_count),
                y_tick_count(_y_tick_count)
            {}
        };
        //
        struct histogram_params_t
        {

        };
        //
        struct stemplot_params_t
        {

        };

    }
}


