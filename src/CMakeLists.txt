cuda_add_library(tenviz
  gl_texture.cpp
  gl_error.cpp
  gl_buffer.cpp
  gl_buffer.cu
  gl_shader.cpp
  gl_shader_program.cpp
  gl_framebuffer.cpp
  error.cpp
  cuda_error.cpp
  projection.cpp
  camera.cpp
  math.cpp
  viewer.cpp
  read_file.cpp
  context.cpp
  context_resource.cpp
  scene.cpp
  bbox.cpp
  bsphere.cpp
  bounds.cpp
  bounds_glrender.cpp
  trackball_camera_manipulator.cpp
  wasd_camera_manipulator.cpp
  time_measurer.cpp
  draw_program.cpp
  style.cpp
  anode.cpp
  so3.cpp
  se3.cpp
  pose.cpp
  _ctenviz.cpp
  )

target_include_directories(tenviz
  PUBLIC
  $<BUILD_INTERFACE:${EIGEN3_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${IGL_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${GLFW_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${GLEW_INCLUDE_DIRS}>
  $<BUILD_INTERFACE:${PYTHON_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/tenviz>
  ${CMAKE_SOURCE_DIR}/3rd-party/Sophus)

if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
  set(OS_LIBRARIES pthread)
endif ()

target_link_libraries(tenviz
  ${Boost_LIBRARIES}
  ${GLFW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${PYTHON_LIBRARY}
  ${OS_LIBRARIES}
  "${TORCH_LIBRARIES}")

 if (MSVC)
	target_compile_options(tenviz PRIVATE /bigobj)
	target_compile_options(tenviz PRIVATE $<$<NOT:$<CONFIG:Debug>>:$<$<COMPILE_LANGUAGE:CXX>:/MP>>)
endif (MSVC)

set_property(TARGET tenviz PROPERTY CXX_STANDARD 17)

set_target_properties(tenviz
  PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

if (CMAKE_COMPILER_IS_GNUXX)
  target_compile_options(tenviz PUBLIC -Wdeprecated-declarations -Wall)
endif (CMAKE_COMPILER_IS_GNUXX)

target_compile_definitions(tenviz PUBLIC $<$<BOOL:${MSVC}>:BOOST_ALL_NO_LIB>)

add_library(_ctenviz SHARED _ctenviz.cpp)
set_property(TARGET _ctenviz PROPERTY CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUXX)
  target_compile_options(_ctenviz PUBLIC -Wdeprecated-declarations -Wall)
endif (CMAKE_COMPILER_IS_GNUXX)

target_link_libraries(_ctenviz tenviz)
target_compile_definitions(_ctenviz PUBLIC $<$<BOOL:${MSVC}>:BOOST_ALL_NO_LIB>)

set_target_properties(_ctenviz PROPERTIES PREFIX "")
if (SKBUILD)
  python_extension_module(_ctenviz)
  install(TARGETS _ctenviz LIBRARY DESTINATION tenviz)
else (SKBUILD)
  set_target_properties(_ctenviz PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tenviz)
endif (SKBUILD)

if (TENSORVIZ_BUILD_TESTS)
  add_subdirectory(test)
endif (TENSORVIZ_BUILD_TESTS)
