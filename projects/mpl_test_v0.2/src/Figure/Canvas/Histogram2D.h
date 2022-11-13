
#pragma once

#include "Canvas2D.h"


namespace Syn
{
    namespace mplc
    {
        class Histogram2D : public Canvas2D
        {
        public:
            Histogram2D(Figure* _parent,
                        const std::vector<float> _data,
                        const std::string& _histogram_id,
                        size_t _bin_count=0);
            ~Histogram2D() = default;

            //
            virtual void setData() override;
            virtual void redraw() override;
            virtual void render(const Ref<Shader>& _shader) override;

            virtual void data(const std::vector<float>& _Y) override;

            // histogram-specific
            void nbins(size_t _n) { m_binCount = _n; }

        protected:
            virtual void setupBins(const glm::vec2& _lim);

        private:
            std::vector<float> m_data;
            size_t m_binCount;
            float m_bins_dx = 0.0f;
            std::map<float, size_t> m_bins;
        };
    }
}