
#pragma once

#include <Synapse/Core.hpp>
#include <Synapse/Renderer/Renderer.hpp>
#include <Synapse/Renderer/Shader/Shader.hpp>
#include <Synapse/Renderer/MeshCreator.hpp>
#include <Synapse/Renderer/Material/Texture2D.hpp>
#include <Synapse/API/OpenGLBindings.hpp>
#include <Synapse/Debug/Log.hpp>

#include <Synapse/Utils/Random/Random.hpp>


namespace Syn
{
    enum class ElementType
    {
        SAND = 0,
        WATER,
        ROCK,
        NONE
    };

    enum class ElementProperties
    {
        NONE            = 0b00000000,
        MOVE_DOWN       = 0b00000001,
        MOVE_SIDE       = 0b00000010,
        MOVE_DOWN_SIDE  = 0b00000100
    };

    // Need to decclare logical operators to work with enum classes
    inline ElementProperties operator | (ElementProperties _a, ElementProperties _b)
        { return ElementProperties(enum_val(_a) | enum_val(_b)); }
    inline auto operator & (ElementProperties _a, ElementProperties _b)
        { return enum_val(_a) & enum_val(_b); }

    // the contents stored for each 'pixel'
    struct element_t
    {
        ElementType type = ElementType::NONE;
        ElementProperties props = ElementProperties::NONE;
    };

    /* Preset elements.
     */
    const element_t _SAND  = { ElementType::SAND,  ElementProperties::MOVE_DOWN | 
                                                   ElementProperties::MOVE_DOWN_SIDE };
    const element_t _WATER = { ElementType::WATER, ElementProperties::MOVE_DOWN |
                                                   ElementProperties::MOVE_SIDE };
    const element_t _ROCK  = { ElementType::ROCK,  ElementProperties::NONE };
    const element_t _NONE  = { ElementType::NONE,  ElementProperties::NONE };


    /* The 'world' class.
     */
    class SandWorld
    {
    public:
        /* Constructor / destructor
         */

        /* Takes as arguments the _px_x and _px_y dimensions (row-major order), the 'pixel' scale 
         * and the shader used for rendering.
         */
        SandWorld(uint32_t _px_x=0, 
                  uint32_t _px_y=0, 
                  uint32_t _px_scale=0, 
                  const Ref<Shader>& _shader=nullptr) :
            m_size({ _px_x, _px_y }), 
            m_pxScale(_px_scale), 
            m_shader(_shader)
        {
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
            uint32_t n = _px_scale;
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
            m_elementBufferSz = m_size.x * m_size.y;
            SYN_CORE_TRACE("Buffer dim: [ ", m_size.x, " x ", m_size.y, " ] (", m_elementBufferSz, ").");

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
            m_elementBuffer = new element_t[m_elementBufferSz];
            m_colorBuffer = new uint8_t[m_elementBufferSz];
            memset(m_colorBuffer, enum_val(ElementType::NONE), m_elementBufferSz * sizeof(uint8_t));

            // set up the rendering objects
            m_texture = API::newTexture2D(m_size.x, m_size.y, ColorFormat::R8);
            m_screenQuad = MeshCreator::createShapeViewportQuad();
        }

        ~SandWorld() 
        {
            delete[] m_elementBuffer;
            delete[] m_colorBuffer;
        }


        /* Accessors
         */


        /* Member functions
         */

        /* Loop through all elements of the array and update all particles positions. In effect 
         * this equates to rendering the particles; the onUpdate function only renders the texture
         * to screen.
         */
        void onUpdate(float _dt);

        /* Copies the rendering buffer to texture and renders the uploaded texture.
         */
        void onRender();

        /* For control and drawing.
         */
        void handleInput();

    private:
        /* Commit the moves stored in m_changes */
        void commitMoves();

        /* Draws a line in the buffers from _p0 to _p1 of type _type. Changes both the color buffer
         * and the type buffer. Uses Bresenham's algorithm.
         */
        void line2D(const glm::vec2& _p0, const glm::vec2& _p1, const element_t& _e);

