
#pragma once

#include <Synapse/Renderer/Buffers/VertexArray.hpp>
#include "Canvas2D.h"


namespace Syn
{
    namespace mplc
    {
        class ScatterPlot2D : public Canvas2D
        {
        public:
            ScatterPlot2D(const std::vector<float>& _data) {}
            ~ScatterPlot2D() {}

            void render() { printf("rendering scatterplot\n"); }
            void redraw() {}

        private:
            std::vector<float> m_dataX;
            std::vector<float> m_dataY;
        };
    }
}