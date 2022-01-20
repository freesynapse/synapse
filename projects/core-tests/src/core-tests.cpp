

#include <Synapse.hpp>
#include <SynapseAddons.hpp>

#include <fstream>
#include <cstdio>

#include <muparser/muParser.h>
#include <muparser/muParserBase.h>
#include "noise_test.hpp"
//#include "PyHelper.hpp"		-- now in ./bak/ and moved to SynapseAddons and SynapseTypes
//#include "PyFunction3D.hpp"	-- now in ./bak/ and moved to SynapseAddons and SynapseTypes

using namespace std;
using namespace Syn;

double MySqr(double a_fVal) {  return a_fVal*a_fVal; }

void testMuParser()
{
	const std::string expression_string =
						"sin(2 * _pi * x) + cos(x / 2 * _pi)+t";

	/*
	try
	{
		Syn::ProfilerTimer timer("eval", true);
		double x = 0, t = 1, y = 0;
		mu::Parser p;
		p.DefineVar("x", &x);
		p.DefineVar("t", &t);
		p.DefineVar("y", &y);
		//p.SetExpr("sin(2 * _pi * x) + cos(x / 2 * _pi) + t");
		//p.SetExpr("0");
		p.SetExpr(expression_string);

		for (double _a=-5; _a<5; _a+=0.1)
		{
			x = _a;  // Change value of variable a
			double e = p.Eval();
			std::cout << e << std::endl;
		}
	}
	catch (mu::Parser::exception_type &e)
	{
		std::cout << e.GetMsg() << std::endl;
	}
	*/
}
//---------------------------------------------------------------------------------------
void testNoise()
{
	int dim = 1024;
	float* noise = new float[dim*dim];

	int os[] = { 1, 2, 3, 4, 5, 6, 7 };
	float as[] = { 1.0f, 4.0f, 16.0f };
	float fs[] = { 0.01f, 0.05f, 0.10f, 0.5f };
	int os_n = sizeof(os) / sizeof(int);
	int as_n = sizeof(as) / sizeof(float);
	int fs_n = sizeof(fs) / sizeof(float);

	std::ofstream fout;
	fout.open("./noise_ranges.txt", std::ios::out);

	for (int i = 0; i < os_n; i++)
	{
		printf("octave %d\n", os[i]);
		for (int j = 0; j < as_n; j++)
		{
			printf("\tamplitude %.2f\n", as[j]);
			for (int k = 0; k < fs_n; k++)
			{
				printf("\t\tfrequency %.2f\n", fs[k]);

				Noise::set_param_octave_count(os[i]);
				Noise::set_param_base_amp(as[j]);
				Noise::set_param_base_frequency(fs[k]);

				float nmin = 1000.0f, nmax = -1000.0f;
				for (int x = 0; x < dim; x++)
				{
					for (int y = 0; y < dim; y++)
					{
						float n = Noise::fbm_perlin2(glm::vec2(x, y));
						nmin = MIN(n, nmin);
						nmax = MAX(n, nmax);
						noise[y * dim + x] = n;
					}
				}
				std::string fname = "O_" + std::to_string(os[i]) + "__A_" + std::to_string(as[j]) + "__F_" + std::to_string(fs[k]) + ".png";
				fout << "octaves: " << std::to_string(os[i]);
				fout <<  ", amplitude: " << std::to_string(as[j]);
				fout << ", frequency: " << std::to_string(fs[k]) << " -- RANGE = [ " << std::to_string(nmin) << " ... " << std::to_string(nmax) << " ]";
				fout << " (nmax - nmin) = " << std::to_string(nmax - nmin) << '\n';
				//Noise::save_noise_PNG(fname, dim, dim, noise);
			}
		}
	}

	fout.close();
	delete[] noise;
}
//---------------------------------------------------------------------------------------
void testNormalizedNoise()
{	
	VxPerlin noise;
	int dim = 16;
	float step = 1.0f / (float)dim;
	glm::vec3 offset = glm::vec3(0.0f);

	// plain normalized Perlin noise
	FILE* file0 = fopen("./noise_ranges2_eval_norm.txt", "w");
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			float nmin = 100.0f, nmax = -100.0f;
			for (int x = 0; x < dim; x++)
				for (int y = 0; y < dim; y++)
					for (int z = 0; z < dim; z++)
					{
						glm::vec3 p = glm::vec3(x, y, z) * step + offset;
						float n = noise.eval3(p);
						nmin = MIN(n, nmin);
						nmax = MAX(n, nmax);
					}
			fprintf(file0, "%1.3f,%1.3f,%1.3f\n", nmin, nmax, nmax-nmin);
			offset.y += 1.0f;
		}
		offset.x += 1.0f;
	}
	fclose(file0);

	// fractal Perlin noise, same dimensions, offsets and step sizes
	FILE* file1 = fopen("./noise_ranges2_fbm.txt", "w");
	offset = glm::vec3(0.0f);
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			float nmin = 100.0f, nmax = -100.0f;
			for (int x = 0; x < dim; x++)
				for (int y = 0; y < dim; y++)
					for (int z = 0; z < dim; z++)
					{
						glm::vec3 p = glm::vec3(x, y, z) * step + offset;
						float n = noise.fbm3(p);
						nmin = MIN(n, nmin);
						nmax = MAX(n, nmax);
					}
			fprintf(file1, "%1.3f,%1.3f,%1.3f\n", nmin, nmax, nmax-nmin);
			offset.y += 1.0f;
		}
		offset.x += 1.0f;
	}
	fclose(file1);

	// plain normalized Perlin noise
	FILE* file2 = fopen("./noise_ranges2_eval_non_norm.txt", "w");
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			float nmin = 100.0f, nmax = -100.0f;
			for (int x = 0; x < dim; x++)
				for (int y = 0; y < dim; y++)
					for (int z = 0; z < dim; z++)
					{
						glm::vec3 p = glm::vec3(x, y, z) * step + offset;
						float n = noise.eval3_non_norm(p);
						nmin = MIN(n, nmin);
						nmax = MAX(n, nmax);
					}
			fprintf(file2, "%1.3f,%1.3f,%1.3f\n", nmin, nmax, nmax-nmin);
			offset.y += 1.0f;
		}
		offset.x += 1.0f;
	}
	fclose(file2);

}
//---------------------------------------------------------------------------------------
void testFractionalNoise()
{
	VxPerlin noise;
	int dim = 16;
	float step = 1.0f / (float)dim;
	glm::vec3 offset = glm::vec3(0.0f);

	int octaves[] = { 1, 3, 5, 7, 9 };
	int on = 5;

	// test different octaves
	for (int o = 0; o < on; o++)
	{
		noise.set_octave_count(octaves[o]);

		std::string fname = "./noise_ranges2_fbm" + std::to_string(octaves[o]) + ".txt";
		FILE* file = fopen(fname.c_str(), "w");

		for (int i = 0; i < 100; i++)
		{
			for (int j = 0; j < 100; j++)
			{
				float nmin = 100.0f, nmax = -100.0f;
				for (int x = 0; x < dim; x++)
					for (int y = 0; y < dim; y++)
						for (int z = 0; z < dim; z++)
						{
							glm::vec3 p = glm::vec3(x, y, z) * step + offset;
							float n_ = noise.eval3(p);
							nmin = MIN(n_, nmin);
							nmax = MAX(n_, nmax);
						}
				fprintf(file, "%1.3f,%1.3f,%1.3f\n", nmin, nmax, nmax-nmin);
				offset.y += 1.0f;
			}
			offset.x += 1.0f;
		}
		fclose(file);
	}

}
//---------------------------------------------------------------------------------------
void exportNoise()
{
	int dim = 512;
	float step = 1.0f / (float)dim;
	float* n = new float[dim*dim];
	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
			n[y*dim+x] = VxNoise::perlin3_fbm(glm::vec3(x, y, 0)*step);

	VxNoise::save_noise_PNG("./512x512_norm_fbm_5_rot.png", dim, dim, n);
	delete[] n;

	dim = 1024;
	step = 1.0f / (float)dim;
	n = new float[dim*dim];
	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
			n[y*dim+x] = VxNoise::perlin3_fbm(glm::vec3(x, y, 0)*step);

	VxNoise::save_noise_PNG("./1024x1024_norm_fbm_5_rot.png", dim, dim, n);
	delete[] n;

	dim = 2048;
	step = 1.0f / (float)dim;
	n = new float[dim*dim];
	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
			n[y*dim+x] = VxNoise::perlin3_fbm(glm::vec3(x, y, 0)*step);

	VxNoise::save_noise_PNG("./2048x2048_norm_fbm_5_rot.png", dim, dim, n);
	delete[] n;
}
//---------------------------------------------------------------------------------------
void testPerlin2()
{
	VxPerlin noise;
	int dim = 512;
	float n[dim*dim];
	float step = 1.0f / (float)dim;
	
	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
			n[y*dim + x] = noise.fbm2(x*step, y*step);
	
	Noise::save_noise_PNG("./perlin2_512x512.png", dim, dim, n);

}
//---------------------------------------------------------------------------------------
void testValue2()
{
	SYN_CORE_TRACE("");

	int dim = 2048;
	float* n = new float[dim*dim];
	float step = 1.0f / (float)dim;

	for (int y = 0; y < dim; y++)
		for (int x = 0; x < dim; x++)
			n[y*dim + x] = VxNoise::value3_fbm_r(x*step, y*step, 0.0f);
	
	VxNoise::save_noise_PNG("./value2_rot_2048x2048.png", dim, dim, n);
	delete[] n;
}
//---------------------------------------------------------------------------------------
void testValue2Range()
{
	SYN_CORE_TRACE("");

	FILE* file0 = fopen("./value_range_eval.txt", "w");
	FILE* file1 = fopen("./value_range_fbm_5.txt", "w");
	int dim = 2048;
	float step = 1.0f / dim;
	for (int k = 0; k < 200; k++)
	{
		printf("%d\n", k);
		fflush(NULL);
		float n0min=10, n0max=-10;
		float n1min=10, n1max=-10;
		for (int y = 0; y < dim; y++)
			for (int x = 0; x < dim; x++)
			{
				float n0 = VxNoise::value2(x*step, y*step);
				float n1 = VxNoise::value2_fbm_r(x*step, y*step);
				n0min = MIN(n0, n0min);
				n0max = MAX(n0, n0max);
				n1min = MIN(n1, n1min);
				n1max = MAX(n1, n1max);
			}
		fprintf(file0, "%1.3f,%1.3f,%1.3f\n", n0min, n0max, n0max-n0min);
		fprintf(file1, "%1.3f,%1.3f,%1.3f\n", n1min, n1max, n1max-n1min);
	}
	printf("\n");
	fclose(file0);
	fclose(file1);
}
//---------------------------------------------------------------------------------------
void testValueVsPerlinSpeed()
{
	SYN_CORE_TRACE("");

	int dim = 2048;
	float* p = new float[dim*dim];
	float* v = new float[dim*dim];
	float sp = 1.0f/(float)dim;
	float sv = 1.0f/(float)dim;
	{
		Timer t("perlin noise", true);
		for (int y = 0; y < dim; y++)
			for (int x = 0; x < dim; x++)
				p[y*dim+x] = VxNoise::perlin3_fbm(x*sp, y*sp, 0.0f);
	}

	{
		Timer t("value noise", true);
		for (int y = 0; y < dim; y++)
			for (int x = 0; x < dim; x++)
				v[y*dim+x] = VxNoise::value3_fbm_r(x*sv, y*sv, 0.0f);
	}

	VxNoise::save_noise_PNG("./speed_test_perlin2_fbm_5.png", dim, dim, p);
	VxNoise::save_noise_PNG("./speed_test_value2_fbm_5.png", dim, dim, v);

	delete[] p;
	delete[] v;
}
//---------------------------------------------------------------------------------------
void testChunkValue()
{
	FILE* file = fopen("./chunk_noise_ranges.txt", "w");
	
	float step = 1.0f / (float)(1<<23);
	VxNoise::set_frequency_value(5.0f * (float)(1<<16));

	float n;
	float nmin_chunk, nmax_chunk;
	float world_offset = 1e5;
	int chunk_radius = 10;
	for (int c_x = -chunk_radius; c_x < chunk_radius; c_x++)
	{
		printf(".");
		fflush(NULL);
		for (int c_y = -chunk_radius; c_y < chunk_radius; c_y++)
		{
			for (int c_z = -chunk_radius; c_z < chunk_radius; c_z++)
			{
				//printf("%d %d %d\n", c_x, c_y, c_z);
				nmin_chunk =  10000000.0f;
				nmax_chunk = -10000000.0f;
				// chunk offset
				glm::vec3 offset = { c_x * VX_CHUNK_SIZE_XZ_F + world_offset,
									 c_x * VX_CHUNK_SIZE_Y_F  + world_offset,
									 c_z * VX_CHUNK_SIZE_XZ_F + world_offset };

				VX_FOR_BEGIN_DEF
					glm::vec3 p = glm::vec3(x, y, z) + offset;
					n = VxNoise::value3_fbm_r(p * step);
					//printf("(%d %d %d):(%.0f, %.0f, %.0f) %.1f\n", c_x, c_y, c_z, p.x, p.y, p.z, n);
					nmin_chunk = MIN(nmin_chunk, n);
					nmax_chunk = MAX(nmax_chunk, n);
				VX_FOR_END

				fprintf(file, "%1.3f,%1.3f,%1.3f\n", nmin_chunk, nmax_chunk, nmax_chunk-nmin_chunk);
			}
		}
	}
	printf("\n");
	fclose(file);

}
//---------------------------------------------------------------------------------------
// RNG TESTS
constexpr int N = 100000000;
constexpr float Nf = (float)N;
float testRand()
{
	Timer timer("", false);

	for (int i = 0; i < N; i++)
		int r = Random::rand_i();
	
	return timer.getDeltaTimeMs();
}
//---------------------------------------------------------------------------------------
float testXorShift64s(uint64_t _state)
{
	Timer timer("", false);

	uint64_t x;
	for (int i = 0; i < N; i++)
	{
		x = _state;
		x ^= x >> 12;
		x ^= x << 25;
		x ^= x >> 27;
		_state = x;
		x *= 0x2545F4914F6CDD1DULL;	// this would be returned
	}

	return timer.getDeltaTimeMs();
}
//---------------------------------------------------------------------------------------
typedef struct xoshiro256ss_state_s 
{	
	uint64_t s[4];
	xoshiro256ss_state_s() { s[0] = 101; s[1] = 201; s[2] = 301; s[3] = 413; }
} xoshiro256ss_state;
uint64_t rol64(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
//---------------------------------------------------------------------------------------
float testXoshiro256ss(xoshiro256ss_state* _state)
{
	Timer timer("", false);
	uint64_t* s = _state->s;
	for (int i = 0; i < N; i++)
	{
		uint64_t result = rol64(s[1] * 5, 7) * 9;
		// update state
		uint64_t t = s[1] << 17;

		s[2] ^= s[0];
		s[3] ^= s[1];
		s[1] ^= s[2];
		s[0] ^= s[3];

		s[2] ^= t;
		s[3] = rol64(s[3], 45);

		// return result;
	}

	return timer.getDeltaTimeMs();
}
//---------------------------------------------------------------------------------------
constexpr int BIT_NOISE1 = 0x68E31DA4;
constexpr int BIT_NOISE2 = 0xB5297A4D;
constexpr int BIT_NOISE3 = 0x1B56C4E9;
float testSquirrel3(uint32_t _pos, uint32_t _seed)
{
	Timer timer("", false);

	for (int i = _pos; i < N + _pos; i++)
	{
		uint32_t m = i;
		m *= BIT_NOISE1;
		m += _seed;
		m ^= (m >> 8);
		m += BIT_NOISE2;
		m ^= (m << 8);
		m *= BIT_NOISE3;
		m ^= (m >> 8);
		// return m;
	}

	return timer.getDeltaTimeMs();
}
//---------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	Log::open("./log.txt");
	Log::output_new_line(true);
	Log::use_stdout(true);

	VxNoise::init();
	Random::init();

	PyWrapper::init();
	PyWrapper::shutdown();
	
	long double* ld = new long double;
	*ld = 1.0;
	double* d = new double;
	*d = 2.0;
	float* f = new float;
	*f = 3.0f;

	printf("long double* = %Lf (%lu)\n", *ld, sizeof(long double*));
	printf("double* = %f (%lu)\n", *d, sizeof(double*));
	printf("float* = %f (%lu)\n", *f, sizeof(float*));
	printf("float = %f (%lu)\n", *f, sizeof(float));
	printf("\n\n");

	double f_d_static = static_cast<double>(*f);
	long double f_ld_static = static_cast<long double>(*f);

	printf("f -> ld static : %f -> %Lf\n", *f, f_ld_static);
	printf("f -> d static : %f -> %f\n", *f, f_d_static);

	double* f_d_ptr_dynamic = reinterpret_cast<double*>(f);

	printf("f ptr-> d ptr reinterpret : %f -> %f\n", *f, *f_d_ptr_dynamic);

	delete ld;
	delete d;


	//float t;
	//t = testRand();
	//printf("rand()         runs per ms: %10.0f (%.2f ms)\n", Nf/t, t);
	//t = testXorShift64s(858274389);
	//printf("xorshift64s()  runs per ms: %10.0f (%.2f ms)\n", Nf/t, t);
	//xoshiro256ss_state x256ss;
	//t = testXoshiro256ss(&x256ss);
	//printf("xoshiro256ss() runs per ms: %10.0f (%.2f ms)\n", Nf/t, t);
	//t = testSquirrel3(0, 1024853831);
	//printf("Squirrel3()    runs per ms: %10.0f (%.2f ms)\n", Nf/t, t);

	//testNoise();
	//testNormalizedNoise();
	//exportNoise();
	//testFractionalNoise();
	//testPerlin2();
	//testValue2();
	//testValue2Range();
	//testValueVsPerlinSpeed();
	//testChunkValue();
	//int s = 16;
	//printf("2^23 = 8388608\n");
	//printf("1<<%d = %d *5 = %d\n", s, (1<<s), 5*(1<<s));
	

	Log::close();

	return 0;

}

