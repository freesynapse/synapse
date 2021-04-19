
#include "Fluid.hpp"


namespace Syn {

	// global config (static class)
	uint32_t 	FluidConfig::SIM_RESOLUTION 		= 128;
	uint32_t 	FluidConfig::DYE_RESOLUTION 		= 1024;
	uint32_t 	FluidConfig::CAPTURE_RESOLUTION 	= 512;
	float 		FluidConfig::DENSITY_DISSIPATION 	= 1;
	float 		FluidConfig::VELOCITY_DISSIPATION 	= 0.2f;
	float 		FluidConfig::PRESSURE 				= 0.8f;
	uint16_t 	FluidConfig::PRESSURE_ITERATIONS 	= 20;
	uint16_t 	FluidConfig::CURL 					= 30;
	float 		FluidConfig::SPLAT_RADIUS 			= 0.25f;
	uint32_t 	FluidConfig::SPLAT_FORCE 			= 6000;
	bool 		FluidConfig::SHADING 				= true;
	bool 		FluidConfig::COLORFUL 				= true;
	uint16_t 	FluidConfig::COLOR_UPDATE_SPEED 	= 10;
	bool 		FluidConfig::PAUSED 				= false;
	glm::vec3	FluidConfig::BACK_COLOR 			= glm::vec3(0.0f);
	bool 		FluidConfig::TRANSPARENT 			= false;
	bool 		FluidConfig::BLOOM 					= true;
	uint16_t 	FluidConfig::BLOOM_ITERATIONS 		= 8;
	uint16_t 	FluidConfig::BLOOM_RESOLUTION 		= 256;
	float 		FluidConfig::BLOOM_INTENSITY 		= 0.8f;
	float 		FluidConfig::BLOOM_THRESHOLD 		= 0.6f;
	float 		FluidConfig::BLOOM_SOFT_KNEE 		= 0.7f;
	bool 		FluidConfig::SUNRAYS 				= true;
	uint16_t 	FluidConfig::SUNRAYS_RESOLUTION 	= 196;
	float 		FluidConfig::SUNRAYS_WEIGHT 		= 1.0f;

