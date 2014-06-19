#include <iostream>
#include <string>
#include <vector>

namespace suffix_array {

typedef std::pair<std::string, std::vector<size_t>> suffix; //(suffix, [path_id])
typedef std::vector<suffix> suffixes;

struct sort_pred {
  bool operator()(const suffix &left, const suffix &right) {
    return left.first < right.first;
  }
};

struct string_equals {
  bool operator()(const suffix &left, const suffix &right) {
    return left.first == right.first;
  }
};

}

std::ostream &operator<<(std::ostream &out, const suffix_array::suffix &suf);
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
  out << v.size() << ' ';
  for (auto it = v.begin(); it != v.end(); ++it) {
    out << *it << ' ';
  }
  return out;
}

template <typename T>
std::istream &operator>>(std::istream &in, std::vector<T> &v) {
  size_t count;
  in >> count;
  for (size_t i = 0; i < count; ++i) {
    T elem;
    in >> elem;
    v.push_back(elem);
  }
  return in;
}

std::ostream &operator<<(std::ostream &out, const suffix_array::suffix &suf) {
  out << "\"" << suf.first << "\" " << suf.second << std::endl;
  return out;
}

std::istream &operator>>(std::istream &in, suffix_array::suffixes &sufs) {
  size_t count;
  in >> count;
  while (count--) {
    std::string suffix;
    in.ignore(256, '"');
    std::getline(in, suffix, '"');
    std::vector<size_t> paths;
    in >> paths;
    sufs.push_back(std::make_pair(suffix, paths));
  }
  return in;
}
