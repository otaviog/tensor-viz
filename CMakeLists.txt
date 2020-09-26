cmake_minimum_required(VERSION 3.8)
project(tensorviz LANGUAGES C CXX)

option(TENSORVIZ_BUILD_TESTS OFF "Whatever to build tests")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

###############
# Find PyTorch


find_package(PythonInterp 3 REQUIRED)
execute_process(
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_torch_dir.py
  OUTPUT_VARIABLE FOUND_TORCH_DIR)
string(STRIP ${FOUND_TORCH_DIR} FOUND_TORCH_DIR)
set(Torch_DIR ${FOUND_TORCH_DIR})

find_package(Torch REQUIRED)
find_library(TORCH_PYTHON_LIBRARY torch_python PATHS "${TORCH_INSTALL_PREFIX}/lib")

######################
# Find other packages

find_package(Eigen3 REQUIRED)
find_package(PythonLibs 3 REQUIRED)
if (SKBUILD)
  find_package(PythonExtensions REQUIRED)
endif (SKBUILD)
find_package(Boost REQUIRED filesystem)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW REQUIRED)

if (NOT WIN32)
  execute_process(
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_torch_cxx11_abi.py
	OUTPUT_VARIABLE TORCH_CXX11_ABI)
  string(STRIP ${TORCH_CXX11_ABI} TORCH_CXX11_ABI)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=${TORCH_CXX11_ABI})  
  set(CMAKE_CXX_FLAGS -fPIC)
endif()

add_subdirectory(src)
