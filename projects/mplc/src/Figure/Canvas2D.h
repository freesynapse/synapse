
#pragma once

#include <stdio.h>


namespace Syn
{
    namespace mplc
    {
        class Figure; // forward decl
        //
        class Canvas2D
        {
        public:
            Canvas2D() {}
            virtual ~Canvas2D() = default;

            virtual void render() = 0;
            virtual void __debug() = 0;


        protected:
            Figure* m_parentFigPtr = nullptr;
        };

    }
}
