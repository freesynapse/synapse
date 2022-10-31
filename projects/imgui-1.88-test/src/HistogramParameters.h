
#pragma once

#include <Synapse/Memory/MemoryTypes.hpp>

using namespace Syn;


// global defaults
#define DEFAULT_FIG_SIZE    (glm::vec2(600.0f, 450.0f))


//---------------------------------------------------------------------------------------
/* Histogram parameters. Default values are set, but it is encouraged to change these.
 */
struct histogram_params_t
{
    /* Size of the canvas, used for normalization of coordinates. Set during construction.
     */
    glm::vec2 figure_sz_px              = { 0, 0 };
    
    /* Space between bars in pixels.
     */
    float bar_spacing_px                = 1;
    
    /* Height of highest bar in pixel-space; if below 0, treated as negaive offset from 
     * canvas_sz.y. TODO : make this relative to the drawable area instead of the canvas.
     */
    float bar_height_px                 = -50;
                                
    /* Offset from axes in pixels, for X only first and last bar, and for Y for all bars.
     */
    glm::vec2 bar_axis_offset_px        = { 2, 2 };

    /* Negative offset in pixels, the amount of backward 'protrusion' of axes at (0, 0).
     */
    glm::vec2 axes_neg_protrusion_px    = { 0, 0 };

    /* Canvas origin in pixel space.
     */
    glm::vec2 canvas_origin_px          = { 80, 50 };

    /* The actual X axis coordinates (pixels). Values represent
     *  [0] : offset from canvas_origin_px.x, both negative and positive values
     *  [1] : offset from canvas_sz.x for negative values and offset from canvas_origin_px.x for positive values 
     */
    glm::vec2 x_axis_lim_px             = { 0, -50 };
    
    /* X axis length -- automatically calculated after normalization.
     */
    float x_axis_length                 = 0.0f;
    
    /* The actual X axis coordinates (pixels). Values represent
     *  [0] : offset from canvas_origin_px.y
     *  [1] : offset from canvas_sz.y for negative values and offset from canvas_origin_px.y for positive values 
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

    /* Are X ticks in between bars (true), or direcly under the middle of the bars (false).
     */
    bool x_ticks_between_bars           = true;

    /* Render X axis ticks?
     */
    bool render_x_ticks                 = true;

    /* Number of Y ticks.
     */
    size_t y_tick_count                 = 10;

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
    glm::vec4 bar_color                 = { 0.298f, 0.361f, 0.490f,  1.0f };
    glm::vec4 axis_color                = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 tick_label_color          = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 title_color               = {   1.0f,   1.0f,   1.0f,  1.0f };
    glm::vec4 canvas_background         = {   0.0f,   0.0f,   0.0f,  1.0f };

    //
    histogram_params_t(const glm::vec2& _fig_sz_px) : figure_sz_px(_fig_sz_px) {}
};

/* Takes a histogram_params_t object and normalizes all pixel values to normalized
 * coordinates [0..1] for rendering. The shader then converts normalized coordinates to
 * normalized device coordinates (NDC).
 * Fields already in normalized coordinates will simply be copied.
 */
struct normalized_params_t
{
    glm::vec2 figure_sz_px;

    float bar_spacing;
    float bar_height;

    glm::vec2 canvas_origin;
    glm::vec2 x_axis_lim;
    float x_axis_length;
    glm::vec2 y_axis_lim;
    float y_axis_length;
    bool render_x_axis;
    bool render_y_axis;
    
    glm::vec2 bar_axis_offset;
    glm::vec2 axes_neg_protrusion;

    glm::vec2 tick_length;
    bool x_ticks_between_bars;
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

    glm::vec4 bar_color;
    glm::vec4 axis_color;
    glm::vec4 tick_label_color;
    glm::vec4 title_color;
    glm::vec4 canvas_background;

    // Converts pixel-space to normalized space [0.0f ... 1.0f].
    #define X   (figure_sz_px.x)
    #define Y   (figure_sz_px.y)
    #define XY  (figure_sz_px)

    #define ORIGIN_X    (canvas_origin.x)
    #define ORIGIN_Y    (canvas_origin.y)

    inline float px_to_fraction_(float _px, float _ax) { return _px / _ax; }
    inline glm::vec2 px_to_fraction_(const glm::vec2& _v, const ImVec2& _ax) { return glm::vec2(_v.x / _ax.x, _v.y / _ax.y); }

    /* Checks if a pixel-space parameter is below zero. If so, negative offset from 
     * figure size in the corresponding axis.
     */
    inline float px_to_fraction(float _lim, float _ax) { return (_lim < 0.0f ? px_to_fraction_(_ax + _lim, _ax) : px_to_fraction_(_lim, _ax)); }
    inline glm::vec2 px_to_fraction(const glm::vec2& _vlim, float _ax) { return { px_to_fraction(_vlim[0], _ax), px_to_fraction(_vlim[1], _ax) }; }
    inline glm::vec2 px_to_fraction(const glm::vec2& _vlim, const glm::vec2& _ax) { return { px_to_fraction(_vlim[0], _ax.x), px_to_fraction(_vlim[1], _ax.y) }; }

    //
    normalized_params_t() {}    // default constructor
    normalized_params_t(const Ref<histogram_params_t>& _params)
    {
        figure_sz_px            = _params->figure_sz_px;

        bar_spacing             = px_to_fraction(_params->bar_spacing_px, X);
        bar_height              = px_to_fraction(_params->bar_height_px, Y);

        canvas_origin           = px_to_fraction(_params->canvas_origin_px, XY);
        x_axis_lim[0]           = px_to_fraction(_params->canvas_origin_px.x + _params->x_axis_lim_px[0], X);
        x_axis_lim[1]           = px_to_fraction(_params->x_axis_lim_px[1], X);
        x_axis_length           = x_axis_lim[1] - x_axis_lim[0];
        y_axis_lim[0]           = px_to_fraction(_params->canvas_origin_px.y + _params->y_axis_lim_px[0], Y);
        y_axis_lim[1]           = px_to_fraction(_params->y_axis_lim_px[1], Y);
        y_axis_length           = y_axis_lim[1] - y_axis_lim[0];

        render_x_axis           = _params->render_x_axis;
        render_y_axis           = _params->render_y_axis;

        bar_axis_offset         = px_to_fraction(_params->bar_axis_offset_px, XY);
        axes_neg_protrusion     = px_to_fraction(_params->axes_neg_protrusion_px, XY);

        tick_length             = px_to_fraction(_params->tick_length_px, XY);
        x_ticks_between_bars    = _params->x_ticks_between_bars;
        render_x_ticks          = _params->render_x_ticks;
        y_tick_count            = _params->y_tick_count;
        render_y_ticks          = _params->render_y_ticks;

        tick_labels_offset      = px_to_fraction(_params->tick_labels_offset_px, XY);
        ticks_font_size_px      = _params->ticks_font_size_px;
        render_ticklabels       = _params->render_ticklabels;
        rotation_x_ticklabels   = _params->rotation_x_ticklabels;
        rotation_y_ticklabels   = _params->rotation_y_ticklabels;

        render_title            = _params->render_title;
        title_size              = _params->title_size;

        bar_color               = _params->bar_color;
        axis_color              = _params->axis_color;
        tick_label_color        = _params->tick_label_color;
        title_color             = _params->title_color;
        canvas_background       = _params->canvas_background;
    }
};


