
#pragma once

#include "Synapse/Types/Linspace.hpp"

#include "Synapse/Core.hpp"
#include "Synapse/Memory/MemoryTypes.hpp"

#include <glm/glm.hpp>

namespace Syn
{
    template<typename T>
    class Meshgrid
    {
    protected:
        /*
         * For rendering, m_z and m_y correspond to X and Z coordinates, respectively. Naming 
         * convention based on mathematical standard, where x and y are horizontal, orthogonal axes 
         * and z is vertical.
         */
        Linspace<T> m_x;
        Linspace<T> m_y;
        // Technically not part of the meshgrid, implemented in inherited classes
        //T* m_z = nullptr;
        //uint32_t m_size = 0;    // no. elements in m_z

    public:
        Meshgrid() = default;
        Meshgrid(const Meshgrid&) = default;
        Meshgrid(const Linspace<T>& _x, const Linspace<T>& _y) :
            m_x(_x), m_y(_y)
        {}
        ~Meshgrid() = default;
        //~Meshgrid() { if (m_z != nullptr) delete[] m_z; }

        /* The X axis (Linspace) */
        const Linspace<T>& getXAxis() const { return m_x; }
        /* The Y axis (Linspace) */
        const Linspace<T>& getYAxis() const { return m_y; }
        // See above
        ///* Z values (computed) */
        //T* getZValues()  const { return m_z;    }
        ///* Number of Z values */
        //uint32_t zsize()           const { return m_size; }

    private:
        virtual void updateCoordinates() {}
    
    };

}