	//-----------------------------------------------------------------------------------
	Fluid::Fluid(uint32_t _w, uint32_t _h)
	{
		float ar = (float)_w / (float)_h;
		glm::vec2 gridResolution 	= glm::vec2((uint32_t)(FluidConfig::SIM_RESOLUTION * ar), FluidConfig::SIM_RESOLUTION);
		glm::vec2 densityResolution = glm::vec2((uint32_t)(FluidConfig::DYE_RESOLUTION * ar), FluidConfig::DYE_RESOLUTION);

		// double channel FBO for swapping (when iterating)
		m_density 	= MakeRef<Slab>(densityResolution.x, densityResolution.y, ColorFormat::RGBA16F, "density");
		m_velocity 	= MakeRef<Slab>(gridResolution.x, gridResolution.y, ColorFormat::RG16F, "velocity");
		m_pressure 	= MakeRef<Slab>(gridResolution.x, gridResolution.y, ColorFormat::RG16F, "pressure");
		// single channel FBO to store computed divergence before pressure equation
		m_divergence = MakeRef<Surface>(gridResolution.x, gridResolution.y, ColorFormat::R16F, "divergence");

		// shaders
		ShaderLibrary::load("../base/fluid-sim/shaders/divergence.glsl");
		ShaderLibrary::load("../base/fluid-sim/shaders/pressure.glsl");
		ShaderLibrary::load("../base/fluid-sim/shaders/clear.glsl");
		ShaderLibrary::load("../base/fluid-sim/shaders/gradientSubtract.glsl");
		ShaderLibrary::load("../base/fluid-sim/shaders/advection.glsl");
		
		m_divergenceShader 		 = ShaderLibrary::get("divergence");
		m_pressureShader 		 = ShaderLibrary::get("pressure");
		m_clearShader 			 = ShaderLibrary::get("clear");
		m_subtractGradientShader = ShaderLibrary::get("gradientSubtract");
		m_advectionShader		 = ShaderLibrary::get("advection");


		// create vertex array for a screen sized quad for blitting
		// textures with respective resolutions (viewports).
		//
		glm::vec2 vertexData[] =
		{
			glm::vec2(-1.0f, -1.0f),
			glm::vec2( 1.0f, -1.0f),
			glm::vec2( 1.0f,  1.0f),
			glm::vec2(-1.0f,  1.0f)
		};
		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
		vbo->setBufferLayout({
			{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" }
		});
		vbo->setData(vertexData, sizeof(vertexData));
		Ref<IndexBuffer> ibo = API::newIndexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
		ibo->setData(indices, 6);
		Ref<VertexArray> m_screenQuad = API::newVertexArray(vbo, ibo);
		Renderer::executeRenderCommands();

	}

	//-----------------------------------------------------------------------------------
	Fluid::~Fluid()
	{

	}

	//-----------------------------------------------------------------------------------
	void Fluid::step(float _dt)
	{
		Renderer::disableBlending();

		// divergence
		m_divergenceShader->enable();
		_setVertexShaderTexelSz(m_divergenceShader, m_divergence);
		/* m_divergenceShader->setUniform1i("u_velocity_sampler", 0); */
		m_velocity->getSrcFBO()->bindTexture(0);
		blit(m_divergence);

		// reset pressure
		m_clearShader->enable();
		/* m_clearShader->setUniform1i("u_texture_sampler", 0); */
		m_clearShader->setUniform1f("u_value", FluidConfig::PRESSURE);
		m_pressure->getSrcFBO()->bindTexture(0);
		blit(m_pressure->getDst());
		m_pressure->swap();

		// compute pressure
		m_pressureShader->enable();
		_setVertexShaderTexelSz(m_pressureShader, m_pressure->getSrc());
		m_pressureShader->setUniform1i("u_divergence_sampler", 0);
		for (uint16_t i = 0; i < FluidConfig::PRESSURE_ITERATIONS; i++)
		{
			m_pressureShader->setUniform1i("u_pressure_sampler", 1);
			m_pressure->getSrcFBO()->bindTexture(1);
			blit(m_pressure->getDst());
			m_pressure->swap();
		}

		// subtract pressure gradient from intermediate velocity
		m_subtractGradientShader->enable();
		_setVertexShaderTexelSz(m_subtractGradientShader, m_pressure->getSrc());
		/* m_subtractGradientShader->setUniform1i("u_pressure_sampler", 0); */
		m_subtractGradientShader->setUniform1i("u_velocity_sampler", 1);
		m_pressure->getSrcFBO()->bindTexture(0);
		m_velocity->getSrcFBO()->bindTexture(1);
		blit(m_velocity->getDst());
		m_velocity->swap();


		// advection
		//
		
		// advect velocity with itself
		m_advectionShader->enable();
		_setVertexShaderTexelSz(m_advectionShader, m_velocity->getSrc());
		m_advectionShader->setUniform1i("u_velocity_sampler", 0);
		m_advectionShader->setUniform1i("u_source_sampler", 0);
		m_advectionShader->setUniform1f("u_dissipation", FluidConfig::VELOCITY_DISSIPATION);
		m_advectionShader->setUniform1f("u_dt", _dt);
		m_velocity->getSrcFBO()->bindTexture(0);
		blit(m_velocity->getDst());
		m_velocity->swap();

		// advect density (dye)
		//m_advectionShader->enable();
		
		m_advectionShader->setUniform1i("u_velocity_sampler", 0);
		m_advectionShader->setUniform1i("u_source_sampler", 1);
		m_advectionShader->setUniform1f("u_dissipation", FluidConfig::DENSITY_DISSIPATION);
		// dt already set above
		m_velocity->getSrcFBO()->bindTexture(0);
		m_density->getSrcFBO()->bindTexture(1);
		blit(m_density->getDst());
		m_density->swap();



	}

	//-----------------------------------------------------------------------------------
	void Fluid::blit(const Ref<Surface>& _surface)
	{
		Renderer::setViewport(glm::ivec2(0), _surface->getSz());
		_surface->getFBO()->bind();

		Renderer::drawIndexedNoDepth(m_screenQuad);

		//Renderer::get().executeRenderCommands();
	}

	//-----------------------------------------------------------------------------------
	void Fluid::blit(const Ref<Slab>& _slab)
	{
		Ref<Surface> s = _slab->getDst();

		Renderer::setViewport(glm::ivec2(0), s->getSz());
		s->getFBO()->bind();

		Renderer::drawIndexedNoDepth(m_screenQuad);
	}

	//-----------------------------------------------------------------------------------
	void Fluid::render(uint32_t _w, uint32_t _h, float _dt)
	{

	}

}



