set(GLFW_PATH CACHE PATH "GLFW LIBRARY PATH")

find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
  HINTS
  ${GLFW_PATH}/build/packages/glfw.3.2/build/native/include
  /usr/include
  /usr/local/include
  )

find_library(GLFW_LIBRARIES
  NAMES
  glfw3dll.lib  glfw
  HINTS
  ${GLFW_PATH}/build/native/lib/v140/x64/dynamic
  /usr/local/lib
  /usr/lib
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW DEFAULT_MSG
  GLFW_INCLUDE_DIR
  GLFW_LIBRARIES)
