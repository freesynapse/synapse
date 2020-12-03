
#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <src/renderer/buffers/Framebuffer.h>


typedef struct slab_
{
	Syn::Ref<Syn::Framebuffer> frontSurface;
	Syn::Ref<Syn::Framebuffer> backSurface;
} Slab;


class Fluid
{
public:
	static Slab createSlab(uint32_t _width, uint32_t _height, const Syn::FramebufferFormat& _fmt);
	static Syn::Ref<Syn::Framebuffer> createSurface(uint32_t _width, uint32_t _height, const Syn::FramebufferFormat& _fmt);

	static void clearSurface(const Syn::Ref<Syn::Framebuffer>& _frame_buffer, const glm::vec4& _color=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	static void swapSurfaces(Slab* _slab);

};
