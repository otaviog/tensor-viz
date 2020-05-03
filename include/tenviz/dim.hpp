#pragma once

namespace tenviz {
/* Specific class to hold size attributes of images.
 */
class Dim2 {
 public:
  /* Constructor.
   * @width target's width.
   * @height target's height.
   */
  Dim2(int width = 0, int height = 0) {
    width_ = width;
    height_ = height;
  }

  virtual ~Dim2() {}

  /* @return the target's width.
   */
  int get_width() const { return width_; }

  /* Sets the target's width.
   *
   * @width its value.
   */
  void set_width(int width) { width_ = width; }

  /* @return the target's height.
   */
  int get_height() const { return height_; }

  /* Sets the target's height.
   *
   * @height its value.
   */
  void set_height(int height) { height_ = height; }

  /* @return the area (width*height).
   */
  virtual int get_area() const { return width_ * height_; }

 private:
  int width_, height_;
};

/* Specific class to hold size attributes of volumes.
 */
class Dim3 : public Dim2 {
 public:
  /* Constructor
   * @width target's width.
   * @height target's height.
   * @depth target's depth.
   */
  Dim3(int width = 0, int height = 0, int depth = 0) : Dim2(width, height) {
    depth_ = depth;
  }

  /* @return the target's depth.
   */
  int get_depth() const { return depth_; }

  /* Sets the target's depth.
   * @depth its new value.
   */
  void set_depth(int depth) { depth_ = depth; }

  /* @return converted to dim2 by removing the depth.
   */
  Dim2 to_dim2() const { return Dim2(get_width(), get_height()); }

  /* @return the volume area.
   */
  int get_area() const override { return Dim2::get_area() * depth_; }

 private:
  int depth_;
};

inline bool operator==(const Dim2 &lfs, const Dim2 &rhs) {
  return lfs.get_width() == rhs.get_width() &&
         lfs.get_height() == rhs.get_height();
}

inline bool operator==(const Dim3 &lfs, const Dim3 &rhs) {
  return lfs.get_width() == rhs.get_width() &&
         lfs.get_height() == rhs.get_height() &&
         lfs.get_depth() == rhs.get_depth();
}

inline bool operator!=(const Dim2 &lfs, const Dim2 &rhs) {
  return !(lfs == rhs);
}

inline bool operator!=(const Dim3 &lfs, const Dim3 &rhs) {
  return !(lfs == rhs);
}
}
