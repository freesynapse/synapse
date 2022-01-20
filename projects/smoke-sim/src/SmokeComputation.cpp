
#include "Smoke.hpp"

#include <Synapse/Debug/Profiler.hpp>


//-----------------------------------------------------------------------------------
// Shader computation functions
//

void Smoke::advect(const Ref<Framebuffer>& _velocity, 
				   const Ref<Framebuffer>& _source, 
				   const Ref<Framebuffer>& _obstacles, 
				   const Ref<Framebuffer>& _dest,
				   float _dissipation)
{
	SYN_PROFILE_FUNCTION();

	m_advectShader->enable();
	// parameters
	m_advectShader->setUniform2fv("u_inverse_size", glm::vec2(1.0f / m_gridWidth, 1.0f / m_gridHeight));
	m_advectShader->setUniform1f("u_time_step", m_timeStep);
	m_advectShader->setUniform1f("u_dissipation", _dissipation);
	// texture samplers
	m_advectShader->setUniform1i("u_velocity_texture_sampler", 0);
	m_advectShader->setUniform1i("u_source_texture_sampler", 1);
	m_advectShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_source->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(m_quadVAO);
}

//-----------------------------------------------------------------------------------
void Smoke::jacobi(const Ref<Framebuffer>& _pressure, 
				   const Ref<Framebuffer>& _divergence, 
				   const Ref<Framebuffer>& _obstacles, 
				   const Ref<Framebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	m_jacobiShader->enable();
	// parameters
	m_jacobiShader->setUniform1f("u_alpha", -m_cellSize * m_cellSize);
	m_jacobiShader->setUniform1f("u_inverse_beta", 0.25f);
	// texture samplers
	/* m_jacobiShader->setUniform1i("u_pressure_texture_sampler", 0); */
	m_jacobiShader->setUniform1i("u_divergence_texture_sampler", 1);
	m_jacobiShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_pressure->bindTexture(0);
	_divergence->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(m_quadVAO);
}

//-----------------------------------------------------------------------------------
void Smoke::subtractGradient(const Ref<Framebuffer>& _velocity, 
							 const Ref<Framebuffer>& _pressure, 
							 const Ref<Framebuffer>& _obstacles, 
							 const Ref<Framebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	m_subtractGradientShader->enable();
	// parameters
	m_subtractGradientShader->setUniform1f("u_gradient_scale", m_gradientScale);
	// texture samplers
	/* m_subtractGradientShader->setUniform1i("u_velocity_texture_sampler", 0); */
	m_subtractGradientShader->setUniform1i("u_pressure_texture_sampler", 1);
	m_subtractGradientShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_pressure->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(m_quadVAO);
}

//-----------------------------------------------------------------------------------
void Smoke::computeDivergence(const Ref<Framebuffer>& _velocity, 
							  const Ref<Framebuffer>& _obstacles, 
							  const Ref<Framebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	m_computeDivergenceShader->enable();
	// parameters
	m_computeDivergenceShader->setUniform1f("u_half_inverse_cell_size", 0.5f / m_cellSize);
	// texture samplers
	/* m_computeDivergenceShader->setUniform1i("u_velocity_texture_sampler", 0); */
	m_computeDivergenceShader->setUniform1i("u_obstacles_texture_sampler", 1);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_obstacles->bindTexture(1);
	// compute
	Renderer::drawIndexedNoDepth(m_quadVAO);
}

//-----------------------------------------------------------------------------------
void Smoke::applyImpulse(const Ref<Framebuffer>& _dest, const glm::vec2& _position, float _value)
{
	SYN_PROFILE_FUNCTION();

	m_densityShader->enable();
	// parameters
	m_densityShader->setUniform2fv("u_point", _position);
	m_densityShader->setUniform1f("u_radius", m_densityRadius);
	m_densityShader->setUniform3fv("u_fill_color", glm::vec3(_value));
	// prepare operations
	_dest->bind(false);
	Renderer::enableGLenum(GL_BLEND);
	// compute
	Renderer::drawIndexedNoDepth(m_quadVAO);
	Renderer::disableGLenum(GL_BLEND);
}

//-----------------------------------------------------------------------------------
void Smoke::applyBuoyancy(const Ref<Framebuffer>& _velocity, 
						  const Ref<Framebuffer>& _temperature, 
						  const Ref<Framebuffer>& _density, 
						  const Ref<Framebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	m_buoyancyShader->enable();
	// parameters
	m_buoyancyShader->setUniform1f("u_ambient_temperature", m_ambientTemperature);
	m_buoyancyShader->setUniform1f("u_time_step", m_timeStep);
	m_buoyancyShader->setUniform1f("u_sigma", m_smokeBuoyancy);
	m_buoyancyShader->setUniform1f("u_kappa", m_smokeWeight);
	// texture samplers
	m_buoyancyShader->setUniform1i("u_velocity_texture_sampler", 0);
	m_buoyancyShader->setUniform1i("u_temperature_texture_sampler", 1);
	m_buoyancyShader->setUniform1i("u_density_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_temperature->bindTexture(1);
	_density->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(m_quadVAO);
}







