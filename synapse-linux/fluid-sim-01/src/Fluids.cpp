
#include "Fluids.hpp"

#include <Synapse.hpp>
#include <Synapse/API/SynapseOpenGLBindings.hpp>


// static member variable declarations
//
uint32_t Fluid::s_gridWidth;
uint32_t Fluid::s_gridHeight;
float 	 Fluid::s_cellSize;
float 	 Fluid::s_densityRadius;

Ref<VertexArray> Fluid::s_quadVAO;

Slab Fluid::s_velocity;
Slab Fluid::s_density;
Slab Fluid::s_pressure;
Slab Fluid::s_temperature;

Ref<FluidFramebuffer> Fluid::s_divergence;
Ref<FluidFramebuffer> Fluid::s_obstacles;

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
	static int grid_factor = 3;
	s_gridWidth = _screen_width / grid_factor;
	s_gridHeight = _screen_height / grid_factor;
	s_cellSize = 1.25f;
	s_densityRadius = (float)s_gridWidth / 8.0f;

	// setup shader sources
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidAdvect.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidJacobi.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidSubtractGradient.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidComputeDivergence.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidAddDensity.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidBuoyancy.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidObstacles.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidFill.glsl");
	ShaderLibrary::load("/home/iomanip/source/synapse/synapse-linux/fluid-sim-01/shaders/fluidVisualization.glsl");

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
	s_velocity 		= createSlab(s_gridWidth, s_gridHeight, ColorFormat::RG16F, "velocity");
	s_density 		= createSlab(s_gridWidth, s_gridHeight, ColorFormat::R16F, "density");
	s_pressure 		= createSlab(s_gridWidth, s_gridHeight, ColorFormat::R16F, "pressure");
	s_temperature 	= createSlab(s_gridWidth, s_gridHeight, ColorFormat::R16F, "temperature");
	s_divergence	= createSurface(s_gridWidth, s_gridHeight, ColorFormat::RGB16F, "divergence");
	s_obstacles 	= createSurface(s_gridWidth*4, s_gridHeight*4, ColorFormat::R16F, "obstacles");
	
	// setup geometry
	s_quadVAO = createQuad();
	createObstacles(s_obstacles);

	// initial conditions and model parameters
	s_ambientTemperature  	 = 0.0f;
	s_impulseTemperature  	 = 10.0f;
	s_impulseDensity 	  	 = 1.0f;
	s_numJacobiIterations 	 = 40;
	s_timeStep 			  	 = 0.125f;
	s_smokeBuoyancy 	  	 = 1.5f;
	s_smokeWeight 		  	 = 0.05f;
	s_gradientScale 	  	 = 1.125f / s_cellSize;
	s_temperatureDissipation = 0.99f;
	s_velocityDissipation 	 = 0.99f;
	s_densityDissipation 	 = 0.999f;
	s_impulsePosition 		 = { s_gridWidth / 2.0f, -(int)s_densityRadius / 2.0f };

	#define OBSTACLES_SPHERE 	0
	#define OBSTACLES_TRIANGLE 	1
	// --- DEBUG
	//
	/*
	 *	Apply 100 iterations and save screenshots to inspect scalar fields.
	 */
	/*
	Renderer::setViewport(glm::ivec2(0, 0), glm::ivec2(s_gridWidth, s_gridHeight));
	Renderer::executeRenderCommands();
	for (int i = 0; i < 400; i++)
		update(0);

	s_velocity.frontSurface->saveAsPNG("../screenshots/___velocity-02.png");
	s_density.frontSurface->saveAsPNG("../screenshots/___density-02.png");
	s_pressure.frontSurface->saveAsPNG("../screenshots/___pressure-02.png");
	s_temperature.frontSurface->saveAsPNG("../screenshots/___temperature-02.png");
	s_divergence->saveAsPNG("../screenshots/___divergence-02.png");
	s_obstacles->saveAsPNG("../screenshots/___obstacles-02.png");
	*/

	// enable correct blending equation
	Renderer::setBlendingEq(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// set ambient temperature
	clearSurface(s_temperature.frontSurface, s_ambientTemperature);

}


//-----------------------------------------------------------------------------------
void Fluid::update(float _time_step, bool _read_mouse)
{
	SYN_PROFILE_FUNCTION();

	// move the source
	static float x_lo_bound = 50.0f;
	static float x_hi_bound = s_gridWidth - 50.0f;
	if (_read_mouse)
	{
		glm::vec2 pos = InputManager::get_mouse_position();
		glm::vec2 viewport = Renderer::getViewport();
		static float gridFactor = viewport.x / s_gridWidth;
		pos.x = MIN(pos.x, viewport.x);
		// translate to grid coordinates
		float x_pos = pos.x / gridFactor;
		// update and limit position
		s_impulsePosition.x = CLAMP(x_pos, x_lo_bound, x_hi_bound);
	}
	//else //(!_read_mouse)
	//{
	//	static float dx = 0.7f;
	//	s_impulsePosition.x += dx;
	//	if ((s_impulsePosition.x >= x_hi_bound) || (s_impulsePosition.x <= x_lo_bound))
	//		dx = -dx;
	//}

	// adjust simulation time step, aimed at 60 fps
	static float fps60 = 1.0f / 60.0f * 1000.0f;
	static float simTimeStep = 0.125f;
	//s_timeStep = simTimeStep * _time_step / fps60;
	
	Renderer::disableGLenum(GL_BLEND);
	Renderer::setViewport(glm::ivec2(0, 0), glm::ivec2(s_gridWidth, s_gridHeight));
	Renderer::executeRenderCommands();

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

	Renderer::executeRenderCommands();

}


