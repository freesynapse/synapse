
#include "FigureBase.h"
#include "Canvas/Canvas2D.h"

namespace Syn
{
    namespace mplc
    {
        Figure::Figure(const glm::vec2& _fig_sz_px)
        {
            glm::vec2 fig_sz = check_fig_size(_fig_sz_px);
            m_figureSizePx = fig_sz;
            
            m_figureParamsPtr = MakeRef<figure_params_t>(fig_sz);
            m_axesScalerPtr = MakeRef<AxesScaler>(m_figureParamsPtr.get());
            
            m_renderObjPtr = MakeRef<FigureRenderObj>(this);
        }
        //-------------------------------------------------------------------------------
        Figure::~Figure()
        {
            for (auto* canvas : m_canvases)
                delete canvas;
        }
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
                update_data_limits(canvas);
            }
            m_renderObjPtr->redraw();
        }
        //-------------------------------------------------------------------------------
        void Figure::addCanvas(Canvas2D* _canvas_ptr)
        {
            m_canvases.push_back(_canvas_ptr);
            _canvas_ptr->setData();
            update_data_limits(_canvas_ptr);
            _canvas_ptr->redraw();
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
        void Figure::update_data_limits(Canvas2D* _canvas)
        {
            if (m_dataLimX != m_dataLimX_prev)
            {
                m_dataLimX[0] = std::min(m_dataLimX[0], _canvas->dataLimX()[0]);
                m_dataLimX[1] = std::max(m_dataLimX[1], _canvas->dataLimX()[1]);
                m_dataLimX_prev = m_dataLimX;
                m_axesScalerPtr->setXLim(m_dataLimX);
            }
            if (m_dataLimY != m_dataLimY_prev)
            {
                m_dataLimY[0] = std::min(m_dataLimY[0], _canvas->dataLimY()[0]);
                m_dataLimY[1] = std::max(m_dataLimY[1], _canvas->dataLimY()[1]);
                m_dataLimY_prev = m_dataLimY;
                m_axesScalerPtr->setYLim(m_dataLimY);
            }

        }

    }
}
