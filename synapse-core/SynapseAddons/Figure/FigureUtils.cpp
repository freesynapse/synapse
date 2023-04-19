
#include "FigureUtils.h"

#include <Synapse/Core.hpp>
#include "FigureParameters.h"
#include "FigureRenderObj.h"


namespace Syn
{
    namespace mplc
    {
        size_t figureMarkerVertexCount(const FigureMarker& _marker)
        {
            switch (_marker)
            {
            case FigureMarker::Square:      return  6;
            case FigureMarker::Diamond:     return  6;
            case FigureMarker::LTriangle:   return  3;
            case FigureMarker::UTriangle:   return  3;
            case FigureMarker::HLine:       return  6;
            case FigureMarker::VLine:       return  6;
            case FigureMarker::Plus:        return 12;
            case FigureMarker::Dot:         return  0;  // TODO : implement this
            case FigureMarker::None:        return  0;
            default:                        return  0;
            }
            return 0;
        }
        //-------------------------------------------------------------------------------
        size_t figureMarkerVertices(normalized_params_t* _params,
                                    std::vector<glm::vec2>& _vertices)
        {
            _vertices.clear();
            FigureMarker marker = _params->scatter_marker;
            
            if (marker == FigureMarker::None)
                return 0;

            float x_m_sz = _params->scatter_marker_x_sz;
            float y_m_sz = _params->scatter_marker_y_sz;

            static float h = 1.0f;
            static float s = sqrt(1.5 * h);
            glm::vec2 A, B, C;

            size_t vertex_count = figureMarkerVertexCount(marker);
            switch (marker)
            {
            case FigureMarker::Square:
                
                _vertices.push_back({ -0.5f * x_m_sz, -0.5f * y_m_sz });
                _vertices.push_back({  0.5f * x_m_sz, -0.5f * y_m_sz });
                _vertices.push_back({  0.5f * x_m_sz,  0.5f * y_m_sz });
                _vertices.push_back({  0.5f * x_m_sz,  0.5f * y_m_sz });
                _vertices.push_back({ -0.5f * x_m_sz,  0.5f * y_m_sz });
                _vertices.push_back({ -0.5f * x_m_sz, -0.5f * y_m_sz });
                break;
            //
            case FigureMarker::Diamond:
                _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.000f * 1.2f * x_m_sz, -0.707f * 1.2f * y_m_sz });
                _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.000f * 1.2f * x_m_sz,  0.707f * 1.2f * y_m_sz });
                _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                break;
            //
            case FigureMarker::LTriangle:
                //r = 1.0
                //A = np.array([-r/2.0, np.sqrt(1.5*r)-r/2.0])
                //B = np.array([ r/2.0, np.sqrt(1.5*r)-r/2.0])
                //C = np.array([ 0.0, 0.0-r/2.0])
                A = { -h*0.5f,    s-(h*0.5f) };
                B = {  h*0.5f,    s-(h*0.5f) };
                C = {    0.0f, 0.0f-(h*0.5f) };
                _vertices.push_back({ C[0] * 1.5f * x_m_sz, C[1] * 1.5f * y_m_sz });
                _vertices.push_back({ B[0] * 1.5f * x_m_sz, B[1] * 1.5f * y_m_sz });
                _vertices.push_back({ A[0] * 1.5f * x_m_sz, A[1] * 1.5f * y_m_sz });
                break;
            //
            case FigureMarker::UTriangle:
                A = { -h*0.5f, 0.0f-(h*0.5f) };
                B = {  h*0.5f, 0.0f-(h*0.5f) };
                C = {    0.0f,    s-(h*0.5f) };
                _vertices.push_back({ A[0] * 1.5f * x_m_sz, A[1] * 1.5f * y_m_sz });
                _vertices.push_back({ B[0] * 1.5f * x_m_sz, B[1] * 1.5f * y_m_sz });
                _vertices.push_back({ C[0] * 1.5f * x_m_sz, C[1] * 1.5f * y_m_sz });
                break;
            //
            case FigureMarker::HLine:
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                break;
            //
            case FigureMarker::VLine:
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                break;
            //
            case FigureMarker::Plus:
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                break;
            //
            default:
                SYN_CORE_WARNING("Invalid FigureMarker (", (uint32_t)marker, ").");
                return 0;
                break;
            }
            //
            return vertex_count;
        }
        //-------------------------------------------------------------------------------
        // NiceScale MEMBER FUNCTIONS
        //
        void NiceScale::calculate()
        {
            if (nice_scale)
                range = nice_num(lim[1] - lim[0], false);
            else
                range = lim[1] - lim[0];

            tick_spacing = nice_num(range / (static_cast<float>(max_ticks) - 1.0f), true);

            if (nice_scale)
            {
                nice_lim[0] = floor(lim[0] / tick_spacing) * tick_spacing;
                nice_lim[1] = ceil(lim[1] / tick_spacing) * tick_spacing;
            }
            else
            {
                nice_lim[0] = (lim[0] / tick_spacing) * tick_spacing;
                nice_lim[1] = (lim[1] / tick_spacing) * tick_spacing;
            }
            
            // update parameters
            max_ticks = static_cast<size_t>((upper_bound - lower_bound) / tick_spacing);
            if (nice_lim[0] == 0.0f)
                max_ticks++;
            range = nice_lim[1] - nice_lim[0];
            set = true;
        }
        //-------------------------------------------------------------------------------
        float NiceScale::nice_num(float _range, bool _round)
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
        //-------------------------------------------------------------------------------
        // AxesScaler MEMBER FUNCTIONS
        //
        Axes::Axes(figure_params_t* _fig_params)
        {
            m_paramsPtr = _fig_params;
            normalized_params_t params = normalized_params_t(m_paramsPtr);
            
            m_converters[0].plot_lim = { params.x_axis_lim[0], params.x_axis_lim[1] };
            //m_converters[0].plot_lim = { params.canvas_origin.x + params.data_axis_offset.x,
            //                             params.canvas_origin.x + params.x_axis_length - params.data_axis_offset.x };
            m_converters[0].update_plot_range();
            //m_converters[1].plot_lim = { params.canvas_origin.y + params.data_axis_offset.y,
            //                             params.data_height };
            m_converters[1].plot_lim = { params.y_axis_lim[0], params.y_axis_lim[1] };

            m_converters[1].update_plot_range();
        }
        //-------------------------------------------------------------------------------
        void Axes::setXLim(const glm::vec2& _x_lim, bool _x_nice_scale)
        {
            m_scalers[0] = NiceScale(_x_lim, _x_nice_scale);
            m_converters[0].xy_lim = m_scalers[0].nice_lim;
            m_converters[0].update_xy_range();
        }
        //-------------------------------------------------------------------------------
        void Axes::setYLim(const glm::vec2& _y_lim, bool _y_nice_scale)
        {
            m_scalers[1] = NiceScale(_y_lim, _y_nice_scale);
            m_converters[1].xy_lim = m_scalers[1].nice_lim;
            m_converters[1].update_xy_range();
        }
    }
}

