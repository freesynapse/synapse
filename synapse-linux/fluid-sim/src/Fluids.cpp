
#include "Fluids.h"

#include <Synapse.h>


// -- FLUID SPECIFIC FUNCTIONS : defined in fluids.h --
Slab Fluid::createSlab(uint32_t _width, uint32_t _height, const Syn::FramebufferFormat& _fmt)
{
	Slab slab;
	slab.frontSurface = createSurface(_width, _height, _fmt);
	slab.backSurface = createSurface(_width, _height, _fmt);
	return slab;
}


//-----------------------------------------------------------------------------------
Syn::Ref<Syn::Framebuffer> Fluid::createSurface(uint32_t _width, uint32_t _height, const Syn::FramebufferFormat& _fmt)
{

	Syn::Ref<Syn::Framebuffer> frameBuffer = Syn::MakeRef<Syn::Framebuffer>(_width, _height, _fmt, false);
	Syn::Renderer::executeRenderCommands();

	frameBuffer->clearFramebuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), GL_COLOR_BUFFER_BIT);
	Syn::Renderer::executeRenderCommands();

	return frameBuffer;
	
}


//-----------------------------------------------------------------------------------
void Fluid::clearSurface(const Syn::Ref<Syn::Framebuffer>& _buffer, const glm::vec4& _color)
{
	_buffer->clearFramebuffer(_color, GL_COLOR_BUFFER_BIT);

}


//-----------------------------------------------------------------------------------
void Fluid::swapSurfaces(Slab* _slab)
{
	Syn::Ref<Syn::Framebuffer> tmp = _slab->backSurface;
	_slab->backSurface = _slab->frontSurface;
	_slab->frontSurface = tmp;
}


//-----------------------------------------------------------------------------------
// Shader computations
//-----------------------------------------------------------------------------------
void Fluid::advect(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _source, Syn::Ref<Syn::Framebuffer> _obstacles, float _dissipation)
{

}

//-----------------------------------------------------------------------------------
void Fluid::jacobi(Syn::Ref<Syn::Framebuffer> _pressure, Syn::Ref<Syn::Framebuffer> _divergence, Syn::Ref<Syn::Framebuffer> _obstacles, Syn::Ref<Syn::Framebuffer> _dest)
{

}

//-----------------------------------------------------------------------------------
void Fluid::subtractGradient(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _pressure, Syn::Ref<Syn::Framebuffer> _obstacles, Syn::Ref<Syn::Framebuffer> _dest)
{

}

//-----------------------------------------------------------------------------------
void Fluid::computeDivergence(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _obstacles, Syn::Ref<Syn::Framebuffer> _dest)
{

}

//-----------------------------------------------------------------------------------
void Fluid::applyImpulse(Syn::Ref<Syn::Framebuffer> _dest, const glm::vec2& _position, float _value)
{

}

//-----------------------------------------------------------------------------------
void Fluid::applyBuoyancy(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _temperature, Syn::Ref<Syn::Framebuffer> _density, Syn::Ref<Syn::Framebuffer> _dest)
{

}




