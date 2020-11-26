#pragma once


// basic utility
//
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <utility>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <fstream>
#include <chrono>
#include <ctime>
#include <future>


// data structures
//
#include <string>
#include <vector>
#include <map>
#include <array>
#include <unordered_map>
#include <set>
#include <unordered_set>


// Windows API
//
#ifdef _WIN64
#include <Windows.h>
#endif

// OpenGL API
//
#include <glm/glm.hpp>
#include <glad/glad.h>
#ifdef _WIN64
#include <GL/glew.h>
#endif



