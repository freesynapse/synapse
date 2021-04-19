
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

	cout << "sizeof(VertexArray) = " << sizeof(VertexArray) << "\n";
	cout << "sizeof(IndexBuffer) = " << sizeof(IndexBuffer) << "\n";
	cout << "sizeof(VertexBuffer) = " << sizeof(VertexBuffer) << "\n";
	cout << "sizeof(Framebuffer) = " << sizeof(Framebuffer) << "\n";
	

	Log::close();

	return 0;
}

