
#pragma once

#include <unordered_map>
#include <Synapse/Memory/MemoryTypes.hpp>
#include <glm/glm.hpp>

#include "imgui/imgui_internal.h"

#include "FigureParameters.h"
#include "FigureRenderObj.h"
#include "FigureUtils.h"
#include "Canvas/CanvasParameters.h"


namespace Syn
{
    namespace mplc
    {
        class Canvas2D; // forward decl

        /* The Figure base class, which all plots inherits from. The Figure is the main, 
         * top hierarchical object. Each Figure object contains the different parts of 
         * the plot; the axes, the canvas (where the data is shown) etc. The different 
         * plots will have constructors for integral types, but underneath all data is
         * represented as floats. 
         */
        class Figure
        {
        public:
            friend class FigureRenderObj;

        public:
            Figure(const glm::vec2& _fig_sz_px);
            ~Figure();

            /* Render Figure, including all canvases.
             */
            void render();
            
            /* Redraws specific canvas.
             */
            void redraw(const std::string& _canvas_id);
            
            /* Redraws all canvases.
             */
            void redraw();            
            void update() { this->redraw(); }

            /* Updates the Figure data limits after redrawing all canvases.
             */
            void updateDataLimits();


            /*---------------------------------------------------------------------------
             * Canvas2D creation functions (FigureCanvasCreator.cpp)
             *---------------------------------------------------------------------------
             */
            
            /* Scatter creators
             */

            /* Creates a scatter plot with Y values, the X vector will be 
             * range(0, Y.size). Returns the canvas ID of the created plot, by which the 
             * plot can be accessed using Figure::canvas(ID).
             */
            const std::string& scatter(const std::vector<float>& _Y, 
                                       const std::string& _scatter_id="",
                                       scatter_params_t _params=scatter_params_t());
            
            /* Creates a scatter plot with X and Y value pairs. Returns the canvas ID of 
             * the created plot, by which the plot can be accessed using 
             * Figure::canvas(ID).
             */                          
            const std::string& scatter(const std::vector<float>& _X, 
                                       const std::vector<float>& _Y, 
                                       const std::string& _scatter_id="",
                                       scatter_params_t _params=scatter_params_t());

            
            /* Lineplot creators 
             */

            /* Creates a scatter plot with Y values. Returns the canvas ID of the created
             * plot, by which the plot can be accessed using Figure::canvas(ID).
             */
            const std::string& lineplot(const std::vector<float>& _Y,
                                        const std::string& _lineplot_id,
                                        lineplot_params_t _params=lineplot_params_t());

            /* Creates a scatter plot with X/Y value pairs. Returns the canvas ID of the 
             * created plot, by which the plot can be accessed using Figure::canvas(ID).
             */
            const std::string& lineplot(const std::vector<float>& _X,
                                        const std::vector<float>& _Y,
                                        const std::string& _lineplot_id,
                                        lineplot_params_t _params=lineplot_params_t());

            /* Creates a scatter plot with multiple Y value series. X values for these 
             * will be [0...len(Y.shape[1]). Returns the canvas ID of the created plot, 
             * by which the plot can be accessed using Figure::canvas(ID).
             */
            const std::string& lineplot(const std::vector<std::vector<float>>& _Y,
                                        const std::string& _lineplot_id,
                                        lineplot_params_t _params=lineplot_params_t());

            /* Creates a scatter plot with multiple X/Y value pairs series. Returns the 
             * canvas ID of the created plot, by which the plot can be accessed using 
             * Figure::canvas(ID).
             */
            const std::string& lineplot(const std::vector<std::vector<float>>& _X,
                                        const std::vector<std::vector<float>>& _Y,
                                        const std::string& _lineplot_id,
                                        lineplot_params_t _params=lineplot_params_t());

            /* Histogram over data, showing the distribution. Two functions are available,
             * where the bin count is specified or not.
             */
            const std::string& histogram(const std::vector<float>& _data,
                                         const std::string& _histogram_id,
                                         histogram_params_t _params=histogram_params_t());
            //
            const std::string& histogram(const std::vector<float>& _data,
                                         const std::string& _histogram_id,
                                         size_t _bin_count,
                                         histogram_params_t _params=histogram_params_t());

            /* General accessors into the Canvas2D if only one canvas is present, for 
             * updating data. Calls the data() functions of the canvas.
             *
             * (FigureCanvasCreator.cpp)
             */
            void data(const std::vector<float>&);
            void data(const std::vector<float>&, const std::vector<float>&);
            void data(const std::vector<std::vector<float>>&);
            void data(const std::vector<std::vector<float>>&, const std::vector<std::vector<float>>&);
        private:
            Canvas2D* get_single_canvas(const std::string& _caller_func);
            //std::pair<std::vector<float>, std::vector<float>> data() { return {}; }
        
