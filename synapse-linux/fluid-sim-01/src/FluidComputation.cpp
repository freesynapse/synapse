
#include "Fluids.hpp"

#include "Synapse/Debug/Profiler.hpp"


//-----------------------------------------------------------------------------------
// Shader computation functions
//

void Fluid::advect(const Ref<FluidFramebuffer>& _velocity, 
				   const Ref<FluidFramebuffer>& _source, 
				   const Ref<FluidFramebuffer>& _obstacles, 
				   const Ref<FluidFramebuffer>& _dest,
				   float _dissipation)
{
	SYN_PROFILE_FUNCTION();

	s_advectShader->enable();
	// parameters
	s_advectShader->setUniform2fv("u_inverse_size", glm::vec2(1.0f / s_gridWidth, 1.0f / s_gridHeight));
	s_advectShader->setUniform1f("u_time_step", s_timeStep);
	s_advectShader->setUniform1f("u_dissipation", _dissipation);
	// texture samplers
	s_advectShader->setUniform1i("u_velocity_texture_sampler", 0);
	s_advectShader->setUniform1i("u_source_texture_sampler", 1);
	s_advectShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_source->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(s_quadVAO);
}

//-----------------------------------------------------------------------------------
void Fluid::jacobi(const Ref<FluidFramebuffer>& _pressure, 
				   const Ref<FluidFramebuffer>& _divergence, 
				   const Ref<FluidFramebuffer>& _obstacles, 
				   const Ref<FluidFramebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	s_jacobiShader->enable();
	// parameters
	s_jacobiShader->setUniform1f("u_alpha", -s_cellSize * s_cellSize);
	s_jacobiShader->setUniform1f("u_inverse_beta", 0.25f);
	// texture samplers
	/* s_jacobiShader->setUniform1i("u_pressure_texture_sampler", 0); */
	s_jacobiShader->setUniform1i("u_divergence_texture_sampler", 1);
	s_jacobiShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_pressure->bindTexture(0);
	_divergence->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(s_quadVAO);
}

//-----------------------------------------------------------------------------------
void Fluid::subtractGradient(const Ref<FluidFramebuffer>& _velocity, 
							 const Ref<FluidFramebuffer>& _pressure, 
							 const Ref<FluidFramebuffer>& _obstacles, 
							 const Ref<FluidFramebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	s_subtractGradientShader->enable();
	// parameters
	s_subtractGradientShader->setUniform1f("u_gradient_scale", s_gradientScale);
	// texture samplers
	/* s_subtractGradientShader->setUniform1i("u_velocity_texture_sampler", 0); */
	s_subtractGradientShader->setUniform1i("u_pressure_texture_sampler", 1);
	s_subtractGradientShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_pressure->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(s_quadVAO);
}

//-----------------------------------------------------------------------------------
void Fluid::computeDivergence(const Ref<FluidFramebuffer>& _velocity, 
							  const Ref<FluidFramebuffer>& _obstacles, 
							  const Ref<FluidFramebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	s_computeDivergenceShader->enable();
	// parameters
	s_computeDivergenceShader->setUniform1f("u_half_inverse_cell_size", 0.5f / s_cellSize);
	// texture samplers
	/* s_computeDivergenceShader->setUniform1i("u_velocity_texture_sampler", 0); */
	s_computeDivergenceShader->setUniform1i("u_obstacles_texture_sampler", 1);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_obstacles->bindTexture(1);
	// compute
	Renderer::drawIndexedNoDepth(s_quadVAO);
}

//-----------------------------------------------------------------------------------
void Fluid::applyImpulse(const Ref<FluidFramebuffer>& _dest, const glm::vec2& _position, float _value)
{
	SYN_PROFILE_FUNCTION();

	s_densityShader->enable();
	// parameters
	s_densityShader->setUniform2fv("u_point", _position);
	s_densityShader->setUniform1f("u_radius", s_densityRadius);
	s_densityShader->setUniform3fv("u_fill_color", glm::vec3(_value));
	// prepare operations
	_dest->bind(false);
	Renderer::enableGLenum(GL_BLEND);
	// compute
	Renderer::drawIndexedNoDepth(s_quadVAO);
	Renderer::disableGLenum(GL_BLEND);
}

//-----------------------------------------------------------------------------------
void Fluid::applyBuoyancy(const Ref<FluidFramebuffer>& _velocity, 
						  const Ref<FluidFramebuffer>& _temperature, 
						  const Ref<FluidFramebuffer>& _density, 
						  const Ref<FluidFramebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	s_buoyancyShader->enable();
	// parameters
	s_buoyancyShader->setUniform1f("u_ambient_temperature", s_ambientTemperature);
	s_buoyancyShader->setUniform1f("u_time_step", s_timeStep);
	s_buoyancyShader->setUniform1f("u_sigma", s_smokeBuoyancy);
	s_buoyancyShader->setUniform1f("u_kappa", s_smokeWeight);
	// texture samplers
	s_buoyancyShader->setUniform1i("u_velocity_texture_sampler", 0);
	s_buoyancyShader->setUniform1i("u_temperature_texture_sampler", 1);
	s_buoyancyShader->setUniform1i("u_density_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_temperature->bindTexture(1);
	_density->bindTexture(2);
	// compute
	Renderer::drawIndexedNoDepth(s_quadVAO);
}







