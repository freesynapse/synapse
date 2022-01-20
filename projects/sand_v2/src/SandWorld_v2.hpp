
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
    // particle types
    #define PARTICLE_EMPTY  0b00000000000000000000000000000000
    #define PARTICLE_SAND   0b00000000000000000000000000000001
    #define PARTICLE_WATER  0b00000000000000000000000000000010
    #define PARTICLE_STONE  0b00000000000000000000000000000100
    #define PARTICLE_DIRT   0b00000000000000000000000000001000

    // particle (chemical) states
    #define PARTICLE_SOLID  (PARTICLE_SAND | PARTICLE_STONE | PARTICLE_DIRT)
    #define PARTICLE_FLUID  (PARTICLE_WATER)
    //#define PARTICLE_GAS

    // update patterns
    #define MOVE_NONE       0b00000
    #define MOVE_DOWN       0b00001
    #define MOVE_DOWN_SIDE  0b00010
    #define MOVE_SIDE       0b00100
    #define MOVE_UP         0b01000
    #define MOVE_UP_SIDE    0b10000

    // particle move patterns
    #define MOVE_PATTERN_NONE   (MOVE_NONE)
    #define MOVE_PATTERN_SOLID  (MOVE_DOWN | MOVE_DOWN_SIDE)
    #define MOVE_PATTERN_FLUID  (MOVE_DOWN | MOVE_SIDE)
    #define MOVE_PATTERN_GAS    (MOVE_UP   | MOVE_SIDE)

    // basic color struct, used in texture
    typedef struct color_t
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;     //  4 bytes
    } color_t;

    // particle struct, contains all needed parts
    typedef struct particle_t
    {
        uint32_t type;          //  4 bytes
        uint32_t movePattern;   //  4 bytes
        float life;             //  4 bytes
        glm::vec2 velocity;     //  8 bytes
        color_t color;          //  4 bytes
        bool hasBeenUpdated;    //  4 bytes

        particle_t() 
            {}
        particle_t(uint32_t _type, uint32_t _move, float _life, glm::vec2 _vel, color_t _color, bool _updated) :
            type(_type), movePattern(_move), life(_life), velocity(_vel), color(_color), hasBeenUpdated(_updated)
            {}

    } particle_t;               // 28 bytes

    // 'global' materials and preset as references
    static particle_t s_particleEmpty(PARTICLE_EMPTY, MOVE_PATTERN_NONE,  0.0f, glm::vec2(0), {  0,   0,   0, 255}, false);
    static particle_t s_particleSand (PARTICLE_SAND,  MOVE_PATTERN_SOLID, 0.0f, glm::vec2(0), {235, 186,  52, 255}, false);
    static particle_t s_particleWater(PARTICLE_WATER, MOVE_PATTERN_FLUID, 0.0f, glm::vec2(0), { 20, 100, 170, 200}, false);
    static particle_t s_particleStone(PARTICLE_STONE, MOVE_PATTERN_NONE,  0.0f, glm::vec2(0), {120, 110, 120, 255}, false);
    static particle_t s_particleDirt (PARTICLE_DIRT,  MOVE_PATTERN_SOLID, 0.0f, glm::vec2(0), {102,  51,   0, 255}, false);


    //----------------------------------------------------------------------------------------------
    /* The 'world' class.
     */
    class SandWorld
    {
    public:
        // Constructor / destrictor
        SandWorld(uint32_t _px_x, uint32_t _px_y, uint32_t _px_scale, const Ref<Shader>& _shader);
        ~SandWorld();

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

        /* Resets the simulation, not world parameters.
         */
        void reset() {}

    private:
        bool moveDown(uint32_t _x, uint32_t _y);
        bool moveDownSide(uint32_t _x, uint32_t _y);
        bool moveSide(uint32_t _x, uint32_t _y);

        void debugParticle(const particle_t& _p)
        {
            std::cout << "particle debug:\n";
            printf("type = %d\nmove = %d\nlife = %.1f\n", _p.type, _p.movePattern, _p.life);
            printf("color = [ %d  %d  %d  %d ]\n", _p.color.r, _p.color.g, _p.color.b, _p.color.a);
            printf("isSolid ? %s\n", isSolid(_p) ? "TRUE" : "FALSE");
            printf("isFluid ? %s\n\n", isFluid(_p) ? "TRUE" : "FALSE");
        }

    private:
        /* Small helper macros */
        uint32_t getIndex(uint32_t _x, uint32_t _y) { return _y * m_size.x + _x; }

        bool inBounds(uint32_t _x, uint32_t _y) { return _x > 0 && _x < m_size.x && _y > 0 && _y < m_size.y; }

        bool isEmpty(uint32_t _x, uint32_t _y)  { return inBounds(_x, _y) && m_particleBuffer[getIndex(_x, _y)].type == PARTICLE_EMPTY; }
        bool isFluid(uint32_t _x, uint32_t _y)  { return inBounds(_x, _y) && m_particleBuffer[getIndex(_x, _y)].movePattern == MOVE_PATTERN_FLUID; }
        bool isFluid(const particle_t& _p)      { return _p.movePattern == MOVE_PATTERN_FLUID; }
        bool isSolid(uint32_t _x, uint32_t _y)  { return inBounds(_x, _y) && m_particleBuffer[getIndex(_x, _y)].movePattern == MOVE_PATTERN_SOLID; }
        bool isSolid(const particle_t& _p)      { return _p.movePattern == MOVE_PATTERN_SOLID; }
        
        particle_t& getParticle(uint32_t _x, uint32_t _y) { return m_particleBuffer[getIndex(_x, _y)]; }
        particle_t& getParticle(uint32_t _index) { return m_particleBuffer[_index]; }
        
        void setParticle(uint32_t _x, uint32_t _y, const particle_t& _p) { setParticle(getIndex(_x, _y), _p); }
        void setParticle(uint32_t _index, const particle_t& _p) { m_particleBuffer[_index] = _p; m_colorBuffer[_index] = _p.color; }

        void moveParticle(uint32_t _x_src, uint32_t _y_src, uint32_t _x_dst, uint32_t _y_dst)
        {
            uint32_t indexSrc = getIndex(_x_src, _y_src);
            uint32_t indexDst = getIndex(_x_dst, _y_dst);
            
            m_particleBuffer[indexDst]  = m_particleBuffer[indexSrc];
            m_colorBuffer[indexDst]     = m_colorBuffer[indexSrc];
            m_particleBuffer[indexSrc]  = s_particleEmpty;
            m_colorBuffer[indexSrc]     = s_particleEmpty.color;
        }

        void swapParticles(uint32_t _x_src, uint32_t _y_src, uint32_t _x_dst, uint32_t _y_dst)
        {
            //printf("swapping\n");
            uint32_t indexSrc = getIndex(_x_src, _y_src);
            uint32_t indexDst = getIndex(_x_dst, _y_dst);
            //printf("before swap:\n(%d, %d):\n", _x_src, _y_src);
            //debugParticle(m_particleBuffer[indexSrc]);
            //printf("(%d, %d):\n", _x_dst, _y_dst);
            //debugParticle(m_particleBuffer[indexDst]);
            particle_t tmp = m_particleBuffer[indexDst];
            m_particleBuffer[indexDst]  = m_particleBuffer[indexSrc];
            m_colorBuffer[indexDst]     = m_colorBuffer[indexSrc];
            m_particleBuffer[indexSrc]  = tmp;
            m_colorBuffer[indexSrc]     = tmp.color;
            //printf("after swap:\n(%d, %d):\n", _x_src, _y_src);
            //debugParticle(m_particleBuffer[indexSrc]);
            //printf("(%d, %d):\n", _x_dst, _y_dst);
            //debugParticle(m_particleBuffer[indexDst]);
        }


    private:
        /* Size of the world, which is the size of the viewport divided by the pixel scale. */
        glm::ivec2 m_size = { 0, 0 };
        uint32_t m_pxScale = 1;
        /* We use two separate buffers; one to store the element types and one to store the colors,
           which is the texture uploaded every frame. In this way, we can skip stepping through the
           element buffer every frame just to update the texture. */
        particle_t* m_particleBuffer;
        color_t* m_colorBuffer;
        uint32_t m_bufferSz;
        /* Physics and stuff */
        float m_gravity = 9.8f;
        /* UI stuff */
        uint8_t m_brushSize = 8;
        particle_t m_selectedBrush = s_particleSand;
        /* Synapse objects */
        Ref<Texture2D> m_texture = nullptr;
        Ref<Shader> m_shader = nullptr;
        Ref<MeshShape> m_screenQuad = nullptr;


    };

    
}
