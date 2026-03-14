#include <dirent.h>
#include <format>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  const std::span<char *> args(argv, static_cast<std::size_t>(argc));
  if (argc < 3) {
    std::cerr << std::format(
        "Usage: {} <refinement-level> <output-directory>\n", args[0]);
    return 1;
  }
  std::size_t refinementLevel{};
  std::string outputDirectory{};
  try {
    refinementLevel = std::stoull(args[1]);
    if (refinementLevel < 5) {
      std::cerr << std::format(
          "Invalid refinement value \"{}\", should be at least 5\n",
          refinementLevel);
      return 1;
    }
    outputDirectory = args[2];

    DIR *dir = opendir(outputDirectory.c_str());
    if (dir != nullptr) {
      /* Directory exists. */
      closedir(dir);
    } else {
      std::cerr << std::format(
          "Invalid output directory \"{}\", please provide a valid directory\n",
          outputDirectory);
      return 1;
    }
  } catch (const std::exception &) {
    std::cerr << "Error: Invalid command line arguments\n";
    std::cerr << std::format(
        "Usage: {} <refinement-level> <output-directory>\n", args[0]);
    return 1;
  }
  return 0;
}