
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <sstream>
#include <iomanip>

#include <Synapse.hpp>


class TClass
{
public:
	TClass(int _0, double _1, const std::string& _2) :
		var0(_0), var1(_1), var2(_2)
	{}

	void print() { std::cout << var0 << ", " << var1 << ", " << var2 << std::endl; }

private:
	int var0;
	double var1;
	std::string var2;
};


//
int main(int argc, char* argv[])
{
	int* p0 = SYN_ALLOCATE(int, 1000);
	int* p1 = SYN_ALLOCATE_N(int, 10000000);
	std::cout << *p0 << "\n";

	Syn::vector<TClass*> ptr_vec = SYN_VECTOR(TClass*);
	for (int i = 0; i < 20; i++)
		ptr_vec.push_back(SYN_ALLOCATE(TClass, i, (double)i, "test"+std::to_string(i)));
	for (int i = 0; i < 20; i++)
		SYN_DEALLOCATE(TClass, ptr_vec[i]);
	std::cout << "ptr vec size(): " << ptr_vec.size() << "\n";

	// vectors
	Syn::vector<int> vec0 = SYN_VECTOR(int); vec0.reserve(1024);
	Syn::vector<int> vec1 = SYN_VECTOR(int); vec1.reserve(1024);
	Syn::vector<int> vec2 = SYN_VECTOR(int); vec2.reserve(1024);
	Syn::vector<int> vec3 = SYN_VECTOR(int); vec3.reserve(1024);
	Syn::vector<int> vec4 = SYN_VECTOR(int); vec4.reserve(1024);
	Syn::vector<int> vec5 = SYN_VECTOR(int); vec5.reserve(1024);


	Syn::Ref<TClass> p2 = SYN_MAKE_REF(TClass, 10, 1002.3, "test");	
	p2->print();

	#ifdef DEBUG_MEMORY_ALLOC
	Syn::memory_log::print_alloc_all(true, true);
	#endif



	std::cout << "\n\n";
	return 0;
}

