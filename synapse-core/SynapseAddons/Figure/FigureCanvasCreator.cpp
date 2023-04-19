
#include "FigureBase.h"
#include "Canvas/Canvas2D.h"
#include "Canvas/ScatterPlot2D.h"
#include "Canvas/LinePlot2D.h"
#include "Canvas/Histogram2D.h"


namespace Syn
{
    namespace mplc
    {
        /*-------------------------------------------------------------------------------
         * 2D scatter plots
         *-------------------------------------------------------------------------------
         */

        //Provided _Y values plotted against X [0 .. _Y.shape[0]]
        //
        const std::string& Figure::scatter(const std::vector<float>& _Y, 
                             const std::string& _scatter_id,
                             scatter_params_t _params)
        {
            // no X data provided, create it
            std::vector<float> X;
            for (size_t i = 0; i < _Y.size(); i++)
                X.push_back(static_cast<float>(i));

            return scatter(X, _Y, _scatter_id, _params);
        }
        //-------------------------------------------------------------------------------
        // Both _X and _Y values are provided
        //
        const std::string& Figure::scatter(const std::vector<float>& _X, 
                                           const std::vector<float>& _Y, 
                                           const std::string& _scatter_id,
                                           scatter_params_t _params)
        {
            ScatterPlot2D* scatter_plot = new ScatterPlot2D(this, _X, _Y, _scatter_id);
            scatter_plot->m_parentRawPtr = this;
            scatter_plot->m_canvasParameters.setFromScatterParams(_params);
            bool res = add_canvas(scatter_plot->canvasID(), scatter_plot);
            if (!res)
                delete scatter_plot;
                
            return scatter_plot->canvasID();
        }

        /*-------------------------------------------------------------------------------
         * 2D line plots
         *-------------------------------------------------------------------------------
         */
        const std::string& Figure::lineplot(const std::vector<float>& _Y,
                                            const std::string& _lineplot_id,
                                            lineplot_params_t _params)
        {
            std::vector<std::vector<float>> X;
            std::vector<std::vector<float>> Y;
            Y.push_back(_Y);
            
            std::vector<float> X_;
            for (size_t i = 0; i < _Y.size(); i++)
                X_.push_back(static_cast<float>(i));
            X.push_back(X_);

            return lineplot(X, Y, _lineplot_id, _params);
        }
        //-------------------------------------------------------------------------------
        const std::string& Figure::lineplot(const std::vector<float>& _X,
                                            const std::vector<float>& _Y,
                                            const std::string& _lineplot_id,
                                            lineplot_params_t _params)
        {
            std::vector<std::vector<float>> X;
            std::vector<std::vector<float>> Y;
            X.push_back(_X);
            Y.push_back(_Y);

            return lineplot(X, Y, _lineplot_id, _params);
        }
        //-------------------------------------------------------------------------------
        const std::string& Figure::lineplot(const std::vector<std::vector<float>>& _Y,
                                            const std::string& _lineplot_id,
                                            lineplot_params_t _params)
        {
            std::vector<std::vector<float>> Y = std::vector<std::vector<float>>(_Y);
            std::vector<std::vector<float>> X;
            //
            for (size_t i = 0; i < Y.size(); i++)
            {
                std::vector<float> X_;
                for (size_t j = 0; j < Y[i].size(); j++)
                    X_.push_back(static_cast<float>(j));
                X.push_back(X_);
            }
            
            return lineplot(X, Y, _lineplot_id, _params);
        }
        //-------------------------------------------------------------------------------
        // Base case : this is called by all above
        //
        const std::string& Figure::lineplot(const std::vector<std::vector<float>>& _X,
                                            const std::vector<std::vector<float>>& _Y,
                                            const std::string& _lineplot_id,
                                            lineplot_params_t _params)
        {
            LinePlot2D* line_plot = new LinePlot2D(this, _X, _Y, _lineplot_id);
            line_plot->m_parentRawPtr = this;
            line_plot->m_canvasParameters.setFromLinePlotParams(_params);
            bool res = add_canvas(line_plot->canvasID(), line_plot);
            if (!res)
                delete line_plot;

            return line_plot->canvasID();
        }
        /*-------------------------------------------------------------------------------
         * 2D histogram
         *-------------------------------------------------------------------------------
         */
        const std::string& Figure::histogram(const std::vector<float>& _data,
                                             const std::string& _histogram_id,
                                             histogram_params_t _params)
        {
            Histogram2D* histogram = new Histogram2D(this, _data, _histogram_id);
            histogram->m_parentRawPtr = this;
            histogram->m_canvasParameters.setFromHistParams(_params);
            bool res = add_canvas(histogram->canvasID(), histogram);
            if (!res)
                delete histogram;

            return histogram->canvasID();
        }
        /*-------------------------------------------------------------------------------
         * Canvas data update accessors
         *-------------------------------------------------------------------------------
         */
        void Figure::data(const std::vector<float>& _Y)
        {
            Canvas2D* canvas = get_single_canvas("void Figure::data(const std::vector<float>& _Y)");
            if (canvas)
                canvas->data(_Y);
        }
        //-------------------------------------------------------------------------------
        void Figure::data(const std::vector<float>& _X, const std::vector<float>& _Y)
        {
            Canvas2D* canvas = get_single_canvas("void Figure::data(const std::vector<float>& _X, const std::vector<float>& _Y)");
            if (canvas)
                canvas->data(_X, _Y);
        }
        //-------------------------------------------------------------------------------
        void Figure::data(const std::vector<std::vector<float>>& _Y)
        {
            Canvas2D* canvas = get_single_canvas("void Figure::data(const std::vector<std::vector<float>>& _Y)");
            if (canvas)
                canvas->data(_Y);
        }
        //-------------------------------------------------------------------------------
        void Figure::data(const std::vector<std::vector<float>>& _X, 
                          const std::vector<std::vector<float>>& _Y)
        {
            Canvas2D* canvas = get_single_canvas("void Figure::data(const std::vector<std::vector<float>>& _X, const std::vector<std::vector<float>>& _X)");
            if (canvas)
                canvas->data(_X, _Y);
        }
        //-------------------------------------------------------------------------------
        Canvas2D* Figure::get_single_canvas(const std::string& _caller_func)
        {
            if (m_canvases.size() != 1)
            {
                SYN_CORE_WARNING("m_canvases.size() != 1 : called from ", _caller_func, ".");
                return nullptr;
            }
            return m_canvases.begin()->second;
        }


    }
}
