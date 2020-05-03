#include "read_file.hpp"
#include <boost/filesystem/operations.hpp>
#include <cstring>
#include <fstream>

using namespace std;

namespace tenviz {
namespace readfile {
FileData ReadFile(const string &filename, bool text_file) {
  ifstream infile(filename.c_str());
  FileData contents;

  if (infile.good()) {
    size_t size = size_t(boost::filesystem::file_size(filename));

    contents = ReadStream(infile, text_file);
  }

  infile.close();

  return contents;
}

FileData ReadStream(istream &stream, bool text_file) {
  stream.seekg(0, ios::end);

  const size_t length = size_t(stream.tellg());
  FileData contents;

  if (length == 0) return contents;

  stream.seekg(0, ios::beg);
  if (text_file) {
    contents.data = new char[length + 1];
    memset(contents.data, '\0', length + 1);
  } else {
    contents.data = new char[length];
  }

  stream.read(contents.data, length);
  contents.size = length;

  return contents;
}
}  // namespace readfile
}  // namespace tenviz
