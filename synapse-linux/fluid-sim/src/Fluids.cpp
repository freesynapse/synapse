
#include "Fluids.hpp"

#include <Synapse.hpp>
#include <Synapse/API/SynapseOpenGLBindings.hpp>


// static member variable declarations
//
uint32_t Fluid::s_gridWidth;
uint32_t Fluid::s_gridHeight;
float 	 Fluid::s_cellSize;
float 	 Fluid::s_densityRadius;

Ref<MeshShape> Fluid::s_screenQuad;

Slab Fluid::s_velocity;
Slab Fluid::s_density;
Slab Fluid::s_pressure;
Slab Fluid::s_temperature;

Ref<Framebuffer> Fluid::s_divergence;
Ref<Framebuffer> Fluid::s_obstacles;

Ref<Shader> Fluid::s_advectShader;
Ref<Shader> Fluid::s_jacobiShader;
Ref<Shader> Fluid::s_subtractGradientShader;
Ref<Shader> Fluid::s_computeDivergenceShader;
Ref<Shader> Fluid::s_densityShader;
Ref<Shader> Fluid::s_buoyancyShader;
Ref<Shader> Fluid::s_obstaclesShader;
Ref<Shader> Fluid::s_fillShader;
Ref<Shader> Fluid::s_fluidVisualizationShader;

float 		Fluid::s_ambientTemperature;
float 		Fluid::s_impulseTemperature;
float 		Fluid::s_impulseDensity;
int 		Fluid::s_numJacobiIterations;
float 		Fluid::s_timeStep;
float 		Fluid::s_smokeBuoyancy;
float 		Fluid::s_smokeWeight;
float 		Fluid::s_gradientScale;
float 		Fluid::s_temperatureDissipation;
float 		Fluid::s_velocityDissipation;
float 		Fluid::s_densityDissipation;
glm::vec2	Fluid::s_impulsePosition;


//-----------------------------------------------------------------------------------
void Fluid::init(uint32_t _screen_width, uint32_t _screen_height)
{
	SYN_PROFILE_FUNCTION();

	//
	SYN_CORE_TRACE("initializing Fluid.");

	//
	s_gridWidth = _screen_width / 2;
	s_gridHeight = _screen_height / 2;
	s_cellSize = 1.25f;
	s_densityRadius = (float)s_gridWidth / 8.0f;

	// setup shader sources
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidAdvect.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidJacobi.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidSubtractGradient.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidComputeDivergence.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidAddDensity.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidBuoyancy.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidObstacles.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidFill.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim/shaders/fluidVisualization.glsl");

	s_advectShader				= ShaderLibrary::get("fluidAdvect");
	s_jacobiShader				= ShaderLibrary::get("fluidJacobi");
	s_subtractGradientShader	= ShaderLibrary::get("fluidSubtractGradient");
	s_computeDivergenceShader	= ShaderLibrary::get("fluidComputeDivergence");
	s_densityShader				= ShaderLibrary::get("fluidAddDensity");
	s_buoyancyShader			= ShaderLibrary::get("fluidBuoyancy");
	s_obstaclesShader			= ShaderLibrary::get("fluidObstacles");
	s_fillShader				= ShaderLibrary::get("fluidFill");
	s_fluidVisualizationShader	= ShaderLibrary::get("fluidVisualization");

	// set up surfaces
	s_velocity 		= createSlab(s_gridWidth, s_gridHeight, ColorFormat::RG16F);
	s_density 		= createSlab(s_gridWidth, s_gridHeight, ColorFormat::R16F);
	s_pressure 		= createSlab(s_gridWidth, s_gridHeight, ColorFormat::R16F);
	s_temperature 	= createSlab(s_gridWidth, s_gridHeight, ColorFormat::R16F);
	s_divergence	= createSurface(s_gridWidth, s_gridHeight, ColorFormat::RGB16F);
	
	// obstacles
	s_obstacles		= createSurface(s_gridWidth, s_gridHeight, ColorFormat::RGB16F);
	createObstacles(s_obstacles, s_gridWidth, s_gridHeight);

	// initial conditions and model parameters
	s_ambientTemperature  	 = 0.0f;
	s_impulseTemperature  	 = 10.0f;
	s_impulseDensity 	  	 = 1.0f;
	s_numJacobiIterations 	 = 40;
	s_timeStep 			  	 = 0.125f;
	s_smokeBuoyancy 	  	 = 1.0f;
	s_smokeWeight 		  	 = 0.05f;
	s_gradientScale 	  	 = 1.125f / s_cellSize;
	s_temperatureDissipation = 0.99f;
	s_velocityDissipation 	 = 0.99f;
	s_densityDissipation 	 = 0.999f;
	s_impulsePosition 		 = { s_gridWidth / 2.0f, -(int)s_densityRadius / 2.0f };

	// enable correct blending equation
	Renderer::setBlendingEq(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// set ambient temperature
	clearSurface(s_temperature.frontSurface, s_ambientTemperature);

	// get quad for rendering of textures
	s_screenQuad = MeshCreator::createShapeViewportQuad();

}


//-----------------------------------------------------------------------------------
Slab Fluid::createSlab(uint32_t _width, uint32_t _height, const ColorFormat& _fmt)
{
	Slab slab;
	slab.frontSurface = createSurface(_width, _height, _fmt);
	slab.backSurface = createSurface(_width, _height, _fmt);
	return slab;
}


//-----------------------------------------------------------------------------------
Ref<Framebuffer> Fluid::createSurface(uint32_t _width, uint32_t _height, const ColorFormat& _fmt)
{

	Ref<Framebuffer> frameBuffer = MakeRef<Framebuffer>(_width, _height, _fmt, false);
	Renderer::executeRenderCommands();

	frameBuffer->clearFramebuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), GL_COLOR_BUFFER_BIT);
	Renderer::executeRenderCommands();

	return frameBuffer;
	
}


