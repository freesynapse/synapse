
#include "FigureUtils.h"

#include <Synapse/Core.hpp>
#include "FigureParameters.h"


namespace Syn
{
    namespace mplc
    {
        size_t scatterPlotMarkerVertexCount(const ScatterPlotMarker& _marker)
        {
            switch (_marker)
            {
            case ScatterPlotMarker::Square:     return  6;
            case ScatterPlotMarker::Diamond:    return  6;
            case ScatterPlotMarker::LTriangle:  return  3;
            case ScatterPlotMarker::UTriangle:  return  3;
            case ScatterPlotMarker::HLine:      return  6;
            case ScatterPlotMarker::VLine:      return  6;
            case ScatterPlotMarker::Plus:       return 12;
            default:                            return  0;
            }
            return 0;
        }
        //-------------------------------------------------------------------------------
        size_t scatterPlotMarkerVertices(normalized_params_t* _params,
                                         std::vector<glm::vec2>& _vertices)
        {
            _vertices.clear();
            ScatterPlotMarker marker = _params->scatter_marker;
            float x_m_sz = _params->scatter_marker_x_sz;
            float y_m_sz = _params->scatter_marker_y_sz;

            static float h = 1.0f;
            static float s = sqrt(1.5 * h);
            glm::vec2 A, B, C;

            size_t vertex_count = scatterPlotMarkerVertexCount(marker);
            switch (marker)
            {
            case ScatterPlotMarker::Square:
                
                _vertices.push_back({ -0.5f * x_m_sz, -0.5f * y_m_sz });
                _vertices.push_back({  0.5f * x_m_sz, -0.5f * y_m_sz });
                _vertices.push_back({  0.5f * x_m_sz,  0.5f * y_m_sz });
                _vertices.push_back({  0.5f * x_m_sz,  0.5f * y_m_sz });
                _vertices.push_back({ -0.5f * x_m_sz,  0.5f * y_m_sz });
                _vertices.push_back({ -0.5f * x_m_sz, -0.5f * y_m_sz });
                break;
            //
            case ScatterPlotMarker::Diamond:
                _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.000f * 1.2f * x_m_sz, -0.707f * 1.2f * y_m_sz });
                _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.000f * 1.2f * x_m_sz,  0.707f * 1.2f * y_m_sz });
                _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                break;
            //
            case ScatterPlotMarker::LTriangle:
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
            case ScatterPlotMarker::UTriangle:
                A = { -h*0.5f, 0.0f-(h*0.5f) };
                B = {  h*0.5f, 0.0f-(h*0.5f) };
                C = {    0.0f,    s-(h*0.5f) };
                _vertices.push_back({ A[0] * 1.5f * x_m_sz, A[1] * 1.5f * y_m_sz });
                _vertices.push_back({ B[0] * 1.5f * x_m_sz, B[1] * 1.5f * y_m_sz });
                _vertices.push_back({ C[0] * 1.5f * x_m_sz, C[1] * 1.5f * y_m_sz });
                break;
            //
            case ScatterPlotMarker::HLine:
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({  0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz,  0.1f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.5f * 2.0f * x_m_sz, -0.1f * 2.0f * y_m_sz });
                break;
            //
            case ScatterPlotMarker::VLine:
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({  0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz,  0.5f * 2.0f * y_m_sz });
                _vertices.push_back({ -0.1f * 2.0f * x_m_sz, -0.5f * 2.0f * y_m_sz });
                break;
            //
            case ScatterPlotMarker::Plus:
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
                SYN_CORE_WARNING("Invalid ScatterPlotMarker (", (uint32_t)marker, ").");
                return 0;
                break;
            }
            //
            return vertex_count;
        }
    }
}

