
#include "fluids.h"

#include "src/Core.h"
#include "src/renderer/Renderer.h"


// -- FLUID SPECIFIC FUNCTIONS : defined in fluids.h --
Slab Fluid::createSlab(uint32_t _width, uint32_t _height, uint32_t _num_compononents)
{
	Slab slab;
	slab.frontSurface = createSurface(_width, _height, _num_compononents, true);
	slab.backSurface = createSurface(_width, _height, _num_compononents, true);
	return slab;
}

Surface Fluid::createSurface(uint32_t _width, uint32_t _height, uint32_t _num_compononents, bool _use_half_floats)
{
	Surface surface;
	surface.desc.width = _width;
	surface.desc.height = _height;
	surface.desc.numComponents = _num_compononents;
	surface.desc.useHalfFloats = _use_half_floats;

	SYN_RENDER_1(surface, {
		glGenFramebuffers(1, &surface.fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, surface.fboID);

		glGenTextures(1, &surface.textureID);
		glBindTexture(GL_TEXTURE_2D, surface.textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (surface.desc.useHalfFloats)
		{
			switch (surface.desc.numComponents)
			{
				case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, surface.desc.width, surface.desc.height, 0, GL_RED, GL_HALF_FLOAT, 0); break;
				case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, surface.desc.width, surface.desc.height, 0, GL_RG, GL_HALF_FLOAT, 0); break;
				case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, surface.desc.width, surface.desc.height, 0, GL_RGB, GL_HALF_FLOAT, 0); break;
				case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, surface.desc.width, surface.desc.height, 0, GL_RGBA, GL_HALF_FLOAT, 0); break;
				default: SYN_CORE_FATAL_ERROR("Illegal number of components (surface.desc.numComponents).");
			}
		}
		else
		{
			switch (surface.desc.numComponents)
			{
				case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, surface.desc.width, surface.desc.height, 0, GL_RED, GL_FLOAT, 0); break;
				case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, surface.desc.width, surface.desc.height, 0, GL_RG, GL_FLOAT, 0); break;
				case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, surface.desc.width, surface.desc.height, 0, GL_RGB, GL_FLOAT, 0); break;
				case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, surface.desc.width, surface.desc.height, 0, GL_RGBA, GL_FLOAT, 0); break;
				default: SYN_CORE_FATAL_ERROR("Illegal number of components (surface.desc.numComponents).");
			}
		}

		GLuint colorBuffer;
		glGenRenderbuffers(1, &colorBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, surface.textureID, 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			SYN_CORE_FATAL_ERROR("Framebuffer not complete.");
		}

		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	});

	Syn::Renderer::executeRenderCommands();

	return surface;
	
}




