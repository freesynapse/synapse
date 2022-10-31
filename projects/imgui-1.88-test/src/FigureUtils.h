
#include <math.h>


/* Source : https://gist.github.com/igodorogea/4f42a95ea31414c3a755a8b202676dfd
 */
struct nice_scale
{
    uint32_t m_max_ticks;
    float    m_tick_spacing;
    float    m_range;
    float    m_lower_bound;
    float    m_upper_bound;

    //
    nice_scale(float _lower_bound, float _upper_bound, size_t _max_ticks=4) :
        m_max_ticks(_max_ticks), m_lower_bound(_lower_bound), m_upper_bound(_upper_bound)
    { 
        calculate();
    }
    //
    void set_bounds(float _lower_bound, float _upper_bound) 
    { 
        m_lower_bound = _lower_bound;
        m_upper_bound = _upper_bound;
        calculate();
    }
    //
    void calculate()
    {
        m_range = nice_num(m_upper_bound - m_lower_bound, false);
        m_tick_spacing = nice_num(m_range / static_cast<float>(m_max_ticks - 1), true);
        m_lower_bound = floor(m_lower_bound / m_tick_spacing) * m_tick_spacing;
        m_upper_bound = ceil(m_upper_bound / m_tick_spacing) * m_tick_spacing;
    }
    //
    float nice_num(float _range, bool _round)
    {
        float exponent = floor(log10(_range));
        float fraction  = _range / pow(10.0f, exponent);
        float nice_fraction;

        if (_round)
        {
            if      (fraction <  1.5f)  nice_fraction =  1;
            else if (fraction <  3.0f)  nice_fraction =  2;
            else if (fraction <  7.0f)  nice_fraction =  5;
            else                        nice_fraction = 10;
        }
        else
        {
            if      (fraction <= 1.0f)  nice_fraction =  1;
            else if (fraction <= 2.0f)  nice_fraction =  2;
            else if (fraction <= 5.0f)  nice_fraction =  5;
            else                        nice_fraction = 10;
        }

        return nice_fraction * pow(10.0f, exponent);
    }

};


