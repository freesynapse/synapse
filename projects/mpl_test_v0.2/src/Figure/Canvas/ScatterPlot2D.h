
#pragma once

#include "Canvas2D.h"
#include "../FigureBase.h"
#include "../FigureParameters.h"


namespace Syn
{
    namespace mplc
    {
        //
        class ScatterPlot2D : public Canvas2D
        {
        public:
            ScatterPlot2D(Figure* _parent, const std::vector<float>& _X, const std::vector<float>& _Y);
            ~ScatterPlot2D() {}

            void setData() override;    // assumes complete m_dataX and m_dataY
            void redraw() override;

        private:
            std::vector<float> m_dataX;
            std::vector<float> m_dataY;
        };
    }
}