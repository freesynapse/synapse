
#pragma once

#include "imgui/imgui_internal.h"

#include "FigureUtils.h"


// global defaults
#define FIGURE_DEFUALT_SIZE (glm::vec2(420.0f, 280.0f))

namespace Syn
{
    namespace mplc
    {
        //-------------------------------------------------------------------------------
        /* ScatterPlot markers
         */
        enum class FigureType
        {
            None        = 0x01,
            Histogram   = 0x02,
            LinePlot    = 0x04,
            ScatterPlot = 0x08,
        };

        //-------------------------------------------------------------------------------
        /* ScatterPlot markers
         */
        enum class ScatterPlotMarker
        {
            Square      = 0x01,
            Diamond     = 0x02,
            LTriangle   = 0x04,
            UTriangle   = 0x08,
            HLine       = 0x10,
            VLine       = 0x20,
            Plus        = 0x40,
        };

        //-------------------------------------------------------------------------------
        /* Histogram parameters. Default values are set, but are encouraged to change.
         */
        struct figure_params_t
        {
            /*---------------------------------------------------------------------------
             * FIGURE BASE PARAMETERS
             *---------------------------------------------------------------------------
             */

            /* Type of plot, used for rendering options. Not set by user but by 
             * derivations of FigureBase and FigureRenderObjBase.
            */
            FigureType figure_type              = FigureType::None;

            /* Size of the canvas, used for normalization of coordinates. Set during 
             * construction.
             */
            glm::vec2 figure_sz_px              = { 0, 0 };
            
            /* Height of highest data point in pixel-space; if below 0, treated as 
             * negaive offset from canvas_sz.y.
             */
            float data_height_px                = -50;
                                        
            /* Space between adjecent data points.
            */
            float data_spacing                  = -1;

            /* Offset from axes in pixels, for X only first and last data point, and 
             * for Y for all data points.
             */
            glm::vec2 data_axis_offset_px       = { 0, 0 };

            /* Negative offset in pixels, the amount of backward 'protrusion' of axes 
             * at (0, 0).
             */
            glm::vec2 axes_neg_protrusion_px    = { 0, 0 };

            /* Canvas origin in pixel space.
            */
            glm::vec2 canvas_origin_px          = { 80, 50 };

            /* The actual X axis coordinates (pixels). Values represent
             *   [0] :  offset from canvas_origin_px.x, both negative and positive values
             *   [1] :  offset from canvas_sz.x for negative values and offset from 
             *          canvas_origin_px.x for positive values 
             */
            glm::vec2 x_axis_lim_px             = { 0, -50 };
            
            /* X axis length -- automatically calculated after normalization.
            */
            float x_axis_length                 = 0.0f;
            
            /* The actual X axis coordinates (pixels). Values represent
             *   [0] :  offset from canvas_origin_px.y
             *   [1] :  offset from canvas_sz.y for negative values and offset from 
             *          canvas_origin_px.y for positive values 
             */
            glm::vec2 y_axis_lim_px             = { 0, -30 };
            
            /* Y axis length -- automatically calculated after normalization.
            */
            float y_axis_length                 = 0.0f;
            
            /* Render X axis?
            */
            bool render_x_axis                  = true;

            /* Render Y axis?
            */
            bool render_y_axis                  = true;

            /* Length of ticks in pixels, specified separately for each axis.
            */
            glm::vec2 tick_length_px            = { 10, 10 };

            /* Number of X ticks.
            */
            size_t x_tick_count                 = 5;

            /* Render X axis ticks?
            */
            bool render_x_ticks                 = true;

            /* Number of Y ticks.
            */
            size_t y_tick_count                 = 5;

            /* Render Y axis ticks?
            */
            bool render_y_ticks                 = true;

            /* Ticklabels offset from the lower tick bound, for each axis, respectively.
            */
            glm::vec2 tick_labels_offset_px     = { 5, 2 };

            /* Font size of ticklabels.
            */
            float ticks_font_size_px            = 12.0f;

            /* Render ticklabels?
            */
            bool render_ticklabels              = true;

            /* Rotation of X axis ticklabels, clock-wise, in degrees.
            */
            int rotation_x_ticklabels           = 0;

