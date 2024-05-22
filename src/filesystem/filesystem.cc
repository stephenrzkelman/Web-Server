#include "filesystem/filesystem.h"
#include <boost/log/trivial.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

bool FileSystem::exists(const fs::path &path) const {
  std::error_code ec;
  bool path_exists = fs::exists(path, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(debug)
        << "failed to check existence of " << path << " because " << ec;
    return false;
  }
  return path_exists;
}

std::optional<std::vector<fs::path>>
FileSystem::list(const fs::path &directory) const {
  if (!exists(directory)) {
    BOOST_LOG_TRIVIAL(debug) << directory << " doesn't exist";
    return std::nullopt;
  }
  if (!is_directory(directory)) {
    BOOST_LOG_TRIVIAL(debug) << directory << " is not a directory";
    return std::nullopt;
  }

  std::vector<fs::path> files;
  for (const auto &dir_entry : fs::directory_iterator{directory}) {
    if (is_regular_file(dir_entry)) {
      files.push_back(dir_entry);
    }
  }
  return files;
};

std::optional<std::string> FileSystem::read(const fs::path &filename) const {
  if (!exists(filename)) {
    BOOST_LOG_TRIVIAL(debug) << filename << " doesn't exist";
    return std::nullopt;
  }
  if (!is_regular_file(filename)) {
    BOOST_LOG_TRIVIAL(debug) << filename << " is not a regular file";
    return std::nullopt;
  }

  std::ifstream ifs(filename);
  std::string data(std::istreambuf_iterator<char>{ifs}, {});
  return data;
}

bool FileSystem::write(const fs::path &filename, const std::string &data) {
  if (!filename.has_filename()) {
    BOOST_LOG_TRIVIAL(debug) << filename << " doesn't refer to a file";
    return false;
  }

  // Creates any parent directories for the file if they don't already exist
  std::error_code ec;
  fs::create_directories(filename.parent_path(), ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(debug)
        << "failed to create parent directories: " << filename.parent_path()
        << " because " << ec;
    return false;
  }

  std::ofstream ofs(filename);
  ofs << data;
  ofs.close();
  return true;
}

bool FileSystem::remove(const fs::path &filename) {
  if (!is_regular_file(filename)) {
    BOOST_LOG_TRIVIAL(debug) << "couldn't delete " << filename
                             << " because it is not a regular file";
    return false;
  }

  std::error_code ec;
  bool removed = fs::remove(filename, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(debug) << "couldn't delete regular file " << filename;
    return false;
  }
  return removed;
}

bool FileSystem::is_regular_file(const fs::path &path) const {
  std::error_code ec;
  bool is_regular_file = fs::is_regular_file(path, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(debug)
        << "failed to check if " << path << " is a regular file because " << ec;
    return false;
  }
  return is_regular_file;
}

bool FileSystem::is_directory(const fs::path &path) const {
  std::error_code ec;
  bool is_directory = fs::is_directory(path, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(debug)
        << "failed to check if " << path << " is a directory because " << ec;
    return false;
  }
  return is_directory;
}

bool FileSystem::create_directories(const fs::path &path) {
  std::error_code ec;
  bool created = fs::create_directories(path, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(debug)
        << "failed to create directories at " << path << " because " << ec;
    return false;
  }
  return true;
}
