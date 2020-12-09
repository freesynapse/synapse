
#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <Synapse/Renderer/Buffers/Framebuffer.hpp>
#include <Synapse/Renderer/Mesh/MeshShape.hpp>

using namespace Syn;


#define DEFAULT_VIEWPORT_WIDTH 		1434
#define DEFAULT_VIEWPORT_HEIGHT 	 976


//
typedef struct slab_
{
	Ref<Framebuffer> frontSurface;
	Ref<Framebuffer> backSurface;
} Slab;


//
class Fluid
{
public:
	static void init(uint32_t _screen_width=DEFAULT_VIEWPORT_WIDTH, uint32_t _screen_height=DEFAULT_VIEWPORT_HEIGHT);

	static Slab createSlab(uint32_t _width, uint32_t _height, const ColorFormat& _fmt);
	static Ref<Framebuffer> createSurface(uint32_t _width, uint32_t _height, const ColorFormat& _fmt);

	static void createObstacles(const Ref<Framebuffer>& _frame_buffer, uint32_t _width, uint32_t _height);

	static void clearSurface(const Ref<Framebuffer>& _frame_buffer, float _color=0.0f);
	static void clearSurface(const Ref<Framebuffer>& _frame_buffer, const glm::vec4& _color=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	static void swapSurfaces(Slab* _slab);

	// updates fluid one timestep
	static void update(float _time_step);
	//
	static void render(float _width, float _height, float _dt);
	//
	static void resetState();

	// computations
	static void advect(const Ref<Framebuffer>& _velocity, 
					   const Ref<Framebuffer>& _source, 
					   const Ref<Framebuffer>& _obstacles, 
					   const Ref<Framebuffer>& _dest,
					   float _dissipation);
	
	static void jacobi(const Ref<Framebuffer>& _pressure, 
					   const Ref<Framebuffer>& _divergence, 
					   const Ref<Framebuffer>& _obstacles, 
					   const Ref<Framebuffer>& _dest);
	
	static void subtractGradient(const Ref<Framebuffer>& _velocity, 
								 const Ref<Framebuffer>& _pressure, 
								 const Ref<Framebuffer>& _obstacles, 
								 const Ref<Framebuffer>& _dest);
	
	static void computeDivergence(const Ref<Framebuffer>& _velocity, 
								  const Ref<Framebuffer>& _obstacles, 
								  const Ref<Framebuffer>& _dest);
	
	static void applyImpulse(const Ref<Framebuffer>& _dest, const glm::vec2& _position, float _value);
	
	static void applyBuoyancy(const Ref<Framebuffer>& _velocity, 
							  const Ref<Framebuffer>& _temperature, 
							  const Ref<Framebuffer>& _density, 
							  const Ref<Framebuffer>& _dest);

private:
	static uint32_t s_gridWidth;
	static uint32_t s_gridHeight;
	static float s_cellSize;
	static float s_densityRadius;

	static Ref<MeshShape> s_screenQuad;

	static Slab s_velocity;
	static Slab s_density;
	static Slab s_pressure;
	static Slab s_temperature;
	
	static Ref<Framebuffer> s_divergence;
	static Ref<Framebuffer> s_obstacles;

	static Ref<Shader> s_advectShader;
	static Ref<Shader> s_jacobiShader;
	static Ref<Shader> s_subtractGradientShader;
	static Ref<Shader> s_computeDivergenceShader;
	static Ref<Shader> s_densityShader;
	static Ref<Shader> s_buoyancyShader;
	static Ref<Shader> s_obstaclesShader;
	static Ref<Shader> s_fillShader;
	static Ref<Shader> s_fluidVisualizationShader;

	// initial conditions and parameters
	static float 		s_ambientTemperature;
	static float 		s_impulseTemperature;
	static float 		s_impulseDensity;
	static int 			s_numJacobiIterations;
	static float 		s_timeStep;
	static float 		s_smokeBuoyancy;
	static float 		s_smokeWeight;
	static float 		s_gradientScale;
	static float 		s_temperatureDissipation;
	static float 		s_velocityDissipation;
	static float 		s_densityDissipation;
	static glm::vec2	s_impulsePosition;


};
