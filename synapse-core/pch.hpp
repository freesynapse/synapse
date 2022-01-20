
#pragma once

// basic utility
//
#include <iostream>
#include <algorithm>
#include <functional>
#include <thread>
#include <utility>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <fstream>
#include <chrono>
#include <ctime>
#include <future>
#include <atomic>

// memory
//
#include <memory>
#ifdef __linux__
#include <malloc.h>
#endif

// data structures
//
#include <string>
#include <vector>
#include <map>
#include <array>
//#include <boost/unordered_map.hpp>
#include <unordered_map>
#include <set>
#include <unordered_set>

// windows API
//
#ifdef _WIN32
#include <Windows.h>
#endif

// OpenGL API
//
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glad/glad.h>
#ifdef _WIN32
#include <GL/glew.h>
#endif