//-----------------------------------------------------------------------------------
void Fluid::createObstacles(const Ref<Framebuffer>& _frame_buffer, uint32_t _width, uint32_t _height)
{
	/* 
	Draw obstacles to framebuffer. Since the obstacles are
	unaffected by the subsequent shader operations, this operation
	only has to be performed once. The obstacles are stored in the 
	obstacle framebuffer (s_obstacles).
	*/	
	// containment border
	const float T = 0.999f;

	struct vdata
	{
		glm::vec2 position;
	};
	vdata vertexData[] =
	{
		glm::vec2(-T, -T),
		glm::vec2( T, -T),
		glm::vec2( T,  T),
		glm::vec2(-T,  T)
	};

	uint32_t indices[] = { 0, 1, 2, 3, 0 };

	Ref<VertexBuffer> vertexBuffer = MakeRef<VertexBuffer>();
	vertexBuffer->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float2, "a_position" },
	});
	vertexBuffer->setData(vertexData, sizeof(vertexData));

	Ref<IndexBuffer> indexBuffer = MakeRef<IndexBuffer>(GL_LINE_STRIP, GL_STATIC_DRAW);
	indexBuffer->setData(indices, 5);

	Ref<VertexArray> vertexArray = MakeRef<VertexArray>(vertexBuffer, indexBuffer);

	Renderer::executeRenderCommands();

	//
	//
	_frame_buffer->bind();
	_frame_buffer->clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	
	s_fillShader->enable();
	vertexArray->bind();

	Renderer::drawIndexedFlat(vertexArray);

	_frame_buffer->saveAsPNG("../screenshots/___out.png");
	_frame_buffer->unbind();
	Renderer::executeRenderCommands();

	// TODO: add more obstacles

}


//-----------------------------------------------------------------------------------
void Fluid::clearSurface(const Ref<Framebuffer>& _frame_buffer, float _color)
{
	_frame_buffer->clearFramebuffer(glm::vec4(_color), GL_COLOR_BUFFER_BIT);

}


