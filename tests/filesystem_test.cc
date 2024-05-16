#include "filesystem/filesystem_interface.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <optional>
#include <vector>

class FakeFileSystem : public FileSystemInterface {
public:
  FakeFileSystem() = default;
  bool exists(const std::filesystem::path &path) const override {
    return filesystem_.contains(path);
  }
  std::optional<std::vector<std::filesystem::path>>
  list(const std::filesystem::path &directory) const override {
    // Return std::nullopt for files
    if (directory.has_filename()) {
      return std::nullopt;
    }
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
    return files;
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

private:
  std::unordered_map<std::filesystem::path, std::string> filesystem_;
};

class FileSystemTest : public testing::Test {
protected:
  void SetUp() override {}
};

// Reading a non-existent file
TEST_F(FileSystemTest, ReadNonexistentFile) {
  FakeFileSystem filesystem;
  EXPECT_EQ(filesystem.read("foo/bar/baz"), std::nullopt);
}

// Reading a directory
TEST_F(FileSystemTest, ReadDirectory) {
  FakeFileSystem filesystem;
  EXPECT_EQ(filesystem.read("foo/bar/"), std::nullopt);
}

// Removing non-existent file
TEST_F(FileSystemTest, RemoveNonexistentFile) {
  FakeFileSystem filesystem;
  EXPECT_FALSE(filesystem.remove("foo/bar/baz"));
}

// Writing, reading, then removing the same file
TEST_F(FileSystemTest, BasicReadWriteRemove) {
  FakeFileSystem filesystem;
  const std::filesystem::path path = "foo/bar/baz";
  const std::string data = "truly one of the sentences of all time";
  EXPECT_TRUE(filesystem.write(path, data));
  EXPECT_EQ(filesystem.read(path), data);
  EXPECT_TRUE(filesystem.remove(path));
  EXPECT_EQ(filesystem.read(path), std::nullopt);
}

// Writing to a directory
TEST_F(FileSystemTest, WriteDirectory) {
  FakeFileSystem filesystem;
  EXPECT_FALSE(filesystem.write("foo/bar/", ""));
}

// Listing the files in a directory (nested files get skipped!)
TEST_F(FileSystemTest, ListDirectory) {
  FakeFileSystem filesystem;
  EXPECT_TRUE(
      filesystem.write("foo/bar/baz", "this is nested and should get skipped"));
  EXPECT_TRUE(filesystem.write("foo/qux", "just a file"));
  EXPECT_TRUE(filesystem.write("foo/baz", "another file"));
  EXPECT_NE(filesystem.list("foo/"), std::nullopt);
  EXPECT_EQ(filesystem.list("foo/").value().size(), 2);
}
