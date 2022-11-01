
#pragma once

#include <math.h>
#include <glm/glm.hpp>


namespace Syn
{
    namespace mplc
    {
        //-------------------------------------------------------------------------------
        /* Converts a ScatterPlotMarker into number of vertices per marker
         */
        enum class ScatterPlotMarker;   // forward decl
        extern inline size_t scatterPlotMarkerVertexCount(const ScatterPlotMarker& _marker);

        //-------------------------------------------------------------------------------
        /* Given a ScatterPlotMarker size, calculates and returns the vertices for a 
         * single marker. The caller can use this with an added offset to position the 
         * markers in the plot.
         */
        struct normalized_params_t; // forward decl
        extern size_t scatterPlotMarkerVertices(normalized_params_t* _params, 
                                                std::vector<glm::vec2>& _vertices);
        
        // Source: https://stackoverflow.com/questions/326679/choosing-an-attractive-linear-scale-for-a-graphs-y-axis
        // [...]
        // double range = ...;
        // int tickCount = ...;
        // double unroundedTickSize = range/(tickCount-1);
        // double x = Math.ceil(Math.log10(unroundedTickSize)-1);
        // double pow10x = Math.pow(10, x);
        // double roundedTickRange = Math.ceil(unroundedTickSize / pow10x) * pow10x;
        // return roundedTickRange;
        // [...]
        //
        struct nice_scale
        {
            uint32_t    max_ticks;
            float       tick_spacing;
            float       range;
            glm::vec2   data_lim;
            float       lower_bound;
            float       upper_bound;

            nice_scale(const glm::vec2& _data_lim, size_t _max_ticks=5) :
                lower_bound(_data_lim[0]), upper_bound(_data_lim[1]), max_ticks(_max_ticks)
            {
                calculate();
            }

            void __debug_print()
            {
                printf("-------------------- nice_scale ---------------------\n");
                printf("uint32_t max_ticks         =   %d\n", max_ticks);
                printf("float    tick_spacing      =   %.2f\n", tick_spacing);
                printf("float    range             =   %.2f\n", range);
                printf("float    lower_bound       =   %.2f\n", lower_bound);
                printf("float    upper_bound       =   %.2f\n", upper_bound);
            }

            void calculate()
            {
                range = upper_bound - lower_bound;
                float unrounded_tick_sz = range / ((float)max_ticks - 1.0f);
                float x = ceil(log10(unrounded_tick_sz) - 1.0f);
                float pow10x = pow(10, x);
                float rounded_tick_range = ceil(unrounded_tick_sz / pow10x) * pow10x;
                tick_spacing = rounded_tick_range;

                lower_bound = rounded_tick_range * round(lower_bound / rounded_tick_range);
                upper_bound = rounded_tick_range * round(1.0f + upper_bound / rounded_tick_range);
            }
                        
        };
        
        /* Converts values from one space into another, used primarily for conversions
         * of X/Y values into normalized, canvas-offset, plot coordinates.
         * Example:
         *  float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
         *  float y_plot_max = _fig_params->data_height;
         *  range_converter converter(dataLimY, { y_plot_min, y_plot_max } );
         *  y_pos_plot = converter.eval(y_value);
         */
        struct range_converter
        {
            float xy_range_inv  = 0.0f;
            float plot_range    = 0.0f;
            glm::vec2 xy_lim    = { 0.0f, 0.0f };
            glm::vec2 plot_lim  = { 0.0f, 0.0f };

            range_converter() {}
            range_converter(const glm::vec2& _xy_lim, const glm::vec2& _plot_lim) :
                xy_lim(_xy_lim), 
                xy_range_inv(1.0f / (_xy_lim[1] - _xy_lim[0])), 
                plot_lim(_plot_lim),
                plot_range(_plot_lim[1] - _plot_lim[0])
            {}

            float eval(float _val) { return ((_val - xy_lim[0]) * plot_range * xy_range_inv) + plot_lim[0]; }
        };

        /* Type deduction function. */
        template<typename T_>
        inline bool isIntegralType(T_ _val) { return std::is_integral<T_>::value; }
        /* Type deduction function. */
        template<typename T_>
        inline bool isFloatingPointType(T_ _val) { return std::is_floating_point<T_>::value; }

    }
}


/* Source : https://gist.github.com/igodorogea/4f42a95ea31414c3a755a8b202676dfd
    */
/*
struct nice_scale
{
    uint32_t max_ticks;
    float    tick_spacing;
    float    range;
    float    lower_bound;
    float    upper_bound;

    //
    nice_scale(float _lower_bound, float _upper_bound, size_t _max_ticks=4) :
        lower_bound(_lower_bound), upper_bound(_upper_bound)
    {
        max_ticks = _max_ticks | 10;
        calculate();
    }
    //
    void set_bounds(float _lower_bound, float _upper_bound) 
    { 
        lower_bound = _lower_bound;
        upper_bound = _upper_bound;
        calculate();
    }
    //
    void calculate()
    {
        range = nice_num(upper_bound - lower_bound, false);
        tick_spacing = nice_num(range / static_cast<float>(max_ticks - 1), true);
        lower_bound = floor(lower_bound / tick_spacing) * tick_spacing;
        upper_bound = ceil(upper_bound / tick_spacing) * tick_spacing;
    }
    //
    float nice_num(float _range, bool _round)
    {
        float exponent = floor(log10(_range));
        float fraction  = _range / pow(10.0f, exponent);
        float nice_fraction;

        if (_round)
        {
            if      (fraction <  1.5f)  nice_fraction =  1;
            else if (fraction <  3.0f)  nice_fraction =  2;
            else if (fraction <  7.0f)  nice_fraction =  5;
            else                        nice_fraction = 10;
        }
        else
        {
            if      (fraction <= 1.0f)  nice_fraction =  1;
            else if (fraction <= 2.0f)  nice_fraction =  2;
            else if (fraction <= 5.0f)  nice_fraction =  5;
            else                        nice_fraction = 10;
        }

        return nice_fraction * pow(10.0f, exponent);
    }

    void print()
    {
        printf("-------------------- nice_scale ---------------------\n");
        printf("uint32_t max_ticks      =   %d\n", max_ticks);
        printf("float    tick_spacing   =   %.2f\n", tick_spacing);
        printf("float    range          =   %.2f\n", range);
        printf("float    lower_bound    =   %.2f\n", lower_bound);
        printf("float    upper_bound    =   %.2f\n", upper_bound);
    }

};
*/

