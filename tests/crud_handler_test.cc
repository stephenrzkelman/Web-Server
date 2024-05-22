#include "filesystem/filesystem_interface.h"
#include "handlers/crud_handler.h"
#include "gtest/gtest.h"
#include <memory>

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

class CrudHandlerTest : public testing::Test {
protected:
  void SetUp() override {}
  http_request parseRequest(boost::asio::mutable_buffer request) {
    boost::system::error_code error;
    boost::beast::http::request_parser<boost::beast::http::string_body> parser;
    parser.put(request, error);
    if (!error && parser.is_done()) {
      return parser.get();
    } else {
      return http_request();
    }
  }
};

// Try getting existent and non-existent files
TEST_F(CrudHandlerTest, GetFiles) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();

  const std::string body = "Hello, world!";
  filesystem->write("/mnt/crud/Shoes/1", body);

  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}},
                      std::move(filesystem));

  http_request ok_request;
  ok_request.method(boost::beast::http::verb::get);
  ok_request.target("/api/Shoes/1");
  http_response response = handler.handle_request(ok_request);
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "text/plain");
  EXPECT_EQ(response.body(), body);

  http_request bad_request;
  bad_request.method(boost::beast::http::verb::get);
  bad_request.target("/api/Shoes/2");
  response = handler.handle_request(bad_request);
  EXPECT_EQ(response.result(),
            boost::beast::http::status::internal_server_error);
  EXPECT_EQ(response.body(), "");
}

// List the files in a directory that exists and doesn't exist
TEST_F(CrudHandlerTest, GetDirectory) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();

  filesystem->write("/mnt/crud/Shoes/2", "");
  filesystem->write("/mnt/crud/Shoes/1", "");
  filesystem->write("/mnt/crud/Shoes/3", "");
  filesystem->write("/mnt/crud/Shoes/foo/bar/baz",
                    "nested files should be ignored");

  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}},
                      std::move(filesystem));

  http_request ok_request;
  ok_request.method(boost::beast::http::verb::get);
  ok_request.target("/api/Shoes/");
  http_response response = handler.handle_request(ok_request);
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "text/plain");
  EXPECT_EQ(response.body(), "{\n"
                             "    \"files\": [\n"
                             "        \"1\",\n"
                             "        \"2\",\n"
                             "        \"3\"\n"
                             "    ]\n"
                             "}\n");

  http_request bad_request;
  bad_request.method(boost::beast::http::verb::get);
  bad_request.target("/api/Gloves/");
  response = handler.handle_request(bad_request);
  EXPECT_EQ(response.result(),
            boost::beast::http::status::internal_server_error);
  EXPECT_EQ(response.body(), "");
}

// Generate some IDs, then read them
TEST_F(CrudHandlerTest, PostGet) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();

  filesystem->write("/mnt/crud/Shoes/foo/bar/baz",
                    "nested files should be ignored");
  filesystem->write("/mnt/crud/Shoes/qux",
                    "non-numeric files should get ignored");

  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}},
                      std::move(filesystem));

  const std::string body1 = "This is the first write!";
  http_request ok_request1;
  ok_request1.method(boost::beast::http::verb::post);
  ok_request1.target("/api/Shoes/");
  ok_request1.body() = body1;

  http_response response1 = handler.handle_request(ok_request1);
  EXPECT_EQ(response1.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response1.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response1.body(), "{\n"
                              "    \"id\": \"1\"\n"
                              "}\n");

  const std::string body2 = "Another write";
  http_request ok_request2;
  ok_request2.method(boost::beast::http::verb::post);
  ok_request2.target("/api/Shoes/");
  ok_request2.body() = body2;

  http_response response2 = handler.handle_request(ok_request2);
  EXPECT_EQ(response2.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response2.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response2.body(), "{\n"
                              "    \"id\": \"2\"\n"
                              "}\n");

  http_request ok_request3;
  ok_request3.method(boost::beast::http::verb::get);
  ok_request3.target("/api/Shoes/1");

  http_response response3 = handler.handle_request(ok_request3);
  EXPECT_EQ(response3.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response3.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response3.body(), body1);

  http_request ok_request4;
  ok_request4.method(boost::beast::http::verb::get);
  ok_request4.target("/api/Shoes/2");

  http_response response4 = handler.handle_request(ok_request4);
  EXPECT_EQ(response4.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response4.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response4.body(), body2);

  // Can't POST to a file
  http_request bad_request;
  bad_request.method(boost::beast::http::verb::post);
  bad_request.target("/api/Shoes/3");
  http_response bad_response = handler.handle_request(bad_request);
  EXPECT_EQ(bad_response.result(), boost::beast::http::status::bad_request);
  EXPECT_EQ(bad_response.body(), "");
}

