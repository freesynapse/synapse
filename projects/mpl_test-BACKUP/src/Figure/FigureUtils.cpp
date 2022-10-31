
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
                //_vertices.push_back({ -0.5f, -0.5f });
                //_vertices.push_back({  0.5f, -0.5f });
                //_vertices.push_back({  0.5f,  0.5f });
                //_vertices.push_back({  0.5f,  0.5f });
                //_vertices.push_back({ -0.5f,  0.5f });
                //_vertices.push_back({ -0.5f, -0.5f });

                // rotate
                /*
                static float theta = M_PI * 0.25f;
                static glm::mat2x2 rot_mat = 
                {
                    cos(theta), -sin(theta),
                    sin(theta),  cos(theta)
                };
                for (int i = 0; i < vertex_count; i++)
                {
                    glm::vec2 pos = _vertices[i];
                    printf("( %.3f, %.3f ) -> ", pos.x, pos.y);
                    _vertices[i] = rot_mat * pos;
                    printf("( %.3f, %.3f )\n", _vertices[i].x, _vertices[i].y);
                    _vertices[i].x = _vertices[i].x * 8 * x_m_sz;
                    _vertices[i].y = _vertices[i].y * 8 * y_m_sz;
                }
                */
                _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.000f * 1.2f * x_m_sz, -0.707f * 1.2f * y_m_sz });
                _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                _vertices.push_back({  0.000f * 1.2f * x_m_sz,  0.707f * 1.2f * y_m_sz });
                _vertices.push_back({ -0.707f * 1.2f * x_m_sz,  0.000f * 1.2f * y_m_sz });
                break;
            //
            case ScatterPlotMarker::LTriangle:
                _vertices.push_back({ -0.5f, -0.5f });
                _vertices.push_back({  0.5f, -0.5f });
                _vertices.push_back({  0.5f,  0.5f });
                break;
            //
            case ScatterPlotMarker::UTriangle:
                _vertices.push_back({ -0.5f, -0.5f });
                _vertices.push_back({  0.5f, -0.5f });
                _vertices.push_back({  0.5f,  0.5f });
                break;
            //
            case ScatterPlotMarker::HLine:
                _vertices.push_back({ -0.5f, -0.1f });
                _vertices.push_back({  0.5f, -0.1f });
                _vertices.push_back({  0.5f,  0.1f });
                _vertices.push_back({  0.5f,  0.1f });
                _vertices.push_back({ -0.5f,  0.1f });
                _vertices.push_back({ -0.5f, -0.1f });
                break;
            //
            case ScatterPlotMarker::VLine:
                _vertices.push_back({ -0.1f, -0.5f });
                _vertices.push_back({  0.1f, -0.5f });
                _vertices.push_back({  0.1f,  0.5f });
                _vertices.push_back({  0.1f,  0.5f });
                _vertices.push_back({ -0.1f,  0.5f });
                _vertices.push_back({ -0.1f, -0.5f });
                break;
            //
            case ScatterPlotMarker::Plus:
                _vertices.push_back({ -0.5f, -0.1f });
                _vertices.push_back({  0.5f, -0.1f });
                _vertices.push_back({  0.5f,  0.1f });
                _vertices.push_back({  0.5f,  0.1f });
                _vertices.push_back({ -0.5f,  0.1f });
                _vertices.push_back({ -0.5f, -0.1f });
                _vertices.push_back({ -0.1f, -0.5f });
                _vertices.push_back({  0.1f, -0.5f });
                _vertices.push_back({  0.1f,  0.5f });
                _vertices.push_back({  0.1f,  0.5f });
                _vertices.push_back({ -0.1f,  0.5f });
                _vertices.push_back({ -0.1f, -0.5f });
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

