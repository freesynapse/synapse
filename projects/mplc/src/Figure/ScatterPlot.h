
#pragma once

#include "Canvas2D.h"
#include "Figure.h"


namespace Syn
{
    namespace mplc
    {
        class Figure; // forward decl
        //
        class ScatterPlot : public Canvas2D
        {
        public:
            ScatterPlot(Figure* _parent_ptr) { this->m_parentFigPtr = _parent_ptr; }
            ~ScatterPlot() { printf("ScatterPlot deleted.\n"); }

            void render() {}
            void __debug()
            {
                printf("  __debug() ScatterPlot\n");
                printf("    m_markerSize = %f\n", m_markerSize);
                printf("    m_parentFigPtr->figSz = %f, %f\n", m_parentFigPtr->figureSz().x, m_parentFigPtr->figureSz().y);
            }
        private:
            float m_markerSize = 10.0f;

        };
    }
}
