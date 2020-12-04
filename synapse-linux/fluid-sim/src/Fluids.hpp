
#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <Synapse/Renderer/Buffers/Framebuffer.hpp>

typedef struct slab_
{
	Syn::Ref<Syn::Framebuffer> frontSurface;
	Syn::Ref<Syn::Framebuffer> backSurface;
} Slab;


class Fluid
{
public:
	static Slab createSlab(uint32_t _width, uint32_t _height, const Syn::ColorFormat& _fmt);
	static Syn::Ref<Syn::Framebuffer> createSurface(uint32_t _width, uint32_t _height, const Syn::ColorFormat& _fmt);

	static void clearSurface(const Syn::Ref<Syn::Framebuffer>& _frame_buffer, const glm::vec4& _color=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	static void swapSurfaces(Slab* _slab);

	// computations
	static void advect(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _source, Syn::Ref<Syn::Framebuffer> _obstacles, float _dissipation);
	static void jacobi(Syn::Ref<Syn::Framebuffer> _pressure, Syn::Ref<Syn::Framebuffer> _divergence, Syn::Ref<Syn::Framebuffer> _obstacles, Syn::Ref<Syn::Framebuffer> _dest);
	static void subtractGradient(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _pressure, Syn::Ref<Syn::Framebuffer> _obstacles, Syn::Ref<Syn::Framebuffer> _dest);
	static void computeDivergence(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _obstacles, Syn::Ref<Syn::Framebuffer> _dest);
	static void applyImpulse(Syn::Ref<Syn::Framebuffer> _dest, const glm::vec2& _position, float _value);
	static void applyBuoyancy(Syn::Ref<Syn::Framebuffer> _velocity, Syn::Ref<Syn::Framebuffer> _temperature, Syn::Ref<Syn::Framebuffer> _density, Syn::Ref<Syn::Framebuffer> _dest);
};
