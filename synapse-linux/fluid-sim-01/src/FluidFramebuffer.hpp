#pragma once

#include <Synapse/Renderer/Buffers/Framebuffer.hpp>


using namespace Syn;


class FluidFramebuffer : public FramebufferBase
{
public:
	FluidFramebuffer(uint32_t _width, 
					 uint32_t _height, 
					 const ColorFormat& _format, 
					 bool _update_on_resize,
					 const std::string& _name);

	virtual void resize(uint32_t _width, uint32_t _height) override;

};


