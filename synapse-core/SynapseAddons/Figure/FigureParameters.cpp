
#include "FigureParameters.h"

#include "Canvas/CanvasParameters.h"


namespace Syn
{
    namespace mplc
    {
        // global namespace instance
        figure_params_t rcParams = figure_params_t();

        //-------------------------------------------------------------------------------
        void figure_params_t::setFromScatterParams(const scatter_params_t& _params)
        {
            figure_type = _params.type;
            scatter_marker = _params.marker;
            scatter_marker_sz_px = _params.marker_size;
            x_tick_count = _params.x_tick_count;
            y_tick_count = _params.y_tick_count;
            data_color = _params.marker_color;
        }
        //-------------------------------------------------------------------------------
        void figure_params_t::setFromLinePlotParams(const lineplot_params_t& _params)
        {
            figure_type = _params.type;
            scatter_marker = _params.marker;
            scatter_marker_sz_px = _params.marker_size;
            line_width_px = _params.line_width_px;
            data_color = _params.line_color;
            x_tick_count = _params.x_tick_count;
            y_tick_count = _params.y_tick_count;
            x_nice_scale = _params.x_nice_scale;
        }
        //-------------------------------------------------------------------------------
        void figure_params_t::setFromHistParams(const histogram_params_t& _params)
        {
            figure_type = _params.type;
            bin_count = _params.bin_count;
        }
        //-------------------------------------------------------------------------------
        void figure_params_t::printParameters()
        {
            printf("-------- Syn::mplc::figure_params_t -------- \n");
            printf("%-32s        :        %s\n", "figure_type", figureTypeStr(figure_type));
            printf("%-32s        :        (%.2f, %.2f)\n", "figure_sz_px", figure_sz_px.x, figure_sz_px.y);
            printf("%-32s        :        %.2f\n", "data_height_px", data_height_px);
            printf("%-32s        :        %.2f\n", "data_spacing", data_spacing);
            printf("%-32s        :        (%.2f, %.2f)\n", "data_axis_offset_px", data_axis_offset_px.x, data_axis_offset_px.y);
            printf("%-32s        :        %.2f\n", "z_value_data", z_value_data);
            printf("%-32s        :        %.2f\n", "z_value_aux", z_value_aux);
            printf("%-32s        :        (%.2f, %.2f)\n", "axes_neg_protrusion_px", axes_neg_protrusion_px.x, axes_neg_protrusion_px.y);
            printf("%-32s        :        (%.2f, %.2f)\n", "canvas_origin_px", canvas_origin_px.x, canvas_origin_px.y);
            printf("%-32s        :        (%.2f, %.2f)\n", "x_axis_lim_px", x_axis_lim_px.x, x_axis_lim_px.y);
            printf("%-32s        :        %s\n", "x_nice_scale", x_nice_scale ? "true" : "false");
            printf("%-32s        :        %.2f\n", "x_axis_length", x_axis_length);
            printf("%-32s        :        (%.2f, %.2f)\n", "y_axis_lim_px", y_axis_lim_px.x, y_axis_lim_px.y);
            printf("%-32s        :        %s\n", "y_nice_scale", y_nice_scale ? "true" : "false");
            printf("%-32s        :        %.2f\n", "y_axis_length", y_axis_length);
            printf("%-32s        :        %s\n", "render_x_axis", render_x_axis ? "true" : "false");
            printf("%-32s        :        %s\n", "render_y_axis", render_y_axis ? "true" : "false");
            printf("%-32s        :        (%.2f, %.2f)\n", "tick_length_px", tick_length_px.x, tick_length_px.y);
            printf("%-32s        :        %zu\n", "x_tick_count", x_tick_count);
            printf("%-32s        :        %s\n", "render_x_ticks", render_x_ticks ? "true" : "false");
            printf("%-32s        :        %zu\n", "y_tick_count", y_tick_count);
            printf("%-32s        :        %s\n", "render_y_ticks", render_y_ticks ? "true" : "false");
            printf("%-32s        :        %.2f\n", "axis_label_font_size_px", axis_label_font_size_px);
            printf("%-32s        :        %s\n", "render_axis_labels", render_axis_labels ? "true" : "false");
            printf("%-32s        :        (%.2f, %.2f)\n", "tick_labels_offset_px", tick_labels_offset_px.x, tick_labels_offset_px.y);
            printf("%-32s        :        %.2f\n", "tick_label_font_size_px", tick_label_font_size_px);
            printf("%-32s        :        %s\n", "render_ticklabels", render_ticklabels ? "true" : "false");
            printf("%-32s        :        %d\n", "rotation_x_ticklabels", rotation_x_ticklabels);
            printf("%-32s        :        %d\n", "rotation_y_ticklabels", rotation_y_ticklabels);
            printf("%-32s        :        %s\n", "render_title", render_title ? "true" : "false");
            printf("%-32s        :        %.2f\n", "title_font_size_px", title_font_size_px);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "tick_color", tick_color.r, tick_color.g, tick_color.b, tick_color.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "axis_color", axis_color.r, axis_color.g, axis_color.b, axis_color.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "title_color", title_color.r, title_color.g, title_color.b, title_color.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "axis_label_color", axis_label_color.r, axis_label_color.g, axis_label_color.b, axis_label_color.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "tick_label_color", tick_label_color.r, tick_label_color.g, tick_label_color.b, tick_label_color.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "figure_background", figure_background.r, figure_background.g, figure_background.b, figure_background.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "data_color", data_color.r, data_color.g, data_color.b, data_color.a);
            printf("%-32s        :        (%.2f, %.2f, %.2f, %.2f)\n", "fill_between_color", fill_between_color.r, fill_between_color.g, fill_between_color.b, fill_between_color.a);
            if (figure_type == FigureType::Histogram)
            {
                printf("%-32s        :        %.2f\n", "bar_spacing_px", bar_spacing_px);
                printf("%-32s        :        %d\n", "bin_count", bin_count);
            }
            if (figure_type == FigureType::ScatterPlot)
            {
                printf("%-32s        :        %.2f\n", "scatter_marker_sz_px", scatter_marker_sz_px);
                printf("%-32s        :        %s\n", "scatter_marker", figureMarkerStr(scatter_marker));
            }
            if (figure_type == FigureType::LinePlot)
                printf("%-32s        :        %.2f\n", "line_width_px", line_width_px);
            printf("%-32s        :        %s\n", "fill_between_x", fill_between_x ? "true" : "false");
            printf("%-32s        :        %s\n", "fill_between_y", fill_between_y ? "true" : "false");
        }
    }
}