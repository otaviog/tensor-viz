#pragma once

#include <map>
#include <string>

#include <GLFW/glfw3.h>

#include "bounds.hpp"
#include "eigen_common.hpp"

namespace tenviz {
/**
 * Existing manipulators on TensorViz
 */
enum CameraManipulator {
  kWASD,     /**<Classic WASD+Mouse strife camera of
               games.*/
  kTrackBall /**< Mouse Trackbal camera.*/
};

/**
 * Base camera manipulator class. Derived classes should implement its own
 * parsing of GLFW's I/O and return the appropriate projection and
 * view matrices.
 */
class ICameraManipulator {
 public:
  /**
   * Reset the positioning and viewing of the camera to the canonical
   * view of an object.
   *
   * @param bounds The scene boundaries.
   */
  virtual void ResetView(const Bounds& bounds) = 0;

  /**
   * @return The OpenGL's projection matrix.
   */
  virtual Eigen::Matrix4f GetProjectionMatrix(int screen_width,
                                              int screen_height,
                                              const Bounds& bounds) = 0;

  /**
   * @return the OpenGL's view matrix.
   */
  virtual Eigen::Matrix4f GetViewMatrix() = 0;

  /**
   * Forces the view matrix to be a given one. Implementations may not
   * adjust its internal representation to reflect this matrix.
   * Meaning that the implementation should keep the view matrix until
   * no new user input is sent.
   *
   * @param view_matrix View matrix.
   */
  virtual void SetViewMatrix(const Eigen::Matrix4f& view_matrix) = 0;

  /**
   * Handle GLFW key state.
   *
   * @param keymap Map with current pressed keys. Never pressed key
   * are not inserted in the map.
   * @param elapsed Elapsed seconds since the last swap buffers.
   * @param bounds The scene boundary.
   */
  virtual void KeyState(const std::map<int, bool>& keymap, double elapsed_time,
                        const Bounds& bounds) = 0;

  /**
   * Handle GLFW key pressed
   *
   * @param window The rendering window.
   * @param glfw_key The key code.
   * @param elapsed_time Elapsed time in seconds between the last
   * frame.
   * @param bounds Scene's boundaries.
   */
  virtual void KeyPressed(GLFWwindow* window, int glfw_key, double elapsed_time,
                          const Bounds& bounds) = 0;

  /**
   * Handle GLFW mouse moved
   */
  virtual void CursorMoved(GLFWwindow* window, double xpos, double ypos) = 0;

  /**
   * Handle GLFW button pressed.
   */
  virtual void CursorButtonPressed(GLFWwindow* window, int glfw_button,
                                   int glfw_action, int glfw_mods) = 0;

  /**
   * Handle GLFW cursor scroll moved.
   */
  virtual void CursorScrollMoved(GLFWwindow* window, double xoffset,
                                 double yoffset) = 0;
};
}  // namespace tenviz
