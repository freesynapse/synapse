
#pragma once

#include <vector>
#include <Synapse/Memory/MemoryTypes.hpp>
#include <glm/glm.hpp>

#include "imgui/imgui_internal.h"

#include "FigureParameters.h"
#include "FigureRenderObj.h"


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

            void render();
            void redraw();
            void update() { this->redraw(); }


            /*---------------------------------------------------------------------------
             * CANVAS 2D CREATION FUNCTIONS
             *---------------------------------------------------------------------------
             */
            void scatter(const std::vector<float>& _data);
            //void scatter(const std::vector<int>& _data);

            /*---------------------------------------------------------------------------
             * Accessors
             *---------------------------------------------------------------------------
             */
            void title(const std::string& _title) { m_figureTitle = _title; }
            const std::string& title() { return m_figureTitle; }
            const glm::vec2& figureSz() const { return m_figureSizePx; }
            inline GLuint figurePtrID() { return m_renderObjPtr->textureID(); };
            inline ImVec2 figureSz() { return ImVec2(m_figureSizePx.x, m_figureSizePx.y); };
            Ref<figure_params_t> params() { return m_renderObjPtr->m_figureParamsPtr; };
            

        private:
            /* Checks figure size at FigureObj and uses default if needed.
             */
            glm::vec2 check_fig_size(const glm::vec2& _fig_sz);
            
            /* Updates the Figure data limits after redrawing all canvases
             */
            void updata_data_limits(Canvas2D* _canvas);


        private:
            glm::vec2 m_figureSizePx;

            std::string m_figureTitle = "";
            Ref<FigureRenderObj> m_renderObjPtr = nullptr;

            std::vector<Canvas2D*> m_canvases;
            glm::vec2 m_dataLimX = { 0.0f, 1.0f };
            glm::vec2 m_dataLimY = { 0.0f, 1.0f };
        };
    }
}

