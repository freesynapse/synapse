
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
            
            m_figureParamsPtr = MakeRef<figure_params_t>(fig_sz);
            m_axesPtr = MakeRef<Axes>(m_figureParamsPtr.get());
            
            m_renderObjPtr = MakeRef<FigureRenderObj>(this);
        }
        //-------------------------------------------------------------------------------
        Figure::~Figure()
        {
            for (auto& canvas : m_canvases)
                delete canvas.second;
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
            for (auto& canvas : m_canvases)
            {
                Canvas2D* canvas_ptr = canvas.second;
                canvas_ptr->redraw();
            }
            updateDataLimits();
            m_renderObjPtr->redraw();
        }
        //-------------------------------------------------------------------------------
        void Figure::add_canvas(const std::string& _canvas_id, Canvas2D* _canvas_ptr)
        {
            m_canvases.insert({ _canvas_id, _canvas_ptr });
            _canvas_ptr->setData();
            //updateDataLimits(); // called by setData().
        }
        //-------------------------------------------------------------------------------
        void Figure::updateDataLimits()
        {
            m_dataLimX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            m_dataLimY = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            
            m_canvasesDataSize = 0;
            for (auto& canvas : m_canvases)
            {
                update_data_limits_canvas(canvas.second);
                m_canvasesDataSize += canvas.second->size();
            }

            if (m_dataLimX != m_dataLimX_prev)
            {
                m_dataLimX_prev = m_dataLimX;
                // the scaler needs to be updated from the resulting NiceScale that depends
                // on the data
                NiceScale x_ticks(m_dataLimX);
                glm::vec2 new_lim = { x_ticks.lower_bound, x_ticks.upper_bound };
                m_axesPtr->setXLim(new_lim);
            }
            
            if (m_dataLimY != m_dataLimY_prev)
            {
                m_dataLimY_prev = m_dataLimY;
                // the scaler needs to be updated from the resulting NiceScale that depends
                // on the data
                NiceScale x_ticks(m_dataLimY);
                glm::vec2 new_lim = { x_ticks.lower_bound, x_ticks.upper_bound };
                m_axesPtr->setYLim(new_lim);
            }
            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW;
        }
        //-------------------------------------------------------------------------------
        // ACCESSORS
        //
        Canvas2D* Figure::canvas(const std::string& _canvas_id)
        {
            if (m_canvases.find(_canvas_id) != m_canvases.end())
                return m_canvases[_canvas_id];
            SYN_CORE_WARNING("Canvas with id '", _canvas_id, "' not found.");
            return nullptr;
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
        void Figure::update_data_limits_canvas(Canvas2D* _canvas)
        {
            m_dataLimX[0] = std::min(m_dataLimX[0], _canvas->dataLimX()[0]);
            m_dataLimX[1] = std::max(m_dataLimX[1], _canvas->dataLimX()[1]);

            m_dataLimY[0] = std::min(m_dataLimY[0], _canvas->dataLimY()[0]);
            m_dataLimY[1] = std::max(m_dataLimY[1], _canvas->dataLimY()[1]);
        }

    }
}
