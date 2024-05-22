#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "filesystem/filesystem_interface.h"
#include <optional>
#include <string>

class FileSystem : public FileSystemInterface {
public:
  FileSystem() = default;
  bool exists(const std::filesystem::path &path) const override;
  std::optional<std::vector<std::filesystem::path>>
  list(const std::filesystem::path &directory) const override;
  std::optional<std::string>
  read(const std::filesystem::path &filename) const override;
  bool write(const std::filesystem::path &filename,
             const std::string &data) override;
  bool remove(const std::filesystem::path &path) override;
  bool is_directory(const std::filesystem::path &path) const override;
  bool create_directories(const std::filesystem::path &path) override;

private:
  // Some helper methods to help in performing error checking, otherwise the
  // server might panic and crash at runtime

  // If the path doesn't exist, these will return false as well
  // We wish to read from regular files because fstream doesn't resolve symlinks
  bool is_regular_file(const std::filesystem::path &path) const;
};

#endif // FILESYSTEM_H
