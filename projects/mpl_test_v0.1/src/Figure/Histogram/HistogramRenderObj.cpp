
#include "Histogram.h"
#include "HistogramRenderObj.h"


namespace Syn
{
    namespace mplc
    {

        HistogramRenderObj::HistogramRenderObj(Histogram* _parent, const glm::vec2& _fig_sz)
        {
            m_parentRawPtr = (Histogram*)_parent;
            m_figureSz = _fig_sz;

            // create the frambuffer
            this->initializeBaseClass();

            // set title, this requires access to the parent class
            this->m_figureTitle = m_parentRawPtr->m_figureTitle;
            this->m_figureParamsPtr->figure_type = FigureType::Histogram;

            // histogram-specific
            m_dataOpenGLPrimitive = GL_TRIANGLES;
        }
        //-------------------------------------------------------------------------------
        void HistogramRenderObj::setInteractiveMode(bool _b)
        {
            m_interactiveMode = _b;
            
            if (m_interactiveMode)
            {
                m_currFillLimX = { m_parentRawPtr->minBin(), m_parentRawPtr->maxBin() };
                m_prevFillLimX = m_currFillLimX;

                m_redrawFlags |= FIGURE_REDRAW_SELECTION;
            }
        }
        //-------------------------------------------------------------------------------
        void HistogramRenderObj::fillBetweenX(const float& _lo, const float& _hi)
        {
            // TODO : there's a bug for selection and limits for floating-point histograms. 
            //        To fix when needed.
            
            static bool reported = false;
        
            if (!m_interactiveMode)
            {
                if (!reported)
                {
                    SYN_CORE_WARNING("m_interactiveMode is FALSE.");
                    reported = true;
                }
                return;
            }

            bool int_mode = m_parentRawPtr->m_integralType;

            float lo = _lo;
            float hi = _hi;
            
            if (!int_mode)
            {
                float lo = m_parentRawPtr->findClosestBin(_lo)->first;
                float hi = m_parentRawPtr->findClosestBin(_hi)->first;
            }

            if (lo < m_parentRawPtr->minBin())
            {
                SYN_CORE_WARNING("invalid histogram interval selected : ", lo, ", ", hi, ".");
                m_currFillLimX[0] = m_parentRawPtr->minBin();
                return;
            }
            else if (hi > m_parentRawPtr->maxBin())
            {
                SYN_CORE_WARNING("invalid histogram interval selected : ", lo, ", ", hi, ".");
                m_currFillLimX[1] = m_parentRawPtr->maxBin();
                return;
            }
            
            // auto adjust entered values within bounds
            if (lo > hi)
            {
                if (hi < m_parentRawPtr->minBin())
                {
                    hi = m_parentRawPtr->m_bins.begin()->first;
                    lo = hi;
                }
                else if (hi < m_parentRawPtr->maxBin())
                {
                    auto next_hi = std::next(m_parentRawPtr->m_bins.find(hi));
                    hi = next_hi->first;
                    lo = std::prev(next_hi)->first;
                }
                else if (hi == m_parentRawPtr->maxBin())
                    lo = std::prev(m_parentRawPtr->m_bins.find(hi))->first;
            }

            m_currFillLimX = { lo, hi };

            // preset coordinates of selection for rendering in FigureRenderObjBase->redrawSelection()
            normalized_params_t norm_params = normalized_params_t(m_figureParamsPtr);
            int offset = static_cast<int>(m_parentRawPtr->minBin());
            float xmin = m_dataPositions[static_cast<int>(m_currFillLimX[0]) - offset].x + norm_params.canvas_origin.x + norm_params.data_axis_offset.x;
            float xmax = m_dataPositions[0].x + norm_params.canvas_origin.x + norm_params.data_axis_offset.x + \
                            (static_cast<int>(m_currFillLimX[1] - offset + 1) * norm_params.data_spacing);

            m_fillPositionsSelX[0] = { xmin, norm_params.y_axis_lim[0] };
            m_fillPositionsSelX[1] = { xmax, norm_params.y_axis_lim[1] };

            // update
            if (m_currFillLimX != m_prevFillLimX || !m_figureParamsPtr->fill_between_x)
            {
                m_redrawFlags |= FIGURE_REDRAW_SELECTION;
                m_prevFillLimX = m_currFillLimX;
            }

            m_figureParamsPtr->fill_between_x = true;
        }
        //-------------------------------------------------------------------------------
        void HistogramRenderObj::redrawData(normalized_params_t* _fig_params)
        {
            // scaling -- find bin with highest counts
            m_dataLimY[1] = 0.0f;
            for (const auto& it : m_parentRawPtr->m_bins)
                m_dataLimY[1] = std::max(m_dataLimY[1], static_cast<float>(it.second));
            float max_n_inv = (1.0f - _fig_params->canvas_origin.y - _fig_params->data_axis_offset.y) / m_dataLimY[1];

            //
            m_dataVertexCount = 0;
            std::vector<glm::vec2> V;
            
            float x_step = (_fig_params->x_axis_length - 2 * (_fig_params->data_axis_offset.x)) / static_cast<float>(m_parentRawPtr->m_binCount);
            float x = _fig_params->canvas_origin.x + _fig_params->data_axis_offset.x;
            //
            float x0, x1, y0, y1;
            for (const auto& it : m_parentRawPtr->m_bins)
            {
                x0 = x + (_fig_params->bar_spacing * 0.5f);
                x1 = x + x_step - (_fig_params->bar_spacing * 0.5f);
                y0 = _fig_params->canvas_origin.y + _fig_params->data_axis_offset.y;
                y1 = _fig_params->canvas_origin.y + \
                    static_cast<float>(it.second) * max_n_inv * _fig_params->data_height + \
                    _fig_params->data_axis_offset.y;

                V.push_back({ x0, y0 });    V.push_back({ x1, y0 });    V.push_back({ x1, y1 });
                V.push_back({ x1, y1 });    V.push_back({ x0, y1 });    V.push_back({ x0, y0 });

                // Store X positions so that the base class can deduce the ticks positions. 
                // For histograms, ticks are drawn in between bars, corresponding to the 
                // x0 position. N.B.: The positions are stored as offsets from 
                // _fig_params->canvas_origin
                //
                m_dataPositions.push_back({ 
                    x0 - (_fig_params->canvas_origin.x + _fig_params->data_axis_offset.x),
                    y1 - y0
                });

                // Store x tick labels, the derived class knows what's best... 
                // FigureRenderObjBase can then just put the labels on the positions 
                // deduced by FigureRenderObjBase->redrawTicks().
                std::stringstream ss;
                float val = it.first;
                if (!m_parentRawPtr->m_integralType)
                    ss << std::setprecision(1) << std::fixed << val;
                else
                    ss << val;
                //        
                m_tickLabelsX.push_back(ss.str());

                x += x_step;
            }
            // add the last tick
            m_dataPositions.push_back({ x1, y1 });

            // store data spacing for tick positions
            float data_spacing = (V.size() > 6 ? V[6].x - V[0].x : 0.0f);
            _fig_params->data_spacing = data_spacing;
            m_figureParamsPtr->data_spacing = data_spacing;

            //
            m_dataVertexCount = static_cast<uint32_t>(V.size());
            //
            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
            });
            vbo->setData((void*)(&V[0]), sizeof(glm::vec2) * m_dataVertexCount);

            m_vaoData = API::newVertexArray(vbo);
            Renderer::get().executeRenderCommands();
        }

    }
}
