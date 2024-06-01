#include "filesystem/fake_filesystem.h"
#include "gtest/gtest.h"
#include <memory>


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
