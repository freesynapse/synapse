
#include <Synapse/Input/InputManager.hpp>
#include <Synapse/Debug/Log.hpp>
#include <Synapse/Debug/Profiler.hpp>
#include <Synapse/Utils/Random/Random.hpp>
#include <Synapse/Utils/Timer/TimeStep.hpp>

#include "SandWorld.hpp"


namespace Syn
{
    void SandWorld::onUpdate(float _dt)
    {
        SYN_PROFILE_FUNCTION();

        // handle mouse interaction
        handleInput();

        // update all cells
        for (size_t x = 0; x < m_size.x; x++)
        {
            for (size_t y = 0; y < m_size.y; y++)
            {
                const element_t& e = getElement(x, y);

                if (e.type == ElementType::NONE)
                    continue;

                if (e.props & ElementProperties::MOVE_DOWN && moveDown(x, y, e)) {}
                else if (e.props & ElementProperties::MOVE_DOWN_SIDE && moveDownSide(x, y, e)) {}
                else if (e.props & ElementProperties::MOVE_SIDE && moveSide(x, y, e)) {}
            }
        }

        commitMoves();

    }

	//-----------------------------------------------------------------------------------
    void SandWorld::onRender()
    {
        SYN_PROFILE_FUNCTION();

        // upload updated texture
        m_texture->setData(m_colorBuffer, sizeof(uint8_t) * m_elementBufferSz);

        // Bind shader, use texture (screen buffer) and render
        m_shader->enable();
        m_texture->bind();
        m_screenQuad->renderNDC();

    }

	//-----------------------------------------------------------------------------------
    void SandWorld::handleInput()
    {
        glm::vec2 pos = InputManager::get_mouse_position();
        glm::ivec2 vp_offset = Renderer::getImGuiViewportOffset();
        static glm::vec2 lastPos(0.0f);
        static glm::vec2 scale_factor = Renderer::getViewportF() / glm::vec2(m_size.x, m_size.y);
        
        if (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1))
        {
            //pos = InputManager::get_mouse_position();
            pos -= vp_offset;
            pos /= scale_factor;
            pos.y = m_size.y - pos.y;

            for (int i = -m_brushRadius; i < m_brushRadius; i++)
            {
                for (int j = -m_brushRadius; j < m_brushRadius; j++)
                {
                    if (inBounds(pos.x + i, pos.y + j))
                    {
                        size_t idx = getIndex(pos.x + i, pos.y + j);
                        m_colorBuffer[idx] = enum_val(m_selectedBrush.type);
                        m_elementBuffer[idx] = m_selectedBrush;
                    }
                }
            }

            //line2D(pos, lastPos, _SAND);
            lastPos = pos;
        }

        //if (TimeStep::getFrameCount() % 100 == 0)
        //{
        //    Log::debug_vector(__func__, "mouse pos", pos);
        //    Log::debug_vector(__func__, "offset", vp_offset);
        //    Log::debug_vector(__func__, "scale factor", scale_factor);
        //}
    }

    // private --------------------------------------------------------------------------
    void SandWorld::commitMoves()
    {
        // remove moves whose desitnations (.first) are already filled
        for (size_t i = 0; i < m_changes.size(); i++)
        {
            if (m_elementBuffer[m_changes[i].first].type != ElementType::NONE)
            {
                m_changes[i] = m_changes.back();
                m_changes.pop_back();
                i--;
            }
        }

        // sort moves
        std::sort(m_changes.begin(), m_changes.end(), 
                  [](auto& a, auto& b) { return a.first < b.first; });
        
        // pick moves at random
        uint32_t iprev = 0;
        m_changes.emplace_back(-1, -1);

        for (size_t i = 0; i < m_changes.size() - 1; i++)
        {
            if (m_changes[i+1].first != m_changes[i].first)
            {
                uint32_t rand = iprev + Random::rand_i(i - iprev);
                size_t dst = m_changes[rand].first;
                size_t src = m_changes[rand].second;

                m_elementBuffer[dst] = m_elementBuffer[src];
                m_colorBuffer[dst] = m_colorBuffer[src];
                m_elementBuffer[src] = element_t();
                m_colorBuffer[src] = enum_val(ElementType::NONE);

                iprev = i + 1;
            }
        }

        m_changes.clear();
    }

    // private --------------------------------------------------------------------------
    bool SandWorld::moveDown(size_t _x, size_t _y, const element_t& _e)
    {
        bool down = isEmpty(_x, _y - 1);
        if (down)
            moveElement(_x, _y, _x, _y - 1);
        return down;
    }

    // private --------------------------------------------------------------------------
    bool SandWorld::moveDownSide(size_t _x, size_t _y, const element_t& _e)
    {
        bool downLeft = isEmpty(_x - 1, _y - 1);
        bool downRight = isEmpty(_x + 1, _y - 1);
        shuffle(downLeft, downRight);
        if (downLeft)       moveElement(_x, _y, _x - 1, _y - 1);
        else if (downRight) moveElement(_x, _y, _x + 1, _y - 1);
        return downLeft || downRight;
    }

    // private --------------------------------------------------------------------------
    bool SandWorld::moveSide(size_t _x, size_t _y, const element_t& _e)
    {
        for (int i = 4; i > 0; i--)
        {
            bool left = isEmpty(_x - i, _y);
            bool right = isEmpty(_x + i, _y);    
            shuffle(left, right);
            if (left)       moveElement(_x, _y, _x - i, _y);
            else if (right) moveElement(_x, _y, _x + i, _y);
            
            if (left || right)
                return true;
        }
        return false;
    }
 
    // private --------------------------------------------------------------------------
    void SandWorld::line2D(const glm::vec2& _p0, const glm::vec2& _p1, const element_t& _e)
    {
        glm::vec2 p0 = _p0;
        glm::vec2 p1 = _p1;

        if (p0.x > p1.x)
        {
            glm::vec2 t = p0;
            p0 = p1;
            p1 = t;
        }

        glm::vec2 d = p1 - p0;

        for (size_t x = p0.x; x < p1.x; x++)
        {
            float y = p0.y + d.y + (x - p0.x) / d.x;
            size_t index = (int)y * m_size.x + x;
            
            setColor(index, (uint8_t)enum_val(_e.type));
            setElement(index, _SAND);
            //m_elementBuffer[index].props = ?
        }
    }


}