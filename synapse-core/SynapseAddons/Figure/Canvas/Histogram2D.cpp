
#include "Histogram2D.h"

#include <cmath>

#include "../FigureUtils.h"


namespace Syn
{
    namespace mplc
    {
        Histogram2D::Histogram2D(Figure* _parent,
                                   const std::vector<float> _data,
                                   const std::string& _histogram_id)
        {
            m_parentRawPtr = _parent;
            // copy figure parameters
            memcpy(&m_canvasParameters, m_parentRawPtr->paramsRawPtr(), sizeof(figure_params_t));
            set_canvas_id(_histogram_id);
            m_data = std::vector<float>(_data);
            m_OpenGLPrimitive = GL_TRIANGLES;
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::setData()
        {
            m_binCount = m_canvasParameters.bin_count;

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

            // update limits
            m_dataLimX = { m_bins.begin()->first, m_bins.rbegin()->first };
            m_dataLimY = { 0.0f, std::numeric_limits<float>::min() };
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
            //float x_step = (params.x_axis_length - (2 * params.data_axis_offset.x)) / static_cast<float>(m_binCount);
            float x_step = (params.x_axis_length - (2 * params.data_axis_offset.x)) * m_bins_dx;

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
            _shader->setUniform4fv("u_color", m_canvasParameters.data_color);
            m_vaoData->bind();
            renderer.drawArrays(m_vertexCount, 0, true, m_OpenGLPrimitive);
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::data(const std::vector<float>& _data)
        {
            m_data = std::vector<float>(_data);
            setData();
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::setupBins(const glm::vec2& _lim)
        {
            m_bins = std::map<float, size_t>();
            
            //
            printf("bin count before = %d\n", m_binCount);
            
            if (m_binCount < 0)
            {
                /* Automatic determination of number of bins according to Freedman-
                 * Diaconis rule: the width of the bins are determined as 
                 *  w = (2 * IQR(x)) / (n^(-1/3))
                 * 
                 * The number of bins, then, is simply 
                 *  nbins = (max(x) - min(x)) / h
                 * 
                 */
                sort(m_data.begin(), m_data.end());
                glm::vec2 iqr = IQR(m_data);
                float h = (2.0f * (iqr[1] - iqr[0])) / (cbrt(static_cast<float>(m_data.size())));
                if (h > 0.0f)
                    m_binCount = static_cast<int>(floor((_lim[1] - _lim[0]) / h));
                else
                    m_binCount = 1;
            }
            printf("data[0] = %f, data[-1] = %f\n", m_data[0], m_data[m_data.size()-1]);
            
            if (m_binCount > 1)
                m_bins_dx = (_lim[1] - _lim[0]) / (m_binCount - 1);
            else if (m_binCount == 0)
                m_bins_dx = 1.0f;
            else
                m_bins_dx = 1.0f;
            
            printf("bin count after = %d\n", m_binCount);
            printf("dx = %f\n", m_bins_dx);

            //SYN_CORE_ASSERT_MESSAGE(m_binCount != 1, "(m_binCount - 1) = 0; pre-emptive avoidance of division by zero.");
            //SYN_CORE_ASSERT_MESSAGE(m_bins_dx != 0.0f, "m_bins_dx = 0.0f");
            //if (m_bins_dx == 0.0f || m_binCount <= 0)
            //{
            //    m_bins[0] = 0;
            //    return;
            //}


            float x = _lim[0];

            printf("--------------------------------------------------------------------\n");
            printf("lim = (%.2f, %.2f)\n", _lim[0], _lim[1]);
            printf("bin_count = %d\n", m_binCount);
            printf("dx = %.4f\n", m_bins_dx);
            for (size_t i = 0; i < m_binCount-1; i++)
            {
                m_bins[x] = 0;
                //printf("m_bins[%2.4f] = %zu\n", x, m_bins[x]);
                x += m_bins_dx;
            }
            // set last bin manually to avoid rounding errors
            x = _lim[1];
            m_bins[x] = 0;
            //printf("m_bins[%2.4f] = %zu\n", x, m_bins[x]);
            printf("--------------------------------------------------------------------\n");
        }
        //-------------------------------------------------------------------------------
        void Histogram2D::__debug_print()
        {
            //printf("--------------------- HISTOGRAM ----------------------\n");
            //printf("    X lim : %.2f, %.2f\n", m_dataLimX[0], m_dataLimX[1]);
            //printf("    Y lim : %.2f, %.2f\n", m_dataLimY[0], m_dataLimY[1]);
            m_parentRawPtr->axesPtr()->__debug_print();

        }
    }
}