            /* Rotation of Y axis ticklabels, clock-wise, in degrees.
            */
            int rotation_y_ticklabels           = 0;

            /* Is there a plot title? 
            */
            bool render_title                   = true;

            /* Font size of plot title
            */
            float title_size                    = 16.0f;

            /* Matches the default ImGui palette.
            */
            glm::vec4 tick_color                = {   1.0f,   1.0f,   1.0f,  1.0f };
            glm::vec4 axis_color                = {   1.0f,   1.0f,   1.0f,  1.0f };
            glm::vec4 tick_label_color          = {   1.0f,   1.0f,   1.0f,  1.0f };
            glm::vec4 title_color               = {   1.0f,   1.0f,   1.0f,  1.0f };
            glm::vec4 figure_background         = {   0.0f,   0.0f,   0.0f,  1.0f };
            glm::vec4 stroke_color              = { 0.298f, 0.361f, 0.490f,  1.0f };


            /*---------------------------------------------------------------------------
            * HISTOGRAM PARAMETERS
            *----------------------------------------------------------------------------
            */

            /* Space between bars in pixels.
            */
            float bar_spacing_px                = 1;
            
            /*---------------------------------------------------------------------------
            * SCATTER PLOT PARAMETERS
            *----------------------------------------------------------------------------
            */

            /* Size of markers, in pixels on the Y axis. Markers X size is calculated
             * from the X/Y aspect ratio upon normalization.
             */
            float scatter_marker_sz_px          = 4.0f;

            /* Marker type, as defined above.
             */
            ScatterPlotMarker scatter_marker    = ScatterPlotMarker::Square;

            
            /*---------------------------------------------------------------------------
            * SELECTION (FILL) PARAMETERS
            *----------------------------------------------------------------------------
            */

            /* Render X selection (i.e. called fillBetweenX).
            */
            bool fill_between_x                 = false;

            /* Render Y selection (i.e. called fillBetweenY).
            */
            bool fill_between_y                 = false;


            //
            figure_params_t(const glm::vec2& _fig_sz_px) : figure_sz_px(_fig_sz_px) {}
        };

        /* Takes a figure_params_t object and normalizes all pixel values to normalized
         * coordinates [0..1] for rendering. The shader then converts normalized 
         * coordinates to normalized device coordinates (NDC). Fields already in 
         * normalized coordinates and non-coordinate parameters will simply be copied.
         */
        struct normalized_params_t
        {
            FigureType figure_type;
            glm::vec2 figure_sz_px;
            
            float data_height;
            float data_spacing;
            glm::vec2 data_axis_offset;

            glm::vec2 canvas_origin;
            glm::vec2 x_axis_lim;
            float x_axis_length;
            glm::vec2 y_axis_lim;
            float y_axis_length;
            bool render_x_axis;
            bool render_y_axis;
            
            glm::vec2 axes_neg_protrusion;

            glm::vec2 tick_length;
            size_t x_tick_count;
            bool render_x_ticks;
            size_t y_tick_count;
            bool render_y_ticks;
            glm::vec2 tick_labels_offset;
            float ticks_font_size_px;
            bool render_ticklabels;
            int rotation_x_ticklabels;
            int rotation_y_ticklabels;
            bool render_title;
            float title_size;

            glm::vec4 axis_color;
            glm::vec4 tick_color;
            glm::vec4 tick_label_color;
            glm::vec4 title_color;
            glm::vec4 figure_background;
            glm::vec4 stroke_color;

            // Histogram parameters
            float bar_spacing;

            // ScatterPlot parameters
            float scatter_marker_sz;
            float scatter_marker_y_sz;
            float scatter_marker_x_sz;
            ScatterPlotMarker scatter_marker;

            // Selection (fill) parameters
            bool fill_between_x;
            bool fill_between_y;

            // Converts pixel-space to normalized space [0.0f ... 1.0f].
            #define AXIS_X      (figure_sz_px.x)
            #define AXIS_Y      (figure_sz_px.y)
            #define AXIS_XY     (figure_sz_px)

            #define ORIGIN_X    (canvas_origin.x)
            #define ORIGIN_Y    (canvas_origin.y)

