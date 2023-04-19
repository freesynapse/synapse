
#pragma once

#include <math.h>
#include <glm/glm.hpp>


namespace Syn
{
    namespace mplc
    {
        static const int X_AXIS = 0;
        static const int Y_AXIS = 1;
        static const int Z_AXIS = 2;

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
        
        //-------------------------------------------------------------------------------
        /* Calculation of quantiles from an iterable type. Presumes sorted data _x.
         */
        template<typename T>
        typename T::value_type quantile(const T& _x, float _q)
        {
            assert(_q >= 0.0 && _q <= 1.0);
            const auto n  = _x.size();
            const auto id = (n - 1) * _q;
            const auto lo = floor(id);
            const auto hi = ceil(id);
            const auto qs = _x[lo];
            const auto h  = (id - lo);

            return (1.0 - h) * qs + h * _x[hi];
        }

        //-------------------------------------------------------------------------------
        /* Calculation of inter-quartile range (using quantile() from above). 
         * Presumes sorted iterable.
         */
        template<typename T>
        glm::vec2 IQR(const T& _x)
        {
            glm::vec2 q = { quantile(_x, 0.25), quantile(_x, 0.75) };
            return q;
        }

        /*-------------------------------------------------------------------------------
         * VISUALLY PLEASING TICKS
         *-------------------------------------------------------------------------------
         */

        /* Helper struct for rendering tick labels.
         */
        struct tick_labels_t
        {
            size_t label_count = 0;
            std::vector<std::string> labels;
            float max_label_width = 0.0f;
            float min_label_width = 0.0f;
        };

        class NiceScale
        {
        public: // open member variables
            glm::vec2 lim;
            size_t max_ticks = 15;
            float tick_spacing;
            float range;
            bool nice_scale = true;
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
            tick_labels_t tick_labels;
        
        public: // Ctor / Dtor
            NiceScale() {}
            NiceScale(const glm::vec2& _lim, bool _nice_scale)
            {   
                lim = _lim;
                nice_scale = _nice_scale;
                calculate();
            }
            ~NiceScale() = default;

        public: // member functions
            void calculate();
            float nice_num(float _range, bool _round);
            //
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
                printf("------------------------- NiceScale -------------------------\n");
                if (_info != "")
                    printf("    %s\n", _info.c_str());
                printf("    size_t  max_ticks         =    %zu\n",  max_ticks);
                printf("    float   tick_spacing      =    %.2f\n", tick_spacing);
                printf("    float   range             =    %.2f\n", range);
                printf("    float   lower_bound       =    %.2f\n", lower_bound);
                printf("    float   upper_bound       =    %.2f\n", upper_bound);
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
            void __debug_print(const std::string& _info="")
            {
                printf("------------------------- RangeConverter -------------------------\n");
                if (_info != "")
                    printf("%s\n", _info.c_str());
                printf("    vec2  xy_lim              =    %.2f, %.2f\n", xy_lim[0], xy_lim[1]);
                printf("    float xy_range            =    %.2f\n", 1.0f / xy_range_inv);
                printf("    float xy_range_inv        =    %.2f\n", xy_range_inv);
                printf("    vec2  plot_lim            =    %.2f, %.2f\n", plot_lim[0], plot_lim[1]);
                printf("    float plot_range          =    %.2f\n", plot_range);
            }
        };

        /*-------------------------------------------------------------------------------
         * SCALER AND RANGE CONVERTER FOR BOTH AXES
         *-------------------------------------------------------------------------------
         */
        struct figure_params_t; // forward decl
        class Axes
        {
        public:
            Axes(figure_params_t* _fig_params);

        public:
            float eval_x(float _val) { return m_converters[0].eval(_val); }
            float eval_y(float _val) { return m_converters[1].eval(_val); }
            NiceScale& x_ticks() { return m_scalers[0]; }
            NiceScale& y_ticks() { return m_scalers[1]; }

            void setXLim(const glm::vec2& _x_lim, bool _x_nice_scale);
            void setYLim(const glm::vec2& _y_lim, bool _y_nice_scale);
            //void setLim(const glm::vec2& _x_lim, const glm::vec2& _y_lim)
            //{
            //    setXLim(_x_lim);
            //    setYLim(_y_lim);
            //}
            void __debug_print(const std::string& _info="")
            {
                printf("-------------------- AXES OBJECT ---------------------\n");
                if (_info != "")
                    printf("%s\n", _info.c_str());
                m_scalers[0].__debug_print("X_AXIS");
                m_converters[0].__debug_print("X CONVERTER");
                m_scalers[1].__debug_print("Y_AXIS");
                m_converters[1].__debug_print("Y CONVERTER");
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
    
        /* Type deduction functions 
         */
        template<typename T_>
        inline bool isIntegralType(T_ _val) { return std::is_integral<T_>::value; }
        template<typename T_>
        inline bool isFloatingPointType(T_ _val) { return std::is_floating_point<T_>::value; }
    }
}
