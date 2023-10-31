#pragma once


#include "../pch.hpp"

#include "Layer.hpp"


namespace Syn {


    class Layerstack
    {
    public:
	Layerstack();
	~Layerstack();

	void pushLayer(Layer* layer);
	void popLayer(Layer* layer);
	void pushOverlay(Layer* layer);
	void popOverlay(Layer* layer);

	// accessors
	std::vector<Layer*>::iterator begin() { return m_layers.begin(); }
	std::vector<Layer*>::iterator end() { return m_layers.end(); }


    private:
	std::vector<Layer*> m_layers;
	size_t m_insertIndex = 0;
    };


}
