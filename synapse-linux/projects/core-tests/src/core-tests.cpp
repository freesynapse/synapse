
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <sstream>
#include <iomanip>

#include <Synapse.hpp>

using namespace std;
using namespace Syn;



int main(int argc, char* argv[])
{
	Log::open("./log.txt");
	Log::output_new_line(true);

	AABB aabb = { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } };
	Log::debug_vector("", "min", aabb.min);
	Log::debug_vector("", "max", aabb.max);

	Log::close();

	return 0;
}

