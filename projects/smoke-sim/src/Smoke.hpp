
#include <SynapseAddons/Fluid/Grid2D.hpp>


using namespace Syn;


#define DEFAULT_VIEWPORT_WIDTH 	1532
#define DEFAULT_VIEWPORT_HEIGHT 1033


class Smoke : public Grid2D
{
public:
	Smoke(uint32_t _screen_width=DEFAULT_VIEWPORT_WIDTH, uint32_t _screen_height=DEFAULT_VIEWPORT_HEIGHT);
	~Smoke() {};

	void createObstacles(const Ref<Framebuffer>& _frame_buffer);

	void update(float _dt) override;
	void render(float _dt, float _width, float _height) override;

	// smoke computation functions
	void advect(const Ref<Framebuffer>& _velocity, 
				const Ref<Framebuffer>& _source, 
				const Ref<Framebuffer>& _obstacles, 
				const Ref<Framebuffer>& _dest,
				float _dissipation);
	
	void jacobi(const Ref<Framebuffer>& _pressure, 
				const Ref<Framebuffer>& _divergence, 
				const Ref<Framebuffer>& _obstacles, 
				const Ref<Framebuffer>& _dest);
	
	void subtractGradient(const Ref<Framebuffer>& _velocity, 
						  const Ref<Framebuffer>& _pressure, 
						  const Ref<Framebuffer>& _obstacles, 
						  const Ref<Framebuffer>& _dest);
	
	void computeDivergence(const Ref<Framebuffer>& _velocity, 
						   const Ref<Framebuffer>& _obstacles, 
						   const Ref<Framebuffer>& _dest);
	
	void applyImpulse(const Ref<Framebuffer>& _dest, const glm::vec2& _position, float _value);
	
	void applyBuoyancy(const Ref<Framebuffer>& _velocity, 
					   const Ref<Framebuffer>& _temperature, 
					   const Ref<Framebuffer>& _density, 
					   const Ref<Framebuffer>& _dest);

private:
	Ref<Shader> m_advectShader;
	Ref<Shader> m_jacobiShader;
	Ref<Shader> m_subtractGradientShader;
	Ref<Shader> m_computeDivergenceShader;
	Ref<Shader> m_densityShader;
	Ref<Shader> m_buoyancyShader;
	Ref<Shader> m_obstaclesShader;
	Ref<Shader> m_fillShader;
	Ref<Shader> m_fluidVisualizationShader;

	Slab m_velocity;
	Slab m_density;
	Slab m_pressure;
	Slab m_temperature;
	
	Ref<Framebuffer> m_divergence;
	Ref<Framebuffer> m_obstacles;

	// initial conditions and parameters
	float 		m_ambientTemperature;
	float 		m_impulseTemperature;
	float 		m_impulseDensity;
	int 		m_numJacobiIterations;
	float 		m_timeStep;
	float 		m_smokeBuoyancy;
	float 		m_smokeWeight;
	float 		m_gradientScale;
	float 		m_temperatureDissipation;
	float 		m_velocityDissipation;
	float 		m_densityDissipation;
	glm::vec2	m_impulsePosition;

};


