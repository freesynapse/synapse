
#include <memory>
#include <iostream>
#include <string.h>


//---------------------------------------------------------------------------------------
class TClass
{
public:
	TClass() {}
	TClass(uint32_t _l0, uint32_t _l1, uint32_t _l2) :
		m_payload_0(_l0), m_payload_1(_l1), m_payload_2(_l2)
	{} 

	void setPayload0(uint32_t _l) { m_payload_0 = _l; }
	void setPayload1(uint32_t _l) { m_payload_1 = _l; }
	void setPayload2(uint32_t _l) { m_payload_2 = _l; }
	uint32_t getPayload0() { return m_payload_0; }
	uint32_t getPayload1() { return m_payload_1; }
	uint32_t getPayload2() { return m_payload_2; }

	void print()
	{
		std::cout << "m_payload_0 = " << m_payload_0
				  << "\nm_payload_1 = " << m_payload_1
				  << "\nm_payload_2 = " << m_payload_2 << std::endl;
	}

private:
	uint32_t m_payload_0 = 0;
	uint32_t m_payload_1 = 0;
	uint32_t m_payload_2 = 0;

};


//---------------------------------------------------------------------------------------
template<typename T>
inline std::shared_ptr<T> shared_ptr_(const char* _c_fnc)
{ 
	std::cout << "<T>alloc: " << _c_fnc << "\n";
	return std::make_shared<T>();
}

template<typename T, typename ...Args>
inline std::shared_ptr<T> shared_ptr_(const char* _c_fnc, Args ...args)
{ 
	std::cout << "<T, Args...>alloc: " << _c_fnc << "\n";
	return std::make_shared<T>(args...);
}



// option 1: use a macro instead of the function call
#define SYN_MAKE_REF(typename, ...) shared_ptr_<typename>(__PRETTY_FUNCTION__, ##__VA_ARGS__)
// option 2: use a #define as the first argument when calling 

void func()
{
	std::shared_ptr<double> p = SYN_MAKE_REF(double);
	
}

//---------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	std::shared_ptr<TClass> p = SYN_MAKE_REF(TClass, 100, 100, 1);
	//std::shared_ptr<TClass> p = shared_ptr(TClass, 1, 2, 3);

	std::shared_ptr<int> p1 = SYN_MAKE_REF(int);
	p->print();

	func();

	char buffer[1024];
	memset(buffer, 0, 1024);
	const char* a = "fjdksfjskf";
	const char* b = "vncmxvnmcxvn,cx";
	const char* c = "yuewryewyrw";
	sprintf(buffer, "%s:%s:%s", a, b, c);
	std::string s(buffer);
	std::cout << "'" << s << "'\n";





	return 0;
}

