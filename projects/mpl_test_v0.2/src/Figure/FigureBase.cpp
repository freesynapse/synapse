
#include "FigureBase.h"

#include "Canvas/Canvas2D.h"
#include "Canvas/ScatterPlot2D.h"


namespace Syn
{
    namespace mplc
    {
        Figure::Figure(const glm::vec2& _fig_sz_px)
        {
            glm::vec2 fig_sz = check_fig_size(_fig_sz_px);
            m_figureSizePx = fig_sz;
            
            m_figureParams = MakeRef<figure_params_t>(fig_sz);
            
            m_renderObjPtr = MakeRef<FigureRenderObj>(this);
        }
        //-------------------------------------------------------------------------------
        Figure::~Figure()
        {
            for (auto* canvas : m_canvases)
                delete canvas;
        }
        //-------------------------------------------------------------------------------
        void Figure::scatter(const std::vector<float>& _data)
        {
            ScatterPlot2D* scatter_plot = new ScatterPlot2D(_data);
            m_canvases.push_back(scatter_plot);
        }
        //-------------------------------------------------------------------------------
        //void Figure::scatter(const std::vector<int>& _data)
        //{
        //    std::vector<float> fdata = std::vector<float>(_data.begin(), _data.end());
        //    ScatterPlot2D* scatter_plot = new ScatterPlot2D(fdata);
        //    m_canvases.push_back(scatter_plot);
        //    update_data_limits(scatter_plot);
        //}        
        //-------------------------------------------------------------------------------
        void Figure::render()
        {
            m_renderObjPtr->render();
        }
        //-------------------------------------------------------------------------------
        void Figure::redraw()
        {
            // redraw all canvases and find limits
            for (auto* canvas : m_canvases)
            {
                canvas->redraw();
                updata_data_limits(canvas);
            }
            m_renderObjPtr->redraw();
        }
        //-------------------------------------------------------------------------------
        // PRIVATE MEMBER FUNCTIONS
        //        
        glm::vec2 Figure::check_fig_size(const glm::vec2& _fig_sz)
        {
            if (_fig_sz.x <= 0 || _fig_sz.y <= 0)
                return FIGURE_DEFUALT_SIZE; // in FigureParameters.h
            return _fig_sz;
        }
        //-------------------------------------------------------------------------------
        void Figure::updata_data_limits(Canvas2D* _canvas)
        {
            m_dataLimX[0] = std::min(m_dataLimX[0], _canvas->dataLimX()[0]);
            m_dataLimX[1] = std::max(m_dataLimX[1], _canvas->dataLimX()[1]);
            m_dataLimY[0] = std::min(m_dataLimY[0], _canvas->dataLimY()[0]);
            m_dataLimY[1] = std::max(m_dataLimY[1], _canvas->dataLimY()[1]);
        }

    }
}
