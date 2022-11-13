
#include "Histogram2D.h"


namespace Syn
{
    namespace mplc
    {
        Histogram2D::Histogram2D(Figure* _parent,
                                   const std::vector<float> _data,
                                   const std::string& _histogram_id,
                                   size_t _bin_count)
        {
            m_parentRawPtr = _parent;
            // copy figure parameters
            memcpy(&m_canvasParameters, m_parentRawPtr->paramsRawPtr(), sizeof(figure_params_t));
            set_canvas_id(_histogram_id);
            m_data = std::vector<float>(_data);
            m_binCount = _bin_count;
            m_OpenGLPrimitive = GL_TRIANGLES;
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::setData()
        {
            glm::vec2 lim = { std::numeric_limits<float>::max(), 
                                std::numeric_limits<float>::min() };
            for (size_t i = 0; i < m_data.size(); i++)
            {
                lim[0] = std::min(lim[0], m_data[i]);
                lim[1] = std::max(lim[1], m_data[i]);
            }

            setupBins(lim);

            for (const auto& val : m_data)
            {
                if (val < lim[0] || val > lim[1])
                    continue;

                std::prev(m_bins.upper_bound(val))->second++;                
            }

            // limits on canvas (for rendering)
            m_dataLimX = { m_bins.begin()->first, m_bins.rbegin()->first };
            m_dataLimY[0] = 0.0f;
            for (const auto& bin : m_bins)
                m_dataLimY[1] = std::max(m_dataLimY[1], static_cast<float>(bin.second));
            
            m_parentRawPtr->updateDataLimits();
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::redraw()
        {
            //Vad gör egentligen m_parentRawPtr->updateDataLimits för bins? Funkar det med en Axes för att hitta rätt värden på X?
            //Tror inte det, men kan vara värt att kolla... Njae...

            std::vector<glm::vec3> V;
            normalized_params_t params = normalized_params_t(m_canvasParameters);
            const Ref<Axes>& axes = m_parentRawPtr->axesPtr();

            // Special case for histograms since these do not obey the same X scaling
            // convention as linear data. Hence, X values will be 'manually' calculated.
            // Y values follow the conventional scaling, and the Axes object is used for
            // calculation.
            //
            float x = params.canvas_origin.x + params.data_axis_offset.x;
            float x_step = (params.x_axis_length - (2 * params.data_axis_offset.x)) / static_cast<float>(m_binCount);

            //
            float x0, x1, y0, y1;
            float z = params.z_value_data;
            float x_offset = 0.5f * params.bar_spacing;
            //
            //for (auto& this_bin : m_bins)
            for (auto this_bin = m_bins.begin(); this_bin != m_bins.end(); this_bin++)
            {
                x0 = axes->eval_x(this_bin->first) + x_offset;
                x1 = axes->eval_x(this_bin->first + m_bins_dx) - x_offset;

                //x0 = x + (0.5f * params.bar_spacing);
                //x1 = x + x_step - (0.5f * params.bar_spacing);
                y0 = axes->eval_y(0.0f);
                y1 = axes->eval_y(static_cast<float>(this_bin->second));
                V.push_back({ x0, y0, z }); V.push_back({ x1, y0, z }); V.push_back({ x1, y1, z }); // 0, 1, 2
                V.push_back({ x1, y1, z }); V.push_back({ x0, y1, z }); V.push_back({ x0, y0, z }); // 2, 3, 0

                x += x_step;
            }

            m_vertexCount = static_cast<uint32_t>(V.size());

            Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
            vbo->setBufferLayout({
                { VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float3, "a_position" },
            });
            vbo->setData((void*)&(V[0]), sizeof(glm::vec3) * m_vertexCount);

            m_vaoData = API::newVertexArray(vbo);

            Renderer::get().executeRenderCommands();
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::render(const Ref<Shader>& _shader)
        {
            static auto& renderer = Renderer::get();
            _shader->setUniform4fv("u_color", m_canvasParameters.stroke_color);
            m_vaoData->bind();
            renderer.drawArrays(m_vertexCount, 0, true, m_OpenGLPrimitive);
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::data(const std::vector<float>& _Y)
        {

        }
        //-------------------------------------------------------------------------------
        void Histogram2D::setupBins(const glm::vec2& _lim)
        {
            m_bins.clear();
            if (m_binCount == 0)
                m_binCount = static_cast<size_t>(_lim[1] - _lim[0]) + 1;
            
            float x = _lim[0];
            float dx = (_lim[1] - _lim[0]) / (m_binCount - 1);

            while (x <= _lim[1])
            {
                m_bins[x] = 0;
                x += dx;
            }
            
            m_binCount = m_bins.size();
            m_bins_dx = dx;
        }

    }
}