// Try deleting files that are there and aren't there
TEST_F(CrudHandlerTest, DeleteFile) {
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();

  const std::string body = "File to be deleted";
  const std::string file_path = "/mnt/crud/Shoes/1";

  // 1: Write the file to the fake filesystem
  filesystem->write(file_path, body);

  // 2: Create the CrudHandler and make a DELETE request
  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}}, std::move(filesystem));

  http_request delete_request;
  delete_request.method(boost::beast::http::verb::delete_);
  delete_request.target("/api/Shoes/1");
  http_response response = handler.handle_request(delete_request);

  // 3: Verify that the file is deleted successfully
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.body(), "");

  // 4: Confirm that the file no longer exists in the filesystem
  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/api/Shoes/1");
  http_response get_response = handler.handle_request(get_request);
  EXPECT_EQ(get_response.result(), boost::beast::http::status::internal_server_error);

  // Delete something that isn't there
  http_request bad_del_request;
  bad_del_request.method(boost::beast::http::verb::delete_);
  bad_del_request.target("/api/Shoes/1");
  http_response bad_del_response = handler.handle_request(bad_del_request);

  EXPECT_EQ(bad_del_response.result(), boost::beast::http::status::bad_request);
  EXPECT_EQ(bad_del_response.body(), "");
}

// attempt to delete directory (that's not allowed)
TEST_F(CrudHandlerTest, DeleteDirectory) {
  std::unique_ptr<FileSystemInterface> filesystem = std::make_unique<FakeFileSystem>();

  const std::string body = "File to be deleted";
  const std::string file_path = "/mnt/crud/Shoes/1";

  // 1: Write the file to the fake filesystem
  filesystem->write(file_path, body);

  // 2: Create the CrudHandler and make a DELETE request
  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}}, std::move(filesystem));

  http_request delete_request;
  delete_request.method(boost::beast::http::verb::delete_);
  delete_request.target("/api/Shoes/");
  http_response response = handler.handle_request(delete_request);

  EXPECT_EQ(response.result(), boost::beast::http::status::bad_request);
  EXPECT_EQ(response.body(), "");
}

TEST_F(CrudHandlerTest, PutSuccess) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/crud/Shoes/2", "");
  filesystem->write("/mnt/crud/Shoes/1", "");
  filesystem->write("/mnt/crud/Shoes/3", "");

  // define handler
    CrudHandler handler("/api", {{"data_path", "/mnt/crud"}},
                        std::move(filesystem));
  // bodies
  const std::string body = "ID 1 NEW";

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::put);
  ok_request.target("/api/Shoes/1");
  ok_request.body() = body;
  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "text/plain");

  // check conctents
  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/api/Shoes/1");
  http_response response_get = handler.handle_request(get_request);
  EXPECT_EQ(response_get.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response_get.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response_get.body(), body);
}

TEST_F(CrudHandlerTest, PutFailureDirectory) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/crud/Shoes/2", "");
  filesystem->write("/mnt/crud/Shoes/1", "");
  filesystem->write("/mnt/crud/Shoes/3", "");

  // define handler
  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}},
                      std::move(filesystem));

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::put);
  ok_request.target("/api/Shoes");
  ok_request.body() = "ID 1 NEW";
  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::bad_request);
}

TEST_F(CrudHandlerTest, PutSuccessNoFile) {
  std::unique_ptr<FileSystemInterface> filesystem =
      std::make_unique<FakeFileSystem>();
  // define some dummy vals
  filesystem->write("/mnt/crud/Shoes/2", "");
  filesystem->write("/mnt/crud/Shoes/1", "");
  filesystem->write("/mnt/crud/Shoes/3", "");

  // define handler
  CrudHandler handler("/api", {{"data_path", "/mnt/crud"}},
                      std::move(filesystem));

  // bodies
  const std::string body = "ID 4 NEW";

  // make request
  http_request ok_request;
  ok_request.method(boost::beast::http::verb::put);
  ok_request.target("/api/Shoes/4");
  ok_request.body() = body;
  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "text/plain");

  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/api/Shoes/4");
  http_response response_get = handler.handle_request(get_request);
  EXPECT_EQ(response_get.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response_get.at(boost::beast::http::field::content_type),
            "text/plain");
  EXPECT_EQ(response_get.body(), body);

}