            inline float px_to_fraction_(float _px, float _ax) { return _px / _ax; }
            inline glm::vec2 px_to_fraction_(const glm::vec2& _v, const ImVec2& _ax) { return glm::vec2(_v.x / _ax.x, _v.y / _ax.y); }

            /* Checks if a pixel-space parameter is below zero. If so, negative offset 
             * from figure size in the corresponding axis.
             */
            inline float px_to_fraction(float _lim, float _ax) { return (_lim < 0.0f ? px_to_fraction_(_ax + _lim, _ax) : px_to_fraction_(_lim, _ax)); }
            inline glm::vec2 px_to_fraction(const glm::vec2& _vlim, float _ax) { return { px_to_fraction(_vlim[0], _ax), px_to_fraction(_vlim[1], _ax) }; }
            inline glm::vec2 px_to_fraction(const glm::vec2& _vlim, const glm::vec2& _ax) { return { px_to_fraction(_vlim[0], _ax.x), px_to_fraction(_vlim[1], _ax.y) }; }

            //
            normalized_params_t() {}    // default constructor
            normalized_params_t(const std::shared_ptr<figure_params_t>& _params)
            {
                figure_type             = _params->figure_type;
                figure_sz_px            = _params->figure_sz_px;

                data_height             = px_to_fraction(_params->data_height_px, AXIS_Y);
                data_spacing            = _params->data_spacing;
                data_axis_offset        = px_to_fraction(_params->data_axis_offset_px, AXIS_XY);

                canvas_origin           = px_to_fraction(_params->canvas_origin_px, AXIS_XY);
                x_axis_lim[0]           = px_to_fraction(_params->canvas_origin_px.x + _params->x_axis_lim_px[0], AXIS_X);
                x_axis_lim[1]           = px_to_fraction(_params->x_axis_lim_px[1], AXIS_X);
                x_axis_length           = x_axis_lim[1] - x_axis_lim[0];
                y_axis_lim[0]           = px_to_fraction(_params->canvas_origin_px.y + _params->y_axis_lim_px[0], AXIS_Y);
                y_axis_lim[1]           = px_to_fraction(_params->y_axis_lim_px[1], AXIS_Y);
                y_axis_length           = y_axis_lim[1] - y_axis_lim[0];

                render_x_axis           = _params->render_x_axis;
                render_y_axis           = _params->render_y_axis;

                axes_neg_protrusion     = px_to_fraction(_params->axes_neg_protrusion_px, AXIS_XY);

                tick_length             = px_to_fraction(_params->tick_length_px, AXIS_XY);
                x_tick_count            = _params->x_tick_count;
                render_x_ticks          = _params->render_x_ticks;
                y_tick_count            = _params->y_tick_count;
                render_y_ticks          = _params->render_y_ticks;

                tick_labels_offset      = px_to_fraction(_params->tick_labels_offset_px, AXIS_XY);
                ticks_font_size_px      = _params->ticks_font_size_px;
                render_ticklabels       = _params->render_ticklabels;
                rotation_x_ticklabels   = _params->rotation_x_ticklabels;
                rotation_y_ticklabels   = _params->rotation_y_ticklabels;

                render_title            = _params->render_title;
                title_size              = _params->title_size;

                axis_color              = _params->axis_color;
                tick_color              = _params->tick_color;
                tick_label_color        = _params->tick_label_color;
                title_color             = _params->title_color;
                figure_background       = _params->figure_background;
                stroke_color            = _params->stroke_color;

                // Histogram parameters
                bar_spacing             = px_to_fraction(_params->bar_spacing_px, AXIS_X);

                // ScatterPlot parameters
                scatter_marker_sz       = px_to_fraction(_params->scatter_marker_sz_px, AXIS_Y);
                scatter_marker_y_sz     = scatter_marker_sz;
                scatter_marker_x_sz     = scatter_marker_sz * (figure_sz_px.y / figure_sz_px.x);
                scatter_marker          = _params->scatter_marker;

                // Selection (fill) parameters
                fill_between_x          = _params->fill_between_x;
                fill_between_y          = _params->fill_between_y;

            }
        };

    }
}
