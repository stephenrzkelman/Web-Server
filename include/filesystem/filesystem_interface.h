#ifndef FILESYSTEM_INTERFACE_H
#define FILESYSTEM_INTERFACE_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

class FileSystemInterface {
public:
  // Pure virtual function to check if a file or directory exists
  virtual bool exists(const std::filesystem::path &path) const = 0;
  // Pure virtual function to list all the files in a directory
  // Returns std::nullopt if the input path doesn't exist or isn't a directory
  virtual std::optional<std::vector<std::filesystem::path>>
  list(const std::filesystem::path &directory) const = 0;
  // Pure virtual function to read the file at a given path
  // Returns std::nullopt if the file doesn't exist, otherwise the file contents
  virtual std::optional<std::string>
  read(const std::filesystem::path &filename) const = 0;
  // Pure virtual function to write a string payload to a given path
  // Returns whether or not the write happened successfully
  virtual bool write(const std::filesystem::path &filename,
                     const std::string &data) = 0;
  // Pure virtual function to remove a file
  // Returns whether or not the removal happened successfully
  virtual bool remove(const std::filesystem::path &filename) = 0;
  // Pure virtual function to check if a path is a directory
  virtual bool is_directory(const std::filesystem::path &path) const = 0;
  // Pure virtual function to create directories
  virtual bool create_directories(const std::filesystem::path &path) = 0;
};

#endif // FILESYSTEM_INTERFACE_H