        /* Accessors for retrieving and setting buffer values based on index or coordinates 
         */
        size_t getIndex(size_t _x, size_t _y) 
            { return _y * m_size.x + _x; }
        const element_t& getElement(size_t _x, size_t _y) 
            { return m_elementBuffer[getIndex(_x, _y)]; }
        const element_t& getElement(size_t _index)        
            { return m_elementBuffer[_index]; }
        const uint8_t& getColor(size_t _x, size_t _y)     
            { return m_colorBuffer[getIndex(_x, _y)]; }
        const uint8_t& getColor(size_t _index)            
            { return m_colorBuffer[_index]; }

        void setElement(size_t _x, size_t _y, const element_t& _e)
            { m_elementBuffer[getIndex(_x, _y)] = _e; }
        void setElement(size_t _index, const element_t& _e)
            { m_elementBuffer[_index] = _e; }
        void setColor(size_t _x, size_t _y, uint8_t _c)
            { m_colorBuffer[getIndex(_x, _y)] = _c; }
        void setColor(size_t _index, uint8_t _c)
            { m_colorBuffer[_index] = _c; }

        /* Marking an element move, stored in the m_changes vector, befor commitMoves(). */
        void moveElement(size_t _x_src, size_t _y_src, size_t _x_dst, size_t _y_dst)
            { m_changes.emplace_back(getIndex(_x_dst, _y_dst), getIndex(_x_src, _y_src)); }
        bool moveDown(size_t _x, size_t _y, const element_t& _e);
        bool moveDownSide(size_t _x, size_t _y, const element_t& _e);
        bool moveSide(size_t _x, size_t _y, const element_t& _e);

        /* Basic checking functions, used a lot so try to keep optimized.
         */
        bool inBounds(size_t _x, size_t _y) { return _x >= 0 && _x < m_size.x && \
                                                     _y >= 0 && _y < m_size.y; }
        bool isEmpty(size_t _x, size_t _y)  { return inBounds(_x, _y) && \
                                                     getElement(_x, _y).type == ElementType::NONE; }
        inline void shuffle(bool& _a, bool& _b)
        {
            if (_a && _b)
            {
                _a = Random::rand_f_r(-1.0f, 1.0f) > 0.0f;
                _b = !_a;
            }
        }

    public:
        /* UI functions */
        const char* getSelectedBrushStr() 
        { 
            switch(m_selectedBrush.type)
            {
                case ElementType::NONE:  return "AIR";   break;
                case ElementType::SAND:  return "SAND";  break;
                case ElementType::WATER: return "WATER"; break;
                case ElementType::ROCK:  return "ROCK";  break;
            }
        }
        void setSelectedBrush(const element_t& _e) { m_selectedBrush = _e; }
         

    private:
        /* World variables
         */
        /* Size of the world, which is the size of the viewport divided by the pixel scale. */
        glm::ivec2 m_size = { 0, 0 };
        uint32_t m_pxScale = 1;
        /* We use two separate buffers; one to store the element types and one to store the colors,
           which is the texture uploaded every frame. In this way, we can skip stepping through the
           element buffer every frame just to update the texture. */
        element_t* m_elementBuffer = nullptr;
        uint8_t* m_colorBuffer = nullptr;
        uint32_t m_elementBufferSz = 0;
        /* All moves are stored in a vector with source and destination of the move. 
         * After all moves are stored, we will execute them at random. */
        std::vector<std::pair<size_t, size_t>> m_changes;

        /* UI variables 
         */
        int m_brushRadius = 10;
        element_t m_selectedBrush = _SAND;

        /* Synapse objects
         */
        Ref<Texture2D> m_texture = nullptr;
        Ref<Shader> m_shader = nullptr;
        Ref<MeshShape> m_screenQuad = nullptr;

    };

    
}
