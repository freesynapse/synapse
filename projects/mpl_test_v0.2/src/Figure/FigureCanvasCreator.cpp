
#include "FigureBase.h"
#include "Canvas/Canvas2D.h"
#include "Canvas/ScatterPlot2D.h"


namespace Syn
{
    namespace mplc
    {
        /*-------------------------------------------------------------------------------
         * 2D scatter plots
         *-------------------------------------------------------------------------------
         */

        /* Provided _Y values plotted against X [0 .. _Y.shape[0]]
         */
        void Figure::scatter(const std::vector<float>& _Y, 
                             scatter_params_t _params) 
        {
            // no X data provided, create it
            std::vector<float> X;
            for (size_t i = 0; i < _Y.size(); i++)
                X.push_back(static_cast<float>(i));

            m_figureParamsPtr->setFromScatterParams(_params);

            ScatterPlot2D* scatter_plot = new ScatterPlot2D(this, X, _Y);
            addCanvas(scatter_plot);
        }
        
        /* Both _X and _Y values are provided
         */
        void Figure::scatter(const std::vector<float>& _X, 
                             const std::vector<float>& _Y, 
                             scatter_params_t _params)
        {
            SYN_CORE_ASSERT(_X.size() == _Y.size(), "X and Y data must be of equal size.");

            m_figureParamsPtr->setFromScatterParams(_params);

            ScatterPlot2D* scatter_plot = new ScatterPlot2D(this, _X, _Y);
            scatter_plot->m_parentRawPtr = this;
            addCanvas(scatter_plot);
        }

    }
}
