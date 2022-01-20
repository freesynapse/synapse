
#pragma once

#include "Synapse.hpp"


namespace Syn {

	class FluidConfig
	{
	public:
		static uint32_t SIM_RESOLUTION;
		static uint32_t DYE_RESOLUTION;
		static uint32_t CAPTURE_RESOLUTION;
		static float DENSITY_DISSIPATION;
		static float VELOCITY_DISSIPATION;
		static float PRESSURE;
		static uint16_t PRESSURE_ITERATIONS;
		static uint16_t CURL;
		static float SPLAT_RADIUS;
		static uint32_t SPLAT_FORCE;
		static bool SHADING;
		static bool COLORFUL;
		static uint16_t COLOR_UPDATE_SPEED;
		static bool PAUSED;
		static glm::vec3 BACK_COLOR;
		static bool TRANSPARENT;
		static bool BLOOM;
		static uint16_t BLOOM_ITERATIONS;
		static uint16_t BLOOM_RESOLUTION;
		static float BLOOM_INTENSITY;
		static float BLOOM_THRESHOLD;
		static float BLOOM_SOFT_KNEE;
		static bool SUNRAYS;
		static uint16_t SUNRAYS_RESOLUTION;
		static float SUNRAYS_WEIGHT;
	};

	// single surface
	class Surface
	{
	public:
		Surface() {};
		Surface(uint32_t _width, uint32_t _height, ColorFormat _fmt, const std::string& _name) :
			m_size(glm::ivec2(_width, _height))
		{
			m_framebuffer = API::newFramebuffer(_fmt, glm::ivec2(m_size.x, m_size.y), false, 1, true, _name);
			Renderer::get().executeRenderCommands();
			// calculate texel size
			m_texelSize = glm::vec2(1.0f / (float)m_size.x, 1.0f / (float)m_size.y);
		}

		const Ref<Framebuffer>& getFBO() 	{ return m_framebuffer;	}
		const glm::vec2& getTexSz() 			{ return m_texelSize; 	}
		const glm::ivec2& getSz()				{ return m_size; 		}

	private:
		Ref<Framebuffer> m_framebuffer;
		glm::ivec2 m_size;
		glm::vec2 m_texelSize;
	};

	// double surface
	class Slab
	{
	public:
		Slab(uint32_t _width, uint32_t _height, ColorFormat _fmt, const std::string& _name)
		{
			m_front = MakeRef<Surface>(_width, _height, _fmt, _name);
			m_back  = MakeRef<Surface>(_width, _height, _fmt, _name);
		}

		void swap()
		{
			Ref<Surface> tmp = m_back;
			m_back = m_front;
			m_front = tmp;
		}

		// accessors
		const Ref<Surface>& getSrc()			{ 	return m_front; 				}
		const Ref<Surface>& getDst()			{ 	return m_back;  				}
		const Ref<Framebuffer> getSrcFBO()		{	return m_front->getFBO(); 		}
		const Ref<Framebuffer> getDstFBO()		{	return m_back->getFBO();  		}
		void setSrc(const Ref<Surface>& _s)		{ 	m_front = _s; 					}
		void setDst(const Ref<Surface>& _s)		{ 	m_back  = _s; 					}
		const glm::vec2& getTexSz()				{	return m_front->getTexSz();		}
		const glm::ivec2& getSz()				{ 	return m_front->getSz();		}

	public:
		Ref<Surface> m_front;
		Ref<Surface> m_back;
	};


	class Fluid
	{
	public:
		Fluid(uint32_t _w, uint32_t _h);
		~Fluid();

		void step(float _dt);
		void blit(const Ref<Surface>& _surface);
		void blit(const Ref<Slab>& _slab);
		void render(uint32_t _w, uint32_t _h, float _dt);


	private:
		inline void _setVertexShaderTexelSz(const Ref<Shader>& _shader, 
											const Ref<Surface>& _surface) 
											__attribute__((always_inline))
		{
			_shader->setUniform2fv("u_texel_size", _surface->getTexSz());
		}


	private:
		// shaders
		Ref<Shader> m_divergenceShader;
		Ref<Shader> m_pressureShader;
		Ref<Shader> m_clearShader;
		Ref<Shader> m_subtractGradientShader;
		Ref<Shader> m_advectionShader;

		// surfaces (i.e. textures attached to framebuffers)
		Ref<Slab> m_velocity;
		Ref<Slab> m_density;
		Ref<Slab> m_pressure;
		Ref<Surface> m_divergence;

		// for rendering textures
		Ref<VertexArray> m_screenQuad;

	};




}
