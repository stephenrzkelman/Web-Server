#include "filesystem/fake_filesystem.h"
#include "handlers/crud_handler.h"
#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include <memory>

class CrudHandlerTest : public testing::Test {
protected:
  void SetUp() override {}
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
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "application/json");
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
  ok_request.set(boost::beast::http::field::content_type, "application/json");
  http_response response = handler.handle_request(ok_request);

  EXPECT_EQ(response.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response.at(boost::beast::http::field::content_type), "application/json");
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
  ok_request1.set(boost::beast::http::field::content_type, "application/json");
  ok_request1.target("/api/Shoes/");
  ok_request1.body() = body1;

  http_response response1 = handler.handle_request(ok_request1);
  EXPECT_EQ(response1.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response1.at(boost::beast::http::field::content_type),
            "application/json");
  EXPECT_EQ(response1.body(), "{\n"
                              "    \"id\": \"1\"\n"
                              "}\n");

  const std::string body2 = "Another write";
  http_request ok_request2;
  ok_request2.method(boost::beast::http::verb::post);
  ok_request2.set(boost::beast::http::field::content_type, "application/json");
  ok_request2.target("/api/Shoes/");
  ok_request2.body() = body2;

  http_response response2 = handler.handle_request(ok_request2);
  EXPECT_EQ(response2.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response2.at(boost::beast::http::field::content_type),
            "application/json");
  EXPECT_EQ(response2.body(), "{\n"
                              "    \"id\": \"2\"\n"
                              "}\n");

  http_request ok_request3;
  ok_request3.method(boost::beast::http::verb::get);
  ok_request3.target("/api/Shoes/1");

  http_response response3 = handler.handle_request(ok_request3);
  EXPECT_EQ(response3.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response3.at(boost::beast::http::field::content_type),
            "application/json");
  EXPECT_EQ(response3.body(), body1);

  http_request ok_request4;
  ok_request4.method(boost::beast::http::verb::get);
  ok_request4.target("/api/Shoes/2");

  http_response response4 = handler.handle_request(ok_request4);
  EXPECT_EQ(response4.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response4.at(boost::beast::http::field::content_type),
            "application/json");
  EXPECT_EQ(response4.body(), body2);

  // Can't POST to a file
  http_request bad_request;
  bad_request.method(boost::beast::http::verb::post);
  bad_request.target("/api/Shoes/3");
  bad_request.set(boost::beast::http::field::content_type, "application/json");
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
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);
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
  ok_request.set(boost::beast::http::field::content_type, "application/json");
  ok_request.target("/api/Shoes/1");
  ok_request.body() = body;
  ok_request.prepare_payload();

  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);

  // check conctents
  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/api/Shoes/1");
  http_response response_get = handler.handle_request(get_request);
  EXPECT_EQ(response_get.result(), boost::beast::http::status::ok);
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
  ok_request.set(boost::beast::http::field::content_type, "application/json");
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
  ok_request.set(boost::beast::http::field::content_type, "application/json");
  ok_request.target("/api/Shoes/4");
  ok_request.body() = body;
  http_response response = handler.handle_request(ok_request);

  // read request
  EXPECT_EQ(response.result(), boost::beast::http::status::no_content);

  http_request get_request;
  get_request.method(boost::beast::http::verb::get);
  get_request.target("/api/Shoes/4");
  http_response response_get = handler.handle_request(get_request);
  EXPECT_EQ(response_get.result(), boost::beast::http::status::ok);
  EXPECT_EQ(response_get.at(boost::beast::http::field::content_type),
            "application/json");
  EXPECT_EQ(response_get.body(), body);

}
