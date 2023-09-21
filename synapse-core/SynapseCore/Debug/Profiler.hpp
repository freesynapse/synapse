
#pragma once


#include <chrono>

#include "../Core.hpp"


#ifdef DEBUG_PROFILING
#define PROFILE_SCOPE(_name) Syn::ProfilerTimer profilerTimer##__LINE__(_name)
#ifdef _MSC_VER
#define SYN_PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCSIG__)
#else
#define SYN_PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#endif
#else
#define PROFILE_SCOPE(_name)
#define SYN_PROFILE_FUNCTION()
#endif



namespace Syn {


    struct ProfilerResult
    {
	std::string m_name;
	long long m_start, m_end;
	uint32_t m_threadID;
    };


    struct ProfilerSession
    {
	std::string m_name;
    };


    class Profiler
    {
    public:
	Profiler() :
	    m_currentSessionID(nullptr), m_profileCount(0)
	{}

	//
	~Profiler()
	{
	    endSession();
	}

	//
	void beginSession(const std::string& _name, const std::string& _filepath="results.json")
	{
	    m_outputStream.open(_filepath);
	    writeHeader();
	    m_currentSessionID = new ProfilerSession{ _name };
	}

	//
	void endSession()
	{
	    writeFooter();
	    m_outputStream.close();
	    delete m_currentSessionID;
	    m_currentSessionID = nullptr;
	    m_profileCount = 0;
	}

	//
	void writeResult(const ProfilerResult& _result)
	{
	    if (m_profileCount++ > 0)
		m_outputStream << ",";

	    std::string name = _result.m_name;
	    std::replace(name.begin(), name.end(), '"', '\'');

	    m_outputStream << "{";
	    m_outputStream << "\"cat\":\"function\",";
	    m_outputStream << "\"dur\":" << (_result.m_end - _result.m_start) << ',';
	    m_outputStream << "\"name\":\"" << name << "\",";
	    m_outputStream << "\"ph\":\"X\",";
	    m_outputStream << "\"pid\":0,";
	    m_outputStream << "\"tid\":" << _result.m_threadID << ",";
	    m_outputStream << "\"ts\":" << _result.m_start;
	    m_outputStream << "}";

	    m_outputStream.flush();
	}

	//
	void writeHeader()
	{
	    m_outputStream << "{\"otherData\": {},\"traceEvents\":[";
	    m_outputStream.flush();
	}

	//
	void writeFooter()
	{
	    m_outputStream << "]}";
	    m_outputStream.flush();
	}

	// singleton instantiation
	static Profiler& get()
	{
	    static Profiler instance;
	    return instance;
	}


    private:
	ProfilerSession* m_currentSessionID = nullptr;
	std::ofstream m_outputStream;
	int m_profileCount = 0;

    };


    // scope-based timer, uses the static Profiler instance to output to .json.
    class ProfilerTimer
    {
    public:
	ProfilerTimer(const char* _name, bool _use_stdout=false) :
	    m_name(_name), m_stopped(false), m_useStdOut(_use_stdout)
	{
	    m_startTimePoint = std::chrono::high_resolution_clock::now();
	}

	//
	~ProfilerTimer()
	{
	    if (!m_stopped)
		stop();
	}

	//
	void stop()
	{
	    auto endTimePoint = std::chrono::high_resolution_clock::now();
	    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint).time_since_epoch().count();
	    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

	    uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
	    Profiler::get().writeResult({ m_name, start, end, threadID });

	    m_stopped = true;

	    if (m_useStdOut)
		std::cout << m_name << ": " << (double)(end - start) / 1000.0 << " ms.\n";
	}


    private:
	const char* m_name = nullptr;
	bool m_stopped = false;
	bool m_useStdOut = false;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimePoint;

    };

}