//-----------------------------------------------------------------------------------
void Fluid::render(float _width, float _height, float _dt)
{
	/* rendering FluidFramebuffer is presumed to be bound already */

	SYN_PROFILE_FUNCTION();

	// bind visualization shader
	s_fluidVisualizationShader->enable();
	Renderer::enableGLenum(GL_BLEND);

	s_fluidVisualizationShader->setUniform2fv("u_scale", glm::vec2(1.0f / _width, 1.0f / _height));
	s_fluidVisualizationShader->setUniform1i("u_texture_sampler", 0);

	// draw scalar field
	//
	//s_velocity.frontSurface->bindTexture(0);
	//s_temperature.frontSurface->bindTexture(0);
	//s_pressure.frontSurface->bindTexture(0);
	//s_divergence->bindTexture(0);
	s_density.frontSurface->bindTexture(0);
	s_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(0.85f, 0.85f, 0.85f));
	Renderer::drawIndexedNoDepth(s_quadVAO);

	// draw obstacles
	s_obstacles->bindTexture(0);
	s_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(0.125f, 0.4f, 0.75f));
	//s_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(1.0f, 0.0f, 1.0f));
	Renderer::drawIndexedNoDepth(s_quadVAO);

}


//-----------------------------------------------------------------------------------
Slab Fluid::createSlab(uint32_t _width, uint32_t _height, const ColorFormat& _fmt, const std::string& _name)
{
	std::string frontName = "";
	std::string backName = "";
	
	if (_name.compare("") != 0)
	{
		frontName = "front_" + _name;
		backName = "back_" + _name; 
	}

	Slab slab;
	slab.frontSurface = createSurface(_width, _height, _fmt, frontName);
	slab.backSurface = createSurface(_width, _height, _fmt, backName);
	return slab;
}


//-----------------------------------------------------------------------------------
Ref<FluidFramebuffer> Fluid::createSurface(uint32_t _width, uint32_t _height, const ColorFormat& _fmt, const std::string& _name)
{

	Ref<FluidFramebuffer> framebuffer = MakeRef<FluidFramebuffer>(_width, _height, _fmt, false, _name);
	framebuffer->clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	Renderer::executeRenderCommands();

	return framebuffer;
	
}


//-----------------------------------------------------------------------------------
Ref<VertexArray> Fluid::createQuad()
{
	/*
	Screen-sized quad for rendering all the textures in their respective
	viewports. In screen-coordinates.
	*/
	glm::vec4 vertexData[] =
	{
		glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f),
		glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f),
		glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f)
	};

	uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

	Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);
	vbo->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" }
	});
	vbo->setData(vertexData, sizeof(vertexData));

	Ref<IndexBuffer> ibo = API::newIndexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
	ibo->setData(indices, 6);

	Ref<VertexArray> vao = API::newVertexArray(vbo, ibo);
	Renderer::executeRenderCommands();

	return vao;
}


