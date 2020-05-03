#pragma once

#include <istream>
#include <string>

namespace tenviz {
namespace readfile {
class FileData {
 public:
  size_t size;
  char *data;

  FileData() {
    data = NULL;
    size = 0;
  }

  void Dispose() { delete[] data; }

  bool is_null() const { return data == NULL; }
};

class ScopedFileData {
 public:
  size_t size;
  char *data;

  ScopedFileData(FileData file_data) : file_data_(file_data) {
    data = file_data_.data;
    size = file_data_.size;
  }

  void Dispose() { file_data_.Dispose(); }

  bool is_null() const { return file_data_.is_null(); }

 private:
  FileData file_data_;
};

FileData ReadFile(const std::string &filename, bool text_file = false);

FileData ReadStream(std::istream &stream, bool text_stream = false);

inline FileData ReadTextFile(const std::string &filename) {
  return ReadFile(filename, true);
}

}  // namespace readfile
}  // namespace tenviz
