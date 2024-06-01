#include "filesystem/filesystem_interface.h"
#include <boost/filesystem.hpp>
#include <memory>
#include <filesystem>

class FakeFileSystem : public FileSystemInterface {
public:
  FakeFileSystem() = default;
  bool exists(const std::filesystem::path &path) const override {
    return filesystem_.contains(path);
  }
  std::optional<std::vector<std::filesystem::path>>
  list(const std::filesystem::path &directory) const override {
    std::vector<std::filesystem::path> files;
    for (const auto &[path_, _] : filesystem_) {
      // Add the path to our list of files if the directory is a prefix for it
      const std::string dir = directory.string();
      const std::string path = path_.string();
      if (path.starts_with(dir)) {
        // Ignore nested files
        const std::string remainder = path.substr(dir.size());
        if (remainder.find('/') == std::string::npos) {
          files.push_back(path_);
        }
      }
    }
    // Guarantee ordering for unit testing
    std::sort(files.begin(), files.end());
    return files;
  }

  FILE_TYPE fileType(std::string file_name) const override {
    std::string file_extension =
    boost::filesystem::path(file_name).extension().string();
    if (FILE_TYPE_MAP.find(file_extension) == FILE_TYPE_MAP.end()) {
      return NO_MATCHING_TYPE;
    } else {
      return FILE_TYPE_MAP.find(file_extension)->second;
    }
  }

  std::optional<std::string>
  read(const std::filesystem::path &filename) const override {
    // If the given path is a directory, then return nullopt
    if (!filename.has_filename()) {
      return std::nullopt;
    }
    if (!exists(filename)) {
      return std::nullopt;
    }
    return filesystem_.at(filename);
  }
  bool write(const std::filesystem::path &filename,
             const std::string &data) override {
    // If the given path is a directory, then return false
    if (!filename.has_filename()) {
      return false;
    }

    filesystem_[filename] = data;
    return true;
  }
  bool remove(const std::filesystem::path &filename) override {
    if (!exists(filename)) {
      return false;
    }
    filesystem_.erase(filename);
    return true;
  }
  bool is_directory(const std::filesystem::path &path) const override {
    for (const auto &[p, _] : filesystem_) {
      if (p.string().starts_with(path.string()) &&
          p.string() != path.string()) {
        return true;
      }
    }
    return false;
  }
  bool create_directories(const std::filesystem::path &path) override {
    return true;
  }

private:
  std::unordered_map<std::filesystem::path, std::string> filesystem_;
};
