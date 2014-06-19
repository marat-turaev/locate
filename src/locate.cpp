#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "data.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

using std::string;
using std::vector;

namespace po = boost::program_options;
namespace fs = boost::filesystem;

void print_help(const po::options_description &desc) {
  std::cout << "Usage: locate --help | [--database FILE] [-p] PATTERN" << std::endl << desc << std::endl;
}

int main(const int argc, const char *argv[]) {
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Show help message")
  ("database,d", po::value<string>()->required()->default_value("index.db"), "Database file")
  ("pattern,p", po::value<string>()->required(), "Search pattern");
  po::positional_options_description pdesc;
  pdesc.add("pattern", 1);

  po::variables_map vm;

  try {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vm);
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

  string input_file_path = vm["database"].as<string>();
  std::ifstream input_file(input_file_path);
  if (!input_file) {
    std::cerr << "Cannot open \""  << input_file_path << "\" for reading" << std::endl;
    return 1;
  }


  vector<fs::path> paths;
  suffix_array::suffixes suffixes;
  input_file >> paths >> suffixes;

  const string pattern = vm["pattern"].as<string>();
  auto from = std::lower_bound(suffixes.begin(), suffixes.end(), pattern, [](suffix_array::suffix const & left , string const & right) {
    return left.first < right;
  });

  auto to = std::upper_bound(suffixes.begin(), suffixes.end(), pattern, [](string const & left, suffix_array::suffix const & right ) {
    return !boost::starts_with(right.first, left) && left < right.first;
  });


  std::vector<size_t> path_ids;
  std::for_each(from, to, [&path_ids](suffix_array::suffix const & suffix) {
    path_ids.insert(path_ids.end(), suffix.second.begin(), suffix.second.end());
  });

  vector<fs::path> found_paths;
  for (auto path_id : path_ids) {
    if (fs::exists(paths[path_id])) {
      found_paths.push_back(paths[path_id]);
    }
  }

  std::sort(found_paths.begin(), found_paths.end());

  for (const auto &path : found_paths) {
    std::cout << path << std::endl;
  }

  return 0;
}
