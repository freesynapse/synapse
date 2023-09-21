
#include "../pch.hpp"

#include "LayerStack.hpp"
#include "Core.hpp"


namespace Syn {


    Layerstack::Layerstack()
    {
    }


    //-----------------------------------------------------------------------------------
    Layerstack::~Layerstack()
    {
        for (Layer* layer : m_layers)
        {
            delete layer;
        }
    }


    //-----------------------------------------------------------------------------------
    void Layerstack::pushLayer(Layer* _layer)
    {
        m_layers.emplace(m_layers.begin() + m_insertIndex, _layer);
        _layer->onAttach();
        m_insertIndex++;
    }


    //-----------------------------------------------------------------------------------
    void Layerstack::popLayer(Layer* _layer)
    {
        auto it = std::find(m_layers.begin(), m_layers.begin() + m_insertIndex, _layer);
        if (it != m_layers.end())
        {
            _layer->onDetach();
            m_layers.erase(it);
            m_insertIndex--;
        }
    }


    //-----------------------------------------------------------------------------------
    void Layerstack::pushOverlay(Layer* _layer)
    {
        m_layers.push_back(_layer);
        _layer->onAttach();
    }


    //-----------------------------------------------------------------------------------
    void Layerstack::popOverlay(Layer* _layer)
    {
        auto it = std::find(m_layers.begin() + m_insertIndex, m_layers.end(), _layer);
        if (it != m_layers.end())
        {
            _layer->onDetach();
            m_layers.erase(it);
        }
    }


}
