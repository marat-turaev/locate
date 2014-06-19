#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <future>
#include <algorithm>
#include <iterator>
#include "data.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

static const size_t SORT_LIMIT = 100000;

using std::string;
using std::vector;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

void print_help(const po::options_description &desc) {
  std::cout << "Usage: updatedb --help | [--database-root DIR] [--output FILE]" << std::endl << desc << std::endl;
}

suffix_array::suffixes build_prefixes(fs::path const &path, size_t number) {
  suffix_array::suffixes result;
  std::string filename = path.filename().generic_string();

  for (size_t i = 0; i < filename.size(); ++i) {
    std::string suffix = filename.substr(i);
    result.push_back(std::make_pair(suffix, std::vector<size_t> {number}));
  }

  return result;
}

void compress(suffix_array::suffixes &suffixes) {
  std::cout << suffixes.size() << std::endl;
  for (int i = suffixes.size() - 1; i > 0 ; --i) {
    if (suffixes[i].first == suffixes[i - 1].first) {
      suffixes[i - 1].second.insert(suffixes[i - 1].second.end(), suffixes[i].second.begin(), suffixes[i].second.end());
    }
  }

  auto it = std::unique(suffixes.begin(), suffixes.end(), suffix_array::string_equals());
  suffixes.resize(std::distance(suffixes.begin(), it));
}

template<class Iterator, class Comp>
void parallel_sort(Iterator data, int length, int limit, Comp comparator) {
  if (length < limit) {
    std::sort(data, data + length, comparator);
  } else {
    auto future = std::async(parallel_sort<Iterator, Comp>, data, length / 2, limit, comparator);
    parallel_sort(data + length / 2, length / 2 + (length & 1), limit, comparator);
    future.wait();
    std::inplace_merge(data, data + length / 2, data + length, comparator);
  }
}

vector<fs::path> traverse_directory(fs::path &root) {
  vector<fs::path> paths;
  std::queue<fs::path> directories_queue;
  directories_queue.push(root);
  while (!directories_queue.empty()) {
    fs::path p = directories_queue.front();
    directories_queue.pop();

    for (fs::directory_iterator it(p), end; it != end; it++) {
      if (fs::is_directory(it->path())) {
        directories_queue.push(fs::canonical(it->path()));
      } else {
        paths.push_back(fs::canonical(it->path()));
      }
    }
  }
  return paths;
}

int main(int argc, char const *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Show help message")
  ("database-root,r", po::value<string>()->required()->default_value("."), "Root catalog for scanning")
  ("output,o", po::value<string>()->required()->default_value("index.db"), "File to save index to");
  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
      print_help(desc);
      return 1;
    }
    po::notify(vm);
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    print_help(desc);
    return 1;
  }

  string output_file_path = vm["output"].as<string>();
  std::ofstream output_file(output_file_path, std::ios::trunc);
  if (!output_file) {
    std::cerr << "Cannot open \"" << output_file_path << "\" for writing" << std::endl;
    return 1;
  }

  fs::path root(vm["database-root"].as<string>());
  if (!fs::exists(root)) {
    std::cerr << root << " doesn't exist" << std::endl;
    return 1;
  } else if (!fs::is_directory(root)) {
    std::cerr << root << " is not a directory" << std::endl;
    return 1;
  }

  vector<fs::path> paths = traverse_directory(root);

  suffix_array::suffixes suffixes;
  for (size_t i = 0; i < paths.size(); ++i) {
    auto prefixes = build_prefixes(paths[i], i);
    suffixes.insert(suffixes.end(), prefixes.begin(), prefixes.end());
  }

  parallel_sort(suffixes.begin(), suffixes.size(), SORT_LIMIT, suffix_array::sort_pred());

  output_file << paths << suffixes;
}