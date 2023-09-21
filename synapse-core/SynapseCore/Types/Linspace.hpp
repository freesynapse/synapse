
#pragma once

#include "../Core.hpp"
#include "../Memory/MemoryTypes.hpp"


namespace Syn
{
    template<typename T>
    class limit_2D_t
    {
    public:
        T min_ = 0.0f;
        T max_ = 0.0f;

        limit_2D_t() = default;
        limit_2D_t(const limit_2D_t&) = default;
        limit_2D_t(T _min, T _max) :
            min_(_min), max_(_max)
        {
            min_ = min(min_, _min);
            max_ = max(max_, _max);
        }
        T range() { return max_ - min_; }
        void print(bool _newline=true) 
        { 
            std::cout << "min=" << min_ << ", max=" << max_;
            if (_newline) std::cout << '\n';
        }
    };

    //-----------------------------------------------------------------------------------
    template<typename T>
    class Linspace
    {
    public:
        T* values = nullptr;
        limit_2D_t<T> lim = { 0.0, 1.0 };
        uint32_t steps = 2;

    public:
        // naive constructor
        Linspace<T>() = default;
        // specific constructor
        Linspace<T>(T _lim_min, T _lim_max, uint32_t _steps) :
            lim(_lim_min, _lim_max), steps(_steps)
        { updateLinspace(); }
        // copy constructor
        Linspace<T>(const Linspace<T>& _other) :
            lim(_other.lim), 
            steps(_other.steps), 
            values(new T[_other.steps])
        {
            // deep copy of data
            std::copy(_other.values, _other.values + _other.steps, values);
        }
        // copy assignment operator
        Linspace<T>& operator=(const Linspace<T>& _other)
        {
            if (this != &_other)
            {
                // free exisiting data
                delete[] values;
                lim = _other.lim;
                steps = _other.steps;
                values = new T[steps];
                // deep copy of data
                std::copy(_other.values, _other.values + _other.steps, values);
            }
            return *this;
        }

        ~Linspace<T>() 
        { if (values != nullptr) { delete[] values; values = nullptr; } }

        uint32_t size() const { return steps; }
        const T range() const { return lim.max_ - lim.min_; }
        const T operator[](uint32_t _index) const { return values[_index]; }
        const T get(uint32_t _index) const { return values[_index]; }
        T* getValues() const { return values; }
        void print() const { SYN_CORE_TRACE("Linspace [ ", lim.min, "  ", lim.max, " ], n = ", steps); }

    public:
        void setLinspace(T _lim_min, T _lim_max, uint32_t _steps)
        { lim = limit_2D_t<T>(_lim_min, _lim_max); steps = _steps; updateLinspace(); }
        void setLinspace(T _lim_min, T _lim_max)
        { lim = limit_2D_t<T>(_lim_min, _lim_max); updateLinspace(); }
        void setLinspace(uint32_t _steps)
        { steps = _steps; updateLinspace(); }

    private:
        void updateLinspace()
        {
            if (values != nullptr) delete[] values;

            values = new T[steps];
            
            T step = lim.range() / (T)(steps - 1);
            T f = lim.min;
            for (int i = 0; i < steps; i++)
            {
                values[i] = f;
                f += step;
            }
        }
    };


}