        private:
            bool add_canvas(const std::string& _canvas_id, Canvas2D* _canvas);

            /*---------------------------------------------------------------------------
             * Accessors
             *---------------------------------------------------------------------------
             */
        public:
            void title(const std::string& _title) { m_figureTitle = _title; }
            const std::string& title() { return m_figureTitle; }
            const glm::vec2& size() const { return m_figureSizePx; }
            inline GLuint framebufferTexturePtr() { return m_renderObjPtr->m_framebuffer->getColorAttachmentIDn(0); };
            inline ImVec2 size() { return ImVec2(m_figureSizePx.x, m_figureSizePx.y); };
            Ref<figure_params_t> paramsPtr() { return m_figureParamsPtr; };
            figure_params_t* paramsRawPtr() { return m_figureParamsPtr.get(); }
            void setRedrawFlags(uint32_t _flags) { m_renderObjPtr->m_redrawFlags = _flags; }
            void addRedrawFlags(uint32_t _flags) { m_renderObjPtr->m_redrawFlags |= _flags; }
            const Ref<Axes>& axesPtr() { return m_axesPtr; }
            const glm::vec2& dataLimX() { return m_dataLimX; }
            const glm::vec2& dataLimY() { return m_dataLimY; }
            // Canvas-related accessors
            const std::unordered_map<std::string, Canvas2D*>& canvases() { return m_canvases; }
            Canvas2D* canvas(const std::string& _canvas_id);
            size_t dataSize() { return m_canvasesDataSize; }
            // aux rendering control (grid lines, selections, etc.)
            void enableGridLines() { m_renderObjPtr->m_auxRenderFlags |= FIGURE_RENDER_GRIDLINES; }
            void disableGridLines() { m_renderObjPtr->m_auxRenderFlags &= (~FIGURE_RENDER_GRIDLINES); }
            void fill(int _axis, const glm::vec2& _lim)
            {
                if (_axis == X_AXIS)    m_renderObjPtr->m_fillLimX = _lim;
                else                    m_renderObjPtr->m_fillLimY = _lim;
                m_renderObjPtr->m_auxRenderFlags |= (_axis == X_AXIS ? FIGURE_RENDER_FILL_X : FIGURE_RENDER_FILL_Y);
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_FILL;
            }
            void disableFill(int _axis)
            {
                if (_axis == X_AXIS)    m_renderObjPtr->m_fillLimX = { 1.0f, -1.0f };
                else                    m_renderObjPtr->m_fillLimY = { 1.0f, -1.0f };
                m_renderObjPtr->m_auxRenderFlags &= (_axis == X_AXIS ? (~FIGURE_RENDER_FILL_X) : (~FIGURE_RENDER_FILL_Y));
                m_renderObjPtr->m_redrawFlags &= (~FIGURE_REDRAW_FILL);
            }
            void fillBetweenX(const glm::vec2& _lim)
            {
                m_renderObjPtr->m_fillLimX = _lim;
                m_renderObjPtr->m_auxRenderFlags |= FIGURE_RENDER_FILL_X;
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_FILL;
            }
            void fillBetweenY(const glm::vec2& _lim)
            {
                m_renderObjPtr->m_fillLimY = _lim;
                m_renderObjPtr->m_auxRenderFlags |= FIGURE_RENDER_FILL_Y;
                m_renderObjPtr->m_redrawFlags |= FIGURE_REDRAW_FILL;
            }


        /*-------------------------------------------------------------------------------
         * Private member functions
         *-------------------------------------------------------------------------------
         */
        private:
            /* Checks figure size at FigureObj and uses default if needed.
             */
            glm::vec2 check_fig_size(const glm::vec2& _fig_sz);
            
            /* Updates the Figure limits from a single canvas.
             */
            void update_data_limits_canvas(Canvas2D* _canvas);


        private:
            glm::vec2 m_figureSizePx;

            std::string m_figureTitle = "";

            Ref<FigureRenderObj> m_renderObjPtr = nullptr;
            Ref<Axes> m_axesPtr = nullptr;

            Ref<figure_params_t> m_figureParamsPtr = nullptr;

            float __debug_update_timer = 0.0f;

            size_t m_canvasesDataSize = 0;
            
            std::unordered_map<std::string, Canvas2D*> m_canvases;
            glm::vec2 m_dataLimX        = {  0.0f,  1.0f };
            glm::vec2 m_dataLimX_prev   = { -1.0f, -1.0f };
            glm::vec2 m_dataLimY        = {  0.0f,  1.0f };
            glm::vec2 m_dataLimY_prev   = { -1.0f, -1.0f };
        };
    }
}