//-----------------------------------------------------------------------------------
void Fluid::clearSurface(const Ref<Framebuffer>& _frame_buffer, const glm::vec4& _color)
{
	_frame_buffer->clearFramebuffer(_color, GL_COLOR_BUFFER_BIT);

}


//-----------------------------------------------------------------------------------
void Fluid::swapSurfaces(Slab* _slab)
{
	Ref<Framebuffer> tmp = _slab->backSurface;
	_slab->backSurface = _slab->frontSurface;
	_slab->frontSurface = tmp;
}


//-----------------------------------------------------------------------------------
void Fluid::update(float _time_step)
{
	SYN_PROFILE_FUNCTION();

	Renderer::setViewport(glm::ivec2(0, 0), glm::ivec2(s_gridWidth, s_gridHeight));

	// advect velocity
	advect(s_velocity.frontSurface, s_velocity.frontSurface, s_obstacles, s_velocity.backSurface, s_velocityDissipation);
	swapSurfaces(&s_velocity);
	// advect temperature
	advect(s_velocity.frontSurface, s_temperature.frontSurface, s_obstacles, s_temperature.backSurface, s_temperatureDissipation);
	swapSurfaces(&s_temperature);
	// advect density
	advect(s_velocity.frontSurface, s_density.frontSurface, s_obstacles, s_density.backSurface, s_densityDissipation);
	swapSurfaces(&s_density);
	//
	applyBuoyancy(s_velocity.frontSurface, s_temperature.frontSurface, s_density.frontSurface, s_velocity.backSurface);
	swapSurfaces(&s_velocity);
	//
	applyImpulse(s_temperature.frontSurface, s_impulsePosition, s_impulseTemperature);
	applyImpulse(s_density.frontSurface, s_impulsePosition, s_impulseDensity);
	//
	computeDivergence(s_velocity.frontSurface, s_obstacles, s_divergence);
	clearSurface(s_pressure.frontSurface, 0.0f);
	// solve pressure
	for (uint32_t i = 0; i < s_numJacobiIterations; i++)
	{
		jacobi(s_pressure.frontSurface, s_divergence, s_obstacles, s_pressure.backSurface);
		swapSurfaces(&s_pressure);
	}
	//
	subtractGradient(s_velocity.frontSurface, s_pressure.frontSurface, s_obstacles, s_velocity.backSurface);
	swapSurfaces(&s_velocity);

}


//-----------------------------------------------------------------------------------
void Fluid::render(float _width, float _height, float _dt)
{
	/* rendering framebuffer is presumed to be bound already */

	SYN_PROFILE_FUNCTION();

	// bind fill shader
	s_fluidVisualizationShader->enable();
	Renderer::enableGLenum(GL_BLEND);

	s_fluidVisualizationShader->setUniform2fv("u_scale", glm::vec2(1.0f / _width, 1.0f / _height));
	s_fluidVisualizationShader->setUniform1i("u_texture_sampler", 0);

	// draw ink
	//s_density.frontSurface->bindTexture(0);
	//s_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(1.0f));
	//Renderer::drawIndexedFlat(s_screenQuad->getVertexArray());

	// draw obstacles
	s_obstacles->bindTexture(0);
	s_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(0.125f, 0.4f, 0.75f));
	Renderer::drawIndexed(s_screenQuad->getVertexArray());

	//
	//Renderer::disableGLenum(GL_BLEND);


}


//-----------------------------------------------------------------------------------
void Fluid::resetState()
{
	//Renderer::resetTexture2D(0);
	//Renderer::resetTexture2D(1);
	//Renderer::resetTexture2D(2);
	SYN_RENDER_0({
		glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);	
		glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);	
		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDisable(GL_BLEND);
	})
}


