
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
            ScatterPlot2D(Figure* _parent, 
                          const std::vector<float>& _X, 
                          const std::vector<float>& _Y,
                          const std::string& _scatter_id);
            ~ScatterPlot2D() {}

            /* Accessors for settting the data of a ScatterPlot2D.
             */
            virtual void data(const std::vector<float>& _Y) override;
            virtual void data(const std::vector<float>& _X, const std::vector<float>& _Y) override;
            /* Accessor for getting the data from a ScatterPlot2D.
             * Warning : requires a deep copy of both data vectors!
             */
            virtual std::pair<std::vector<float>, std::vector<float>> data() override { return { m_dataX, m_dataY }; }
            
            virtual const size_t size() const override { return m_dataX.size(); }


        protected:
            void setData() override;    // assumes complete m_dataX and m_dataY
            void redraw() override;
            void render(const Ref<Shader>& _shader) override;

        private:
            std::vector<float> m_dataX;
            std::vector<float> m_dataY;
        };
    }
}