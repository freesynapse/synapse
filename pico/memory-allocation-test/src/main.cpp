
#include <vector>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>

#include "Allocator.hpp"
#include "header.hpp"


using namespace Syn;



// shared pointer example

//
class testClass
{
public:
	testClass(int a, double b, const std::string& c) :
		m_a(a), m_b(b), m_c(c)
	{}

	void print() { std::cout << "a=" << m_a << ", b=" << m_b << ", c='" << m_c << "'.\n"; }
public:
	int m_a;
	double m_b;
	std::string m_c;
};


int main(int argc, char* argv[])
{
	/*
	// test explicit ------------------------------------
	int* i = SYN_ALLOCATE(int, 1000); //allocate_n<int>(1000);
	std::cout << *i << "\n";
	testClass* pTestClass = SYN_ALLOCATE(testClass, 10, 11.2, "hejsan"); //allocate<testClass>(10, 11.2, "hejsan");
	pTestClass->print();
	double* d = SYN_ALLOCATE(double, 10.2); //allocate<double>(10.2);
	std::cout << *d << "\n";
	uint32_t* puint = SYN_ALLOCATE(uint32_t);
	*puint = 42;
	std::cout << *puint << "\n";
	uint32_t* puint1 = SYN_ALLOCATE_N(uint32_t, 1000000000);

	SYN_DEALLOCATE(testClass, pTestClass); //deallocate<testClass>(pTestClass);
	SYN_DEALLOCATE(double, d); //deallocate<double>(d);
	SYN_DEALLOCATE_N(uint32_t, puint1);

	// test STL -----------------------------------------
	std::cout << "\n";
	{
		Syn::vector<int> vec = SYN_VECTOR(int);
		int n = 65;
		for (int i = 0; i < n; i++)
			vec.push_back(i);
	}

	Syn::vector<double> vec1 = SYN_VECTOR(double);
	for (int i = 0; i < 1000; i++)
		vec1.push_back(i);
	std::cout << "scoped vec<double> size=" << vec1.size() << "\n";

	// test std::shared_ptr -----------------------------
	std::cout << "\n";
	Syn::Ref<int> p = SYN_MAKE_REF(int);
	Syn::Ref<testClass> p1 = SYN_MAKE_REF(testClass, 100, 10010.1, "apa");
	p1->print();
	std::cout << "\n";

	int* p0 = SYN_ALLOCATE_N(int, 10000000);
	std::cout << "pointer reserved memory = " << (float)malloc_size_func((void*)p0) / (float)(1024*1024) << " Mb.\n";
	int n = 10000;
	{
	Syn::vector<int> vector0 = SYN_VECTOR(int);
	vector0.reserve(n);
	for (int i = 0; i < n; i++)
		vector0[i] = i;
	for (auto& i : vector0)
		std::cout << i << " ";
	std::cout << "\n\n";
	}

	SYN_DEALLOCATE_N(int, p0);
	*/
	Syn::vector<int> vec = SYN_VECTOR(int);
	std::cout << vec.size() << "\n";
	for (int i = 0; i < 20; i++)
		vec.push_back(i);
	Syn::vector<double> vec1 = SYN_VECTOR(double); vec1.reserve(10);

	Syn::map<int, std::string> map0 = SYN_MAP(int, std::string);
	map0[0] = "1000";
	map0[10] = "apaspdksafkdskfjksfjkajfkdsf";

	Syn::unordered_map<int, std::string> umap0 = SYN_UNORDERED_MAP(int, std::string);
	umap0[90] = "kdflsfd";
	umap0[32] = "jkdsöadskf";

	Syn::list<int> 
	list0 = SYN_LIST(int);
	list0.push_back(100);
	list0.push_back(1);

	std::cout << "\nfootprint of STL allocation tracking: " << s_STLMemRsrcHandler.getMemSize() << " bytes.\n";

	std::cout << "\n\n";
	
	int* pint = SYN_ALLOCATE(int, 100);
	int* pint2 = SYN_ALLOCATE_N(int, 12);
	int* pint3 = SYN_ALLOCATE(int);
	
	test_fnc0();
	test_fnc1();


	memory_log::print_alloc_all(false, true);
		std::cout << "\n\n";

}



