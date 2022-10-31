
#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>

#include "imgui/imgui_internal.h"

#include "Canvas2D.h"


namespace Syn
{
    namespace mplc
    {
        
        /* The base object, holds pointers to different Canvases (eg. scatter plots etc).
         * The different Canvas-creation methods (.scatter(), .lineplot() etc) creates a
         * new Canvas object. Upon creation, an optional ID can be chosen, which allows 
         * later access to that Canvas. Everytime a Canvas is created, it's added to 
         * the Canvas vector.
         * The render() member function calls each respective Canvas' render function.
         * Selections are made on the figure and passed down to the repsective Canvas.
         */
        class Figure
        {
        public:
            Figure(const glm::vec2& _fig_sz_px) : 
                m_figureSizePx(_fig_sz_px) 
            {}
            ~Figure()
            {
                for (auto* canvas : m_canvases)
                    delete canvas;
            };

            void scatter();

            // accessors
            GLuint figurePtrID() { return 0; }
            const glm::vec2& figureSz() { return m_figureSizePx; }
            ImVec2 imGuiFigureSz() { return ImVec2(m_figureSizePx.x, m_figureSizePx.y); }

            //
            void render()
            {
                for (auto* canvas : m_canvases)
                    canvas->render();
            };
            //
            void __debug()
            {
                for (auto* canvas : m_canvases)
                    canvas->__debug();
            }

            void updateCanvas(const std::string& _canvas_id) {};
            void removeCanvas(const std::string& _canvas_id) {};
        
        private:
            glm::vec2 m_figureSizePx = { 0.0f, 0.0f };
            glm::vec2 m_dataXLim = { 0.0f, 1.0f };
            glm::vec2 m_dataYLim = { 0.0f, 1.0f };

            std::vector<Canvas2D*> m_canvases;
        };
    }
}