//-----------------------------------------------------------------------------------
void Fluid::createObstacles(const Ref<FluidFramebuffer>& _frame_buffer)
{
	/* 
	Draw obstacles to FluidFramebuffer. Since the obstacles are
	unaffected by the subsequent shader operations, this operation
	only has to be performed once. The obstacles are stored in the 
	obstacle FluidFramebuffer (s_obstacles).
	*/	
	
	// containment border
	//
	float T = 0.999f;
	//float x = 1.0f - 0.00139470f;
	//float y = 1.0f - 0.00204918f;

	glm::vec4 vertexData[] =
	{
		glm::vec4(-T, -T, 0.0f, 1.0f),
		glm::vec4( T, -T, 0.0f, 1.0f),
		glm::vec4( T,  T, 0.0f, 1.0f),
		glm::vec4(-T,  T, 0.0f, 1.0f)
	};

	uint32_t borderIndices[] = { 0, 1, 2, 3, 0 };

	//
	_frame_buffer->bind();
	_frame_buffer->clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	
	s_fillShader->enable();

	Ref<VertexBuffer> borderVertexBuffer = MakeRef<VertexBuffer>();
	borderVertexBuffer->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" },
	});
	borderVertexBuffer->setData(vertexData, sizeof(vertexData));

	Ref<IndexBuffer> borderIndexBuffer = MakeRef<IndexBuffer>(GL_LINE_STRIP, GL_STATIC_DRAW);
	borderIndexBuffer->setData(borderIndices, 5);

	Ref<VertexArray> borderVertexArray = MakeRef<VertexArray>(borderVertexBuffer, borderIndexBuffer);

	Renderer::executeRenderCommands();
	
	// draw border
	Renderer::drawIndexedNoDepth(borderVertexArray);

	// triangle
	//
	#if (OBSTACLES_TRIANGLE==1)
	int n_triangles = 12;
	float x_wide = 0.05f;
	float y_high = 0.1f;
	float x_step = 1.8f / (float)n_triangles; 
	float x_start = -((float)n_triangles / 2.0f) * x_step + (x_step / 2.0f);
	float x_center = x_start;
	float y_center = 0.0f;
	glm::vec4 triangleVertices[n_triangles * 3];

	glm::vec4* ntrivertex = &triangleVertices[0];
	for (int i = 0; i < n_triangles; i++)
	{
		*ntrivertex++ = glm::vec4(x_center - x_wide, y_center + y_high, 0.0f, 1.0f);
		*ntrivertex++ = glm::vec4(x_center,          y_center - y_high, 0.0f, 1.0f);
		*ntrivertex++ = glm::vec4(x_center + x_wide, y_center + y_high, 0.0f, 1.0f);
		x_center += x_step;
	}

	Ref<VertexBuffer> triangleVertexBuffer = API::newVertexBuffer(GL_STATIC_DRAW);
	triangleVertexBuffer->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" }
	});
	triangleVertexBuffer->setData(triangleVertices, sizeof(triangleVertices));
	SYN_CORE_TRACE("9*sizeof(glm::vec4)=", 9*sizeof(glm::vec4), "\tsizeof(trianglesVertices)=", sizeof(triangleVertices));
	Ref<VertexArray> triangleVertexArray = API::newVertexArray(triangleVertexBuffer);

	Renderer::executeRenderCommands();

	SYN_CORE_TRACE("num verts=", sizeof(triangleVertices) / sizeof(glm::vec4));
	// draw sphere
	Renderer::drawArraysNoDepth(triangleVertexArray, sizeof(triangleVertices) / sizeof(glm::vec4), 0, GL_TRIANGLES);
	#endif


	// sphere
	//
	#if (OBSTACLES_SPHERE==1)
	float w = (float)_frame_buffer->getWidth();
	float h = (float)_frame_buffer->getHeight();
	uint32_t slices = 64;
	float spehereVertices[slices * 4 * 3];
	float twopi = 3.14159265358979323f * 2.0f;
	float radius = 0.2f;
	float theta = 0.0f;
	float dtheta = twopi / (float)(slices - 1);
	float* nvertex = &spehereVertices[0];
	float cx = 0.0f;
	float cy = 0.0f;

	for (uint32_t i = 0; i < slices; i++)
	{
		// center point
		*nvertex++ = cx;
		*nvertex++ = cy;
		*nvertex++ = 0.0f;
		*nvertex++ = 1.0f;

		// edge of circle at theta
		*nvertex++ = cx + radius * cosf(theta) * h / w;
		*nvertex++ = cy + radius * sinf(theta);
		*nvertex++ = 0.0f;
		*nvertex++ = 1.0f;

		// edge of circle at theta + dtheta
		theta += dtheta;
		*nvertex++ = cx + radius * cosf(theta) * h / w;
		*nvertex++ = cy + radius * sinf(theta);
		*nvertex++ = 0.0f;
		*nvertex++ = 1.0f;
	}

	Ref<VertexBuffer> sphereVertexBuffer = API::newVertexBuffer(GL_STATIC_DRAW);
	sphereVertexBuffer->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" }
	});
	sphereVertexBuffer->setData(spehereVertices, sizeof(spehereVertices));

	Ref<VertexArray> sphereVertexArray = API::newVertexArray(sphereVertexBuffer);

	Renderer::executeRenderCommands();

	// draw sphere
	Renderer::drawArraysNoDepth(sphereVertexArray, slices * 3, 0, GL_TRIANGLES);
	#endif

	//_frame_buffer->saveAsPNG("../screenshots/___out.png");
	_frame_buffer->unbind();
	Renderer::executeRenderCommands();

	// TODO: add more obstacles

}


//-----------------------------------------------------------------------------------
void Fluid::clearSurface(const Ref<FluidFramebuffer>& _frame_buffer, float _color)
{
	_frame_buffer->clear(glm::vec4(_color));

}


//-----------------------------------------------------------------------------------
void Fluid::clearSurface(const Ref<FluidFramebuffer>& _frame_buffer, const glm::vec4& _color)
{
	_frame_buffer->clear(_color);

}


//-----------------------------------------------------------------------------------
void Fluid::swapSurfaces(Slab* _slab)
{
	Ref<FluidFramebuffer> tmp = _slab->backSurface;
	_slab->backSurface = _slab->frontSurface;
	_slab->frontSurface = tmp;
}



