
#include <Synapse.hpp>

#include "Smoke.hpp"



using namespace Syn;


//-----------------------------------------------------------------------------------
Smoke::Smoke(uint32_t _screen_width, uint32_t _screen_height)
{
	SYN_PROFILE_FUNCTION();

	//
	SYN_CORE_TRACE("initializing Smoke.");

	//
	static int grid_factor = 3;
	m_gridWidth = _screen_width / grid_factor;
	m_gridHeight = _screen_height / grid_factor;
	m_cellSize = 1.25f;
	m_densityRadius = (float)m_gridWidth / 8.0f;

	// setup shader sources
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidAdvect.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidJacobi.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidSubtractGradient.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidComputeDivergence.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidAddDensity.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidBuoyancy.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidObstacles.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidFill.glsl");
	ShaderLibrary::load("../assets/shaders/smoke-sim/fluidVisualization.glsl");

	m_advectShader				= ShaderLibrary::get("fluidAdvect");
	m_jacobiShader				= ShaderLibrary::get("fluidJacobi");
	m_subtractGradientShader	= ShaderLibrary::get("fluidSubtractGradient");
	m_computeDivergenceShader	= ShaderLibrary::get("fluidComputeDivergence");
	m_densityShader				= ShaderLibrary::get("fluidAddDensity");
	m_buoyancyShader			= ShaderLibrary::get("fluidBuoyancy");
	m_obstaclesShader			= ShaderLibrary::get("fluidObstacles");
	m_fillShader				= ShaderLibrary::get("fluidFill");
	m_fluidVisualizationShader	= ShaderLibrary::get("fluidVisualization");

	// set up surfaces
	m_velocity 		= createSlab(m_gridWidth, m_gridHeight, ColorFormat::RG16F, "velocity");
	m_density 		= createSlab(m_gridWidth, m_gridHeight, ColorFormat::R16F, "density");
	m_pressure 		= createSlab(m_gridWidth, m_gridHeight, ColorFormat::R16F, "pressure");
	m_temperature 	= createSlab(m_gridWidth, m_gridHeight, ColorFormat::R16F, "temperature");
	m_divergence	= createSurface(m_gridWidth, m_gridHeight, ColorFormat::RGB16F, "divergence");
	m_obstacles 	= createSurface(m_gridWidth*4, m_gridHeight*4, ColorFormat::R16F, "obstacles");
	
	// setup geometry
	m_quadVAO = createQuad();
	createObstacles(m_obstacles);

	// initial conditions and model parameters
	m_ambientTemperature  	 = 0.0f;
	m_impulseTemperature  	 = 10.0f;
	m_impulseDensity 	  	 = 1.5f;	// 1.0f
	m_numJacobiIterations 	 = 40;
	m_timeStep 			  	 = 0.125f;
	m_smokeBuoyancy 	  	 = 1.5f;
	m_smokeWeight 		  	 = 0.05f;
	m_gradientScale 	  	 = 1.125f / m_cellSize;
	m_temperatureDissipation = 0.99f;
	m_velocityDissipation 	 = 0.99f;
	m_densityDissipation 	 = 0.999f;
	m_impulsePosition 		 = { m_gridWidth / 2.0f, -(int)m_densityRadius / 2.0f };

	#define OBSTACLES_SPHERE 	1
	#define OBSTACLES_TRIANGLES	0
	// --- DEBUG
	//
	/*
	*	Apply 100 iterations and save screenshots to inspect scalar fields.
	*/
	/*
	Renderer::setViewport(glm::ivec2(0, 0), glm::ivec2(m_gridWidth, m_gridHeight));
	Renderer::executeRenderCommands();
	for (int i = 0; i < 400; i++)
		update(0.0f);

	m_velocity.frontSurface->saveAsPNG("../screenshots/___velocity-02.png");
	m_density.frontSurface->saveAsPNG("../screenshots/___density-02.png");
	m_pressure.frontSurface->saveAsPNG("../screenshots/___pressure-02.png");
	m_temperature.frontSurface->saveAsPNG("../screenshots/___temperature-02.png");
	m_divergence->saveAsPNG("../screenshots/___divergence-02.png");
	m_obstacles->saveAsPNG("../screenshots/___obstacles-02.png");
	*/

	// enable correct blending equation
	Renderer::setBlendingEq(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// set ambient temperature
	clearSurface(m_temperature.frontSurface, m_ambientTemperature);

	Renderer::get().executeRenderCommands();

}


//-----------------------------------------------------------------------------------
void Smoke::update(float _time_step)
{
	SYN_PROFILE_FUNCTION();

	bool read_mouse = InputManager::is_button_pressed(SYN_MOUSE_BUTTON_1);

	// move the source
	static float x_lo_bound = 50.0f;
	static float x_hi_bound = m_gridWidth - 50.0f;
	if (read_mouse)
	{
		glm::vec2 pos = InputManager::get_mouse_position();
		glm::vec2 viewport = Renderer::getViewport();
		static float gridFactor = viewport.x / m_gridWidth;
		pos.x = std::min(pos.x, viewport.x);
		// translate to grid coordinates
		float x_pos = pos.x / gridFactor;
		// update and limit position
		m_impulsePosition.x = CLAMP(x_pos, x_lo_bound, x_hi_bound);
	}
	
	Renderer::disableGLenum(GL_BLEND);
	Renderer::setViewport(glm::ivec2(0, 0), glm::ivec2(m_gridWidth, m_gridHeight));
	Renderer::executeRenderCommands();

	// advect velocity
	advect(m_velocity.frontSurface, m_velocity.frontSurface, m_obstacles, m_velocity.backSurface, m_velocityDissipation);
	swapSurfaces(&m_velocity);
	// advect temperature
	advect(m_velocity.frontSurface, m_temperature.frontSurface, m_obstacles, m_temperature.backSurface, m_temperatureDissipation);
	swapSurfaces(&m_temperature);
	// advect density
	advect(m_velocity.frontSurface, m_density.frontSurface, m_obstacles, m_density.backSurface, m_densityDissipation);
	swapSurfaces(&m_density);
	//
	applyBuoyancy(m_velocity.frontSurface, m_temperature.frontSurface, m_density.frontSurface, m_velocity.backSurface);
	swapSurfaces(&m_velocity);
	// add density and temperature
	applyImpulse(m_temperature.frontSurface, m_impulsePosition, m_impulseTemperature);
	applyImpulse(m_density.frontSurface, m_impulsePosition, m_impulseDensity);
	//
	computeDivergence(m_velocity.frontSurface, m_obstacles, m_divergence);
	clearSurface(m_pressure.frontSurface, 0.0f);
	// solve pressure
	for (uint32_t i = 0; i < m_numJacobiIterations; i++)
	{
		jacobi(m_pressure.frontSurface, m_divergence, m_obstacles, m_pressure.backSurface);
		swapSurfaces(&m_pressure);
	}
	//
	subtractGradient(m_velocity.frontSurface, m_pressure.frontSurface, m_obstacles, m_velocity.backSurface);
	swapSurfaces(&m_velocity);

	Renderer::executeRenderCommands();

}


//-----------------------------------------------------------------------------------
void Smoke::render(float _dt, float _width, float _height)
{
	/* rendering Framebuffer is presumed to be bound already */

	SYN_PROFILE_FUNCTION();

	// bind visualization shader
	m_fluidVisualizationShader->enable();
	Renderer::enableGLenum(GL_BLEND);

	m_fluidVisualizationShader->setUniform2fv("u_scale", glm::vec2(1.0f / _width, 1.0f / _height));
	m_fluidVisualizationShader->setUniform1i("u_texture_sampler", 0);

	// draw field
	//
	//m_velocity.frontSurface->bindTexture(0);
	//m_temperature.frontSurface->bindTexture(0);
	//m_pressure.frontSurface->bindTexture(0);
	//m_divergence->bindTexture(0);
	m_density.frontSurface->bindTexture(0);
	m_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(0.85f, 0.85f, 0.85f));
	Renderer::drawIndexedNoDepth(m_quadVAO);

	// draw obstacles
	m_obstacles->bindTexture(0);
	m_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(0.125f, 0.4f, 0.75f));
	//m_fluidVisualizationShader->setUniform3fv("u_fill_color", glm::vec3(1.0f, 0.0f, 1.0f));
	Renderer::drawIndexedNoDepth(m_quadVAO);

}


//-----------------------------------------------------------------------------------
void Smoke::createObstacles(const Ref<Framebuffer>& _frame_buffer)
{
	/* 
	Draw obstacles to Framebuffer. Since the obstacles are
	unaffected by the subsequent shader operations, this operation
	only has to be performed once. The obstacles are stored in the 
	obstacle Framebuffer (m_obstacles).
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
	
	m_fillShader->enable();

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
	#if (OBSTACLES_TRIANGLES==1)
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
	//Ref<VertexBuffer> triangleVertexBuffer = SYN_NEW_VERTEX_BUFFER_REF(GL_STATIC_DRAW);
	triangleVertexBuffer->setBufferLayout({
		{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" }
	});
	triangleVertexBuffer->setData(triangleVertices, sizeof(triangleVertices));
	Ref<VertexArray> triangleVertexArray = API::newVertexArray(triangleVertexBuffer);
	//Ref<VertexArray> triangleVertexArray = SYN_NEW_VERTEX_ARRAY_VBO(triangleVertexBuffer);

	Renderer::executeRenderCommands();

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



