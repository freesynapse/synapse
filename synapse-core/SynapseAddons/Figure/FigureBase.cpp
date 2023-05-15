
#include "FigureBase.h"
#include "Canvas/Canvas2D.h"
#include "Canvas/ScatterPlot2D.h"

#include "Canvas/Histogram2D.h"


namespace Syn
{
    namespace mplc
    {
        // global namespace instances
        Ref<Font> ss_title_font          = nullptr;
        Ref<Font> ss_axis_label_font     = nullptr;
        Ref<Font> ss_tick_label_font     = nullptr;
        Ref<Shader> ss_mplc_shader_2D    = nullptr;
        Ref<Shader> ss_mplc_shader_font  = nullptr;
        //-------------------------------------------------------------------------------
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
        void Figure::saveAsPNG(const std::string& _filename)
        {
            render();
            m_renderObjPtr->getFramebuffer()->saveAsPNG(_filename);
        }
        //-------------------------------------------------------------------------------
        bool Figure::add_canvas(const std::string& _canvas_id, Canvas2D* _canvas_ptr)
        {
            bool histogram_present = false;
            for (const auto& it : m_canvases)
            {
                if (it.second->m_canvasParameters.figure_type == FigureType::Histogram)
                {
                    histogram_present = true;
                    return false;
                }
            }

            if (_canvas_ptr->m_canvasParameters.figure_type == FigureType::Histogram &&
                !m_canvases.empty())
                return false;

            m_canvases.insert({ _canvas_id, _canvas_ptr });
            _canvas_ptr->setData();
            m_figureParamsPtr->figure_type = _canvas_ptr->m_canvasParameters.figure_type;
            return true;
            //updateDataLimits(); // called by setData().
        }
        //-------------------------------------------------------------------------------
        void Figure::updateDataLimits()
        {
            m_dataLimX = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            m_dataLimY = { std::numeric_limits<float>::max(), std::numeric_limits<float>::min() };
            
            m_canvasesDataSize = 0;
            bool x_nice_scale = true;
            bool y_nice_scale = true;
            float hist_x_add = 0.0f;
            for (auto& canvas : m_canvases)
            {
                update_data_limits_canvas(canvas.second);
                m_canvasesDataSize += canvas.second->size();
                // update nice scale properties from canvases
                x_nice_scale &= canvas.second->m_canvasParameters.x_nice_scale;
                y_nice_scale &= canvas.second->m_canvasParameters.y_nice_scale;

                if (canvas.second->m_canvasParameters.figure_type == FigureType::Histogram)
                {
                    Histogram2D* hist_ptr = dynamic_cast<Histogram2D*>(canvas.second);
                    hist_x_add = hist_ptr->bins_dx();
                    x_nice_scale = true;
                }
            }
            // update Figure behaviour
            m_figureParamsPtr->x_nice_scale = x_nice_scale;
            m_figureParamsPtr->y_nice_scale = y_nice_scale;

            if (m_dataLimX != m_dataLimX_prev)
            {
                m_dataLimX_prev = m_dataLimX;
                // the scaler needs to be updated from the resulting NiceScale that depends
                // on the data
                NiceScale x_ticks = NiceScale({ m_dataLimX[0], m_dataLimX[1] + hist_x_add }, x_nice_scale);
                glm::vec2 new_lim = { x_ticks.lower_bound, x_ticks.upper_bound };
                m_axesPtr->setXLim(new_lim, x_nice_scale);
            }
            
            if (m_dataLimY != m_dataLimY_prev)
            {
                m_dataLimY_prev = m_dataLimY;
                // the scaler needs to be updated from the resulting NiceScale that depends
                // on the data
                NiceScale y_ticks(m_dataLimY, y_nice_scale);
                glm::vec2 new_lim = { y_ticks.lower_bound, y_ticks.upper_bound };
                m_axesPtr->setYLim(new_lim, y_nice_scale);
            }
            m_renderObjPtr->m_redrawFlags = FIGURE_REDRAW;
        }
        //-------------------------------------------------------------------------------
        // ACCESSORS
        //
        glm::vec2 Figure::dataLimX(const std::string& _canvas_id)
        {
            Canvas2D* canvas = nullptr;
            if (m_canvases.size() == 1)
                canvas = m_canvases.begin()->second;
            else if (m_canvases.find(_canvas_id) != m_canvases.end())
                canvas = m_canvases[_canvas_id];

            if (canvas != nullptr)
            {
                glm::vec2 xlim = canvas->m_dataLimX;
                if (canvas->m_canvasParameters.figure_type == FigureType::Histogram)
                    xlim += glm::vec2(0.0f, dynamic_cast<Histogram2D*>(canvas)->m_bins_dx);
                return xlim;
            }
            
            //
            SYN_CORE_WARNING("Canvas with id '", _canvas_id, "' not found.");
            return glm::vec2(0.0f);
        }
        //-------------------------------------------------------------------------------
        glm::vec2 Figure::dataLimY(const std::string& _canvas_id)
        {
            Canvas2D* canvas = nullptr;
            if (m_canvases.size() == 1)
                canvas = m_canvases.begin()->second;
            else if (m_canvases.find(_canvas_id) != m_canvases.end())
                canvas = m_canvases[_canvas_id];

            if (canvas != nullptr)
                return canvas->m_dataLimY;
            
            //
            SYN_CORE_WARNING("Canvas with id '", _canvas_id, "' not found.");
            return glm::vec2(0.0f);
        }
        //-------------------------------------------------------------------------------
        Canvas2D* Figure::canvas(const std::string& _canvas_id)
        {
            if (m_canvases.size() == 1)
                return m_canvases.begin()->second;
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
                return rcParams.figure_sz_px; // in FigureParameters.h
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
