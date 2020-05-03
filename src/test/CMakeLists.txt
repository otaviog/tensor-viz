include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
  ${CMAKE_SOURCE_DIR}/3rd-party/Catch2)

add_executable(test_tensorviz_cpp
  test_camera.cpp
  catch_main.cpp)
set_property(TARGET test_tensorviz_cpp PROPERTY CXX_STANDARD 17)
target_link_libraries(test_tensorviz_cpp tenviz "${TORCH_LIBRARIES}")