//-----------------------------------------------------------------------------------
// Shader computation functions
//
void Fluid::advect(const Ref<Framebuffer>& _velocity, 
				   const Ref<Framebuffer>& _source, 
				   const Ref<Framebuffer>& _obstacles, 
				   const Ref<Framebuffer>& _dest,
				   float _dissipation)
{
	SYN_PROFILE_FUNCTION();

	s_advectShader->enable();
	// parameters
	s_advectShader->setUniform2fv("u_inverse_size", glm::vec2(1.0f / s_gridWidth, 1.0f / s_gridHeight));
	s_advectShader->setUniform1f("u_time_step", s_timeStep);
	s_advectShader->setUniform1f("u_dissipation", _dissipation);
	// texture samplers
	/* s_advectShader->setUniform1i("u_velocity_texture_sampler", 0); */
	s_advectShader->setUniform1i("u_source_texture_sampler", 1);
	s_advectShader->setUniform1i("u_obstacles_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_source->bindTexture(1);
	_obstacles->bindTexture(2);
	// compute
	Renderer::drawIndexedFlat(s_screenQuad->getVertexArray()->getArrayID(), GL_TRIANGLES);
	//
	resetState();
}

//-----------------------------------------------------------------------------------
void Fluid::jacobi(const Ref<Framebuffer>& _pressure, 
				   const Ref<Framebuffer>& _divergence, 
				   const Ref<Framebuffer>& _obstacles, 
				   const Ref<Framebuffer>& _dest)
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
	Renderer::drawIndexedFlat(s_screenQuad->getVertexArray()->getArrayID(), GL_TRIANGLES);
	//
	resetState();
}

//-----------------------------------------------------------------------------------
void Fluid::subtractGradient(const Ref<Framebuffer>& _velocity, 
							 const Ref<Framebuffer>& _pressure, 
							 const Ref<Framebuffer>& _obstacles, 
							 const Ref<Framebuffer>& _dest)
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
	Renderer::drawIndexedFlat(s_screenQuad->getVertexArray()->getArrayID());
	//
	resetState();
}

//-----------------------------------------------------------------------------------
void Fluid::computeDivergence(const Ref<Framebuffer>& _velocity, 
							  const Ref<Framebuffer>& _obstacles, 
							  const Ref<Framebuffer>& _dest)
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
	Renderer::drawIndexedFlat(s_screenQuad->getVertexArray()->getArrayID());
	//
	resetState();
}

//-----------------------------------------------------------------------------------
void Fluid::applyImpulse(const Ref<Framebuffer>& _dest, const glm::vec2& _position, float _value)
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
	Renderer::drawIndexedFlat(s_screenQuad->getVertexArray()->getArrayID());
	//
	resetState();
}

//-----------------------------------------------------------------------------------
void Fluid::applyBuoyancy(const Ref<Framebuffer>& _velocity, 
						  const Ref<Framebuffer>& _temperature, 
						  const Ref<Framebuffer>& _density, 
						  const Ref<Framebuffer>& _dest)
{
	SYN_PROFILE_FUNCTION();

	s_buoyancyShader->enable();
	// parameters
	s_buoyancyShader->setUniform1f("u_ambient_temperature", s_ambientTemperature);
	s_buoyancyShader->setUniform1f("u_time_step", s_timeStep);
	s_buoyancyShader->setUniform1f("u_sigma", s_smokeBuoyancy);
	s_buoyancyShader->setUniform1f("u_kappa", s_smokeWeight);
	// texture samplers
	/* s_buoyancyShader->setUniform1i("u_velocity_texture_sampler", 0); */
	s_buoyancyShader->setUniform1i("u_temperature_texture_sampler", 1);
	s_buoyancyShader->setUniform1i("u_density_texture_sampler", 2);
	// prepare operations
	_dest->bind(false);
	_velocity->bindTexture(0);
	_temperature->bindTexture(1);
	_density->bindTexture(2);
	// compute
	Renderer::drawIndexedFlat(s_screenQuad->getVertexArray()->getArrayID());
	//
	resetState();
}




