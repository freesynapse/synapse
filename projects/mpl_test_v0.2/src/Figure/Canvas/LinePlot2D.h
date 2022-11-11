
#pragma once

#include "Canvas2D.h"


namespace Syn
{
    namespace mplc
    {
        class LinePlot2D : public Canvas2D
        {
        public:
            LinePlot2D(Figure* _parent, 
                       const std::vector<std::vector<float>>& _X,
                       const std::vector<std::vector<float>>& _Y,
                       const std::string& _lineplot_id);
            ~LinePlot2D() = default;

            //
            virtual void setData() override;
            virtual void redraw() override;
            virtual void render(const Ref<Shader>& _shader) override;

            virtual void data(const std::vector<float>& _Y) override;
            virtual void data(const std::vector<float>& _X, 
                              const std::vector<float>& _Y) override;
            virtual void data(const std::vector<std::vector<float>>& _Y) override;
            virtual void data(const std::vector<std::vector<float>>& _X, 
                              const std::vector<std::vector<float>>& _Y) override;

            // accessors
            /* Returns the max shape of the data. The number of rows are expected to be
             * equal for X and Y (axis 0). However, the length of individual column 
             * vectors (axis 1) could be of different length (although X and Y have to
             * match for individual columns).
             */
            const glm::ivec2& shape() { return m_maxShape; }

        private:
            std::vector<std::vector<float>> m_dataX;
            std::vector<std::vector<float>> m_dataY;
            glm::ivec2 m_maxShape = { 0, 0 };

            uint32_t m_markerVertexCount = 0;
            Ref<VertexArray> m_vaoMarkers = nullptr;

        };
    }
}