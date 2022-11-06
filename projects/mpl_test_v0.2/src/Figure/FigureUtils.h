
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
        enum class FigureMarker;   // forward decl
        extern inline size_t figureMarkerVertexCount(const FigureMarker& _marker);

        //-------------------------------------------------------------------------------
        /* Given a figureMarker size, calculates and returns the vertices for a single 
         * marker. The caller can use this with an added offset to position the markers
         * in the plot.
         */
        struct normalized_params_t; // forward decl
        extern size_t figureMarkerVertices(normalized_params_t* _params, 
                                           std::vector<glm::vec2>& _vertices);
        
        /*-------------------------------------------------------------------------------
         * VISUALLY PLEASING TICKS
         *-------------------------------------------------------------------------------
         */
        class NiceScale
        {
        public: // open member variables
            glm::vec2 lim;
            size_t max_ticks = 10;
            float tick_spacing;
            float range;
            union
            {
                glm::vec2 nice_lim;
                struct
                {
                    float lower_bound;
                    float upper_bound;
                };
            };
            bool set = false;
        
        public: // Ctor / Dtor
            NiceScale() {}
            NiceScale(const glm::vec2& _lim)
            {   
                lim = _lim;
                calculate();
            }
            ~NiceScale() = default;

        public: // member variables
            void calculate()
            {
                range = nice_num(lim[1] - lim[0], false);
                tick_spacing = nice_num(range / (static_cast<float>(max_ticks) - 1.0f), true);
                nice_lim[0] = floor(lim[0] / tick_spacing) * tick_spacing;
                nice_lim[1] = ceil(lim[1] / tick_spacing) * tick_spacing;
                set = true;
            }
            float nice_num(float _range, bool _round)
            {
                float exponent;
                float fraction;
                float nice_fraction;

                exponent = floor(log10(_range));
                fraction = _range / pow(10.f, exponent);

                if (_round) 
                {   if (fraction < 1.5)
                        nice_fraction = 1;
                    else if (fraction < 3)
                        nice_fraction = 2;
                    else if (fraction < 7)
                        nice_fraction = 5;
                    else
                        nice_fraction = 10;
                } 
                else 
                {   if (fraction <= 1)
                        nice_fraction = 1;
                    else if (fraction <= 2)
                        nice_fraction = 2;
                    else if (fraction <= 5)
                        nice_fraction = 5;
                    else
                        nice_fraction = 10;
                }
                return nice_fraction * pow(10, exponent);                
            }
            void set_lim(const glm::vec2& _lim)
            {
                lim = _lim;
                calculate();                
            }
            void set_lim(float _lo, float _hi)
            {
                lim = { _lo, _hi };
                calculate();
            }
            void set_max_ticks(size_t _max_ticks)
            {
                max_ticks = static_cast<size_t>(_max_ticks);
                calculate();
            }
            void __debug_print(const std::string& _info="")
            {
                printf("-------------------- nice_scale ---------------------\n");
                if (_info != "")
                    printf("%s\n", _info.c_str());
                printf("size_t  max_ticks         =   %zu\n",  max_ticks);
                printf("float   tick_spacing      =   %.2f\n", tick_spacing);
                printf("float   range             =   %.2f\n", range);
                printf("float   lower_bound       =   %.2f\n", lower_bound);
                printf("float   upper_bound       =   %.2f\n", upper_bound);
            }
        };
        
        /*-------------------------------------------------------------------------------
         * RANGE CONVERTER
         *-------------------------------------------------------------------------------
         */
        /* Converts values from one space into another, used primarily for conversions
         * of X/Y values into normalized, canvas-offset, plot coordinates.
         * Example:
         *  float y_plot_min = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
         *  float y_plot_max = _fig_params->data_height;
         *  range_converter converter(dataLimY, { y_plot_min, y_plot_max } );
         *  y_pos_plot = converter.eval(y_value);
         */
        class RangeConverter
        {
        public: // open member variables
            float xy_range_inv  = -1.0f;
            float plot_range    = -1.0f;
            glm::vec2 xy_lim    = { -1.0f, -1.0f };
            glm::vec2 plot_lim  = { -1.0f, -1.0f };

        public: // Ctor / Dtor
            RangeConverter() {}
            RangeConverter(const glm::vec2& _xy_lim, const glm::vec2& _plot_lim) :
                xy_lim(_xy_lim), 
                xy_range_inv(1.0f / (_xy_lim[1] - _xy_lim[0])), 
                plot_lim(_plot_lim),
                plot_range(_plot_lim[1] - _plot_lim[0]) {}
            ~RangeConverter() = default;

        public: // member functions
            float eval(float _val)
            {
                //SYN_CORE_ASSERT(xy_lim != glm::vec2(-1.0f) && plot_lim != glm::vec2(-1.0f) &&
                //                xy_range_inv != -1.0f && plot_range != -1.0f,
                //                "RangeConverter not set.");
                return ((_val - xy_lim[0]) * plot_range * xy_range_inv) + plot_lim[0];
            }
            void update_xy_range()      { xy_range_inv = 1.0f / (xy_lim[1] - xy_lim[0]); }
            void update_plot_range()    { plot_range = plot_lim[1] - plot_lim[0]; }
        };

        /*-------------------------------------------------------------------------------
         * SCALER AND RANGE CONVERTER FOR BOTH AXES
         *-------------------------------------------------------------------------------
         */
        struct figure_params_t; // forward decl
        class AxesScaler
        {
        public:
            AxesScaler(figure_params_t* _fig_params);

        public:
            float eval_x(float _val) { return m_converters[0].eval(_val); }
            float eval_y(float _val) { return m_converters[1].eval(_val); }
            NiceScale& x_ticks() { return m_scalers[0]; }
            NiceScale& y_ticks() { return m_scalers[1]; }

            void setXLim(const glm::vec2& _x_lim)
            {
                m_converters[0].xy_lim = _x_lim;
                m_converters[0].update_xy_range();
                m_scalers[0] = NiceScale(_x_lim);
            }
            void setYLim(const glm::vec2& _y_lim)
            {
                m_converters[1].xy_lim = _y_lim;
                m_converters[1].update_xy_range();
                m_scalers[1] = NiceScale(_y_lim);
            }
            void setLim(const glm::vec2& _x_lim, const glm::vec2& _y_lim)
            {
                setXLim(_x_lim);
                setYLim(_y_lim);
            }
            
        private:
            figure_params_t* m_paramsPtr = nullptr;
        public:
            NiceScale m_scalers[2];         // X = 0, Y = 1
            RangeConverter m_converters[2];

        };


        /*-------------------------------------------------------------------------------
         * SMALL UTILITY FUNCTIONS
         *-------------------------------------------------------------------------------
         */

        /* Type deduction function. */
        template<typename T_>
        inline bool isIntegralType(T_ _val) { return std::is_integral<T_>::value; }
        /* Type deduction function. */
        template<typename T_>
        inline bool isFloatingPointType(T_ _val) { return std::is_floating_point<T_>::value; }


    }
}

/*
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
struct nice_scale_01_t
{
    uint32_t    max_ticks;
    float       tick_spacing;
    float       range;
    glm::vec2   data_lim;
    float       lower_bound;
    float       upper_bound;

    nice_scale_01_t(const glm::vec2& _data_lim, size_t _max_ticks=5) :
        lower_bound(_data_lim[0]), upper_bound(_data_lim[1]), max_ticks(_max_ticks)
    {
        calculate();
    }

    void __debug_print(const std::string& _info="")
    {
        printf("-------------------- nice_scale_01_t ---------------------\n");
        if (_info != "")
            printf("%s\n", _info.c_str());
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
*/

