
#include <Synapse/Input/InputManager.hpp>
#include <Synapse/Debug/Log.hpp>
#include <Synapse/Debug/Profiler.hpp>
#include <Synapse/Utils/Random/Random.hpp>
#include <Synapse/Utils/Timer/TimeStep.hpp>

#include "SandWorld_v2.hpp"


namespace Syn
{
    //----------------------------------------------------------------------------------------------
    SandWorld::SandWorld(uint32_t _px_x, uint32_t _px_y, uint32_t _px_scale, const Ref<Shader>& _shader)
    {
        m_size = glm::ivec2(_px_x, _px_y);
        m_pxScale = _px_scale;
        m_shader = _shader;

        SYN_CORE_TRACE("Initializing simulation.");

        // Safety checks
        if (_px_x == 0 || _px_y == 0)
        {
            m_size = Renderer::getViewport();
            SYN_CORE_TRACE("No viewport provided, using default viewport.");
        }
        if (_px_scale == 0)
        {
            m_pxScale = 1;
            SYN_CORE_TRACE("No pixel scale provided, setting to 1.");
        }

        // enforce the pixel scale to be a multiple of 2
        int msb = 0;
        uint32_t n = m_pxScale;
        while (n != 0)
        {
            n = n / 2;
            msb++;
        }
        m_pxScale = (1 << (msb - 1));
        SYN_CORE_TRACE("Pixel scale = ", m_pxScale);

        // Set pixel size in the texture based on the viewport size and the pixel scaling 
        // factor.
        m_size = { m_size.x / m_pxScale, m_size.y / m_pxScale };
        m_bufferSz = m_size.x * m_size.y;
        SYN_CORE_TRACE("Buffer dim: [ ", m_size.x, " x ", m_size.y, " ] (", m_bufferSz, ").");

        /* Initialize the grid and color buffer.
            * If the color of the element was to be included in the element_t struct, the color of
            * the element will then be interleaved with the type and properties of the element. 
            * By using two buffers with the same indexing scheme, we don't need to step through 
            * the element_t buffer every frame to get the data for texture upload. 
            * 
            * The next point: Since we are using fixed colors, perhaps a indexer in the fragment
            * shader can be used, so that only a uint8_t can be used as texture for upload every
            * frame (and thus be uploaded through m_texture->setData()).
            */
        m_particleBuffer = new particle_t[m_bufferSz];
        m_colorBuffer = new color_t[m_bufferSz];
        for (uint32_t y = m_size.y - 1; y > 0; y--)
            for (uint32_t x = m_size.x - 1; x > 0; x--)
            {
                uint32_t index = getIndex(x, y);
                m_particleBuffer[index] = s_particleEmpty;
                m_colorBuffer[index] = s_particleEmpty.color;
            }

        uint32_t index = (m_size.y / 2) * m_size.x + m_size.x / 2;
        m_particleBuffer[index] = s_particleWater;
        m_colorBuffer[index] = s_particleWater.color;
        index = getIndex(m_size.x/2, 1);
        m_particleBuffer[index] = s_particleSand;
        m_colorBuffer[index] = s_particleSand.color;

        // set up the rendering objects
        m_texture = API::newTexture2D(m_size.x, m_size.y, ColorFormat::RGBA8);
        m_screenQuad = MeshCreator::createShapeViewportQuad();
    }
    //----------------------------------------------------------------------------------------------
    SandWorld::~SandWorld()
    {
        delete[] m_particleBuffer;
        delete[] m_colorBuffer;
    }
    //----------------------------------------------------------------------------------------------
    void SandWorld::onUpdate(float _dt)
    {
        static uint32_t n = 0;
        /* Simulate bootom up to avoid using double buffering */
        for (uint32_t y = m_size.y - 1; y > 0; y--)
        {
            for (uint32_t x = m_size.x - 1; x > 0; x--)
            {
                if (isEmpty(x, y))
                    continue;
                particle_t& p_here  = getParticle(getIndex(x, y));
                particle_t& p_below = getParticle(getIndex(x, y+1));

                if (n == 0)
                    debugParticle(p_here);

                // first check below
                if (p_here.movePattern & MOVE_DOWN && isEmpty(x, y+1))
                {
                    moveDown(x, y);
                }
                // solid particles moving down will swap places with fluids
                else if (p_here.movePattern & MOVE_DOWN && isSolid(p_here) && isFluid(x, y+1)) 
                {
                    swapParticles(x, y, x, y+1);
                }
                
                // solids (for now)
                else if (p_here.movePattern & MOVE_DOWN_SIDE) 
                {
                    moveDownSide(x, y);
                }

                // fluid particles move to the side and not diagnonally
                else if (p_here.movePattern & MOVE_SIDE)
                {
                    moveSide(x, y);
                }

            }
        }
        n++;

        handleInput();
    }
    //----------------------------------------------------------------------------------------------
    void SandWorld::onRender()
    {
         SYN_PROFILE_FUNCTION();

        // upload updated texture
        m_texture->setData(m_colorBuffer, sizeof(color_t) * m_bufferSz);

        // Bind shader, use texture (screen buffer) and render
        m_shader->enable();
        m_texture->bind();
        m_screenQuad->renderNDC();
   }
    //----------------------------------------------------------------------------------------------
    void SandWorld::handleInput()
    {
        glm::vec2 mousePos = InputManager::get_mouse_position();
        glm::ivec2 offset = Renderer::getImGuiViewportOffset();
        static glm::vec2 scale = Renderer::getViewportF() / glm::vec2(m_size.x, m_size.y);

        if (InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1))
        {
            glm::vec2 pos = mousePos - glm::vec2(offset.x, offset.y);
            pos /= scale;

            // use m_selectedParticle to draw
            for (int i = -m_brushSize; i < m_brushSize; i++)
            {
                for (int j = -m_brushSize; j < m_brushSize; j++)
                {
                    if (inBounds(pos.x+i, pos.y+j) && (i*i + j*j < m_brushSize*m_brushSize))
                    {
                        //
                        uint32_t index = getIndex(pos.x+i, pos.y+j);
                        m_particleBuffer[index] = m_selectedBrush;
                        m_colorBuffer[index] = m_selectedBrush.color;
                    }
                }
            }
        }
    }
    //----------------------------------------------------------------------------------------------
    bool SandWorld::moveDown(uint32_t _x, uint32_t _y)
    {
        // already checked that the space below is empty
        if (inBounds(_x, _y))
        {
            moveParticle(_x, _y, _x, _y+1);
            return true;
        }
        return false;
    }
    //----------------------------------------------------------------------------------------------
    bool SandWorld::moveDownSide(uint32_t _x, uint32_t _y)
    {
        bool l = isEmpty(_x+1, _y+1);
        bool r = isEmpty(_x-1, _y+1);
        if (l && r)
        {
            l = Random::rand_b();
            r = !l;
        }
        if (l)      moveParticle(_x, _y, _x+1, _y+1);
        else if (r) moveParticle(_x, _y, _x-1, _y+1);

        return l || r;
    }
    //----------------------------------------------------------------------------------------------
    bool SandWorld::moveSide(uint32_t _x, uint32_t _y)
    {
        bool l = isEmpty(_x+1, _y);
        bool r = isEmpty(_x-1, _y);
        if (l && r)
        {
            l = Random::rand_b();
            r = !l;
        }
        return false;
        if (l)      moveParticle(_x, _y, _x+1, _y);
        else if (r) moveParticle(_x, _y, _x-1, _y);
        return l || r;
    }


}