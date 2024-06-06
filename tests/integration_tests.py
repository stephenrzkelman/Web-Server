import subprocess
from typing import List
import argparse
import sys
import signal
import time
import shutil

class IntegrationTest(): 
    def __init__(self, config, server) -> None:
        self.start_server(config, server)
        self.is_server_on = True # assumes that start_server always successfully starts the server, should add error handling
        self.num_tests = 0
        self.num_tests_pass = 0

    def start_server(self, config: str, server: str) -> subprocess.Popen: 
        '''
        start the web server using params defined in config
        
        config (str): file name for nginx config file 
        return: Popen object
        '''

        # TODO: create some way to generate a config file automatically instead of statically encoding one? potentailly create an overloaded start_server() function
        # TODO: error handle server start. if the port is already in use, try running on a different port or kill? can also terminate script early
        self.server = subprocess.Popen([server, config])
        time.sleep(0.1)
    
    def end_server(self) -> None: 
        '''
        kills the web server created by start_server

        return: None
        '''

        self.server.kill()
        print("Server has been killed")

    def print_results(self) -> None: 
        '''
        prints overall results of tests
        '''

        print(f"Overall Test Results: {self.num_tests_pass} / {self.num_tests} passed")

    def curl_server(self, args: List[str]) -> str: 
        '''
        run curl with args and return the response 

        args (List[str]): the arguments that follow curl. each token is its own entry in the list

        return (str): response from curl
        '''
        
        curl_process = subprocess.run(['curl'] + args, capture_output=True)
        return curl_process.stdout.decode('utf-8')
    
    def netcat_server(self, args: List[str], content_to_send: str, timeout: int = 1) -> str:
        '''
        run netcat with args and return the response 

        args (List[str]): the arguments that follow netcat. each token is its own entry in the list
        timeout (int): used for nc -w. If a connection and stdin are idle for more than timeout seconds, then the connection is silently closed.
        content_to_send (str): what content is sent alongside a network request. in netcat, this content is written to stdin which triggers an event in netcat

        return (str): response from netcat
        '''

        command = ['nc', '-w', str(timeout)] + args
        nc_process = subprocess.run(command, capture_output=True, input=content_to_send.encode('utf-8'))
        return nc_process.stdout.decode('utf-8')
    
    def __test_case_helper(self, name: str, expected: str, given: str) -> bool: 
        '''
        defines and runs a test case, updates count of results

        name (str): the name of the test case. used in print output
        expected (str): what the expected response should be
        given (str): the given response to validate

        return (bool): indicates whether test case succeeded or not
        '''

        # TODO, nice2have: consider option to write output to a results file instead of printing all of the results

        # validate the response 
        self.num_tests += 1
        try:
            assert(given == expected)
        except AssertionError: 
            print(f"{name}: FAIL")
            print(f"Expected:\n{expected}")
            print(f"Given:\n{given}")
            return False

        print(f"{name}: PASS")
        self.num_tests_pass += 1
        return True

    def test_case_curl(self, name: str, expected: str, args: List[str]) -> bool:
        '''
        generates response using curl and runs test case

        name (str): the name of the test case. used in print output
        expected(str): what the expected response should be
        args (List[str]): the arguments that follow 'command'. each token is its own entry in the list

        return (bool): indicates whether test case succeeded or not
        '''

        print(f"\nStarting curl test case: {name}")
        response = self.curl_server(args)
        return self.__test_case_helper(name, expected, response)


    def test_case_nc(self, name: str, expected: str, args: List[str], content_to_send: str = '') -> bool:
        '''
        defines and runs a test case

        name (str): the name of the test case. used in print output
        expected(str): what the expected response should be
        args (List[str]): the arguments that follow 'command'. each token is its own entry in the list
        content_to_send (str): what content is sent alongside a network request

        return (bool): indicates whether test case succeeded or not
        '''
        
        print(f"\nStarting nc test case: {name}")
        response = self.netcat_server(args, content_to_send)
        return self.__test_case_helper(name, expected, response)
    
    def test_case_sleep_handler(self) -> bool:
        print(f"\nStarting sleep handler test case")

        # Call sleep handler then immediately call an echo request in another process
        sleep_process = subprocess.Popen(['curl', 'localhost:80/sleep'], stdout=subprocess.PIPE)
        
        # Echo request should immediately return the response
        echo_process = subprocess.Popen(["curl", "localhost:80/echo"], stdout=subprocess.PIPE)

        echo_out = ""
        try:
            # wait for echo process to complete then collect output
            # given timeout of 2 sec to test if echo process returns response without waiting for sleep
            out, _ = echo_process.communicate(timeout=2)
            echo_out = out.decode('utf-8')
            # ensure sleep process is still running (echo is handled before sleep)
            assert(sleep_process.poll() is None)
        except AssertionError: 
            print(f"sleep handler test case: FAIL")
            print(f"Expected echo process to finish before sleep process finished")
            return False
        except Exception as e:
            echo_out = e
            echo_process.kill()

        sleep_out = ""
        try:
            # wait for sleep process to complete then collect output
            out, _ = sleep_process.communicate(timeout=4)
            sleep_out = out.decode('utf-8')
        except Exception as e:
            sleep_out = e
            sleep_process.kill()

        sleep_expected = "Sleep handler test"
        sleep_pass = self.__test_case_helper(name="test_case_sleep_handler:sleep", expected= sleep_expected, given=sleep_out) 
        echo_expected = "GET /echo HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n"
        echo_pass = self.__test_case_helper(name="test_case_sleep_handler:echo", expected= echo_expected, given=echo_out)

        return echo_pass and sleep_pass

    def signal_handler(self, sig, frame):
        print('Integration tests interrupted, received SIGINT (ctrl+c)')
        self.end_server()
        sys.exit(0)

def main(config = str, server = str):  
    # Init Integration Test and Signal Handler for ctrl+c
    tester = IntegrationTest(config, server)
    signal.signal(signal.SIGINT, tester.signal_handler)
    shutil.rmtree("./crud_files", ignore_errors=True)

    # A GET request sent through curl should receive a response
    tester.test_case_curl(name = "test_curl_basic",
                          expected = "GET /echo HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n", 
                          args = ['localhost:80/echo'])
    
    # curl localhost:80/ should have a keep-alive parameter
    tester.test_case_curl(name = "test_curl_localhost:80",
                          expected = "GET /echo HTTP/1.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n",
                          args = ["-H", "Host:", "-H", "Connection: keep-alive", "localhost:80/echo"])
    
    # a simple message sent through netcat should send an error
    not_found_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n"
    tester.test_case_nc(name = "test_nc_not_1.1", 
                        expected =not_found_header, 
                        args = ['localhost', '80'],
                        content_to_send='GET /sodhfoisdfjhosdifj HTTP/1.1\r\n\r\n')
    
    # sending a valid HTTP request through netcat should receive an echo'd response appended to the response header
    valid_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 77\r\n\r\n"
    valid_request = "GET /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n"
    tester.test_case_nc(name = "test_nc_basic",
                        expected= valid_header + valid_request,
                        args = ["localhost", "80"],
                        content_to_send=valid_request)
    
    # multithread test
    tester.test_case_sleep_handler()

    # even a header with a long amount of content should return a proper response
    long_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 2777\r\n\r\n"
    long_request = '''GET /echo HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\nLorem-ipsum: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod \
tempor incididunt ut labore et dolore magna aliqua. Sagittis aliquam malesuada bibendum arcu. Pharetra diam sit amet nisl suscipit. Sollicitudin aliquam ultrices sagittis orci a \
scelerisque purus. Turpis in eu mi bibendum neque egestas congue quisque egestas. Dignissim convallis aenean et tortor at. Velit aliquet sagittis id consectetur purus ut faucibus \
pulvinar elementum. Tristique nulla aliquet enim tortor at auctor urna nunc id. Ornare aenean euismod elementum nisi quis eleifend quam adipiscing vitae. Egestas erat imperdiet sed \
euismod. Vitae sapien pellentesque habitant morbi tristique senectus et netus et. Egestas pretium aenean pharetra magna ac placerat. Quis ipsum suspendisse ultrices gravida dictum. Est \
sit amet facilisis magna etiam. Leo urna molestie at elementum. Arcu non odio euismod lacinia at quis. Amet volutpat consequat mauris nunc congue nisi. Diam maecenas ultricies mi eget \
mauris pharetra et ultrices neque. Pharetra magna ac placerat vestibulum lectus mauris ultrices. Facilisis sed odio morbi quis commodo odio aenean sed adipiscing. Vulputate eu \
scelerisque felis imperdiet proin fermentum leo vel orci. Vitae ultricies leo integer malesuada nunc vel risus commodo. Id nibh tortor id aliquet lectus. Sit amet venenatis urna cursus \
eget nunc scelerisque viverra mauris. Praesent semper feugiat nibh sed pulvinar proin gravida hendrerit lectus. Congue mauris rhoncus aenean vel elit scelerisque mauris pellentesque \
pulvinar. Lectus arcu bibendum at varius vel pharetra vel turpis. Ut tellus elementum sagittis vitae. Urna neque viverra justo nec. Duis convallis convallis tellus id interdum velit \
laoreet. Neque sodales ut etiam sit amet nisl. Interdum consectetur libero id faucibus nisl tincidunt eget. Aliquam faucibus purus in massa tempor nec. Facilisi nullam vehicula ipsum a \
arcu cursus vitae congue mauris. Nisi est sit amet facilisis magna etiam tempor orci. Diam maecenas sed enim ut sem. Mauris pellentesque pulvinar pellentesque habitant morbi tristique \
senectus et netus. Lectus arcu bibendum at varius. Eu nisl nunc mi ipsum. Ridiculus mus mauris vitae ultricies leo. Interdum consectetur libero id faucibus nisl tincidunt. Auctor augue \
mauris augue neque gravida. Facilisi etiam dignissim diam quis enim lobortis scelerisque. Venenatis lectus magna fringilla urna. Tellus integer feugiat scelerisque varius morbi enim. \
Diam quam nulla porttitor massa id. Diam maecenas ultricies mi eget mauris pharetra et ultrices neque. Ipsum suspendisse ultrices gravida dictum fusce ut. Lobortis mattis aliquam \
faucibus purus in massa. Ultricies integer quis auctor elit sed.\"\r\n\r\n'''

    tester.test_case_nc(name = "test_nc_long_content",
                        expected=long_header + long_request,
                        args = ["localhost", "80"],
                        content_to_send=long_request)

    ## CRUD test
    # POST request to create a resource
    post_data = '{"name": "test", "value": "123"}'
    tester.test_case_curl(
        name="test_curl_CRUD_POST",
        expected='{\n    "id": "1"\n}\n',
        args=['-X', 'POST', '-H', 'Content-Type: application/json', '-d', post_data, 'localhost:80/api/test'],
    )
    
    # GET request to retrieve the created resource
    tester.test_case_curl(
        name="test_curl_CRUD_GET_retrieve_POST",
        expected='{"name": "test", "value": "123"}',
        args=['-X', 'GET', 'localhost:80/api/test/1']
    )
    
    # PUT request to update the resource
    put_data = '{"name": "test", "value": "456"}'
    tester.test_case_curl(
        name="test_curl_CRUD_PUT_update",
        expected= "",
        args=['-X', 'PUT', '-H', 'Content-Type: application/json', '-d', put_data, 'localhost:80/api/test/1'],
    )
    
    # GET request to confirm the update
    tester.test_case_curl(
        name="test_curl_CRUD_retrieve_PUT",
        expected='{"name": "test", "value": "456"}',
        args=['-X', 'GET', 'localhost:80/api/test/1']
    )

    # DELETE request to remove the resource
    tester.test_case_curl(
        name="test_curl_CRUD_delete",
        expected= "",
        args=['-X', 'DELETE', 'localhost:80/api/test/1']
    )
    
    # GET request to confirm the deletion
    tester.test_case_curl(
        name="test_curl_GET_confirm_delete",
        expected= "",
        args=['-X', 'GET', 'localhost:80/api/test/1']
    )

    # PUT request to make something at a specific ID
    put_data = '{"name": "test", "value": "789"}'
    tester.test_case_curl(
        name="test_curl_CRUD_PUT_creation",
        expected= "",
        args=['-X', 'PUT', '-H', 'Content-Type: application/json', '-d', put_data, 'localhost:80/api/test/1'],
    )

    # GET request to confirm the creation
    tester.test_case_curl(
        name="test_curl_CRUD_GET_confirm_PUT_creation",
        expected='{"name": "test", "value": "789"}',
        args=['-X', 'GET', 'localhost:80/api/test/1']
    )

    # GET for LIST functionality
    tester.test_case_curl(
        name="test_curl_CRUD_GET_LIST",
        expected='{\n    "files": [\n        "1"\n    ]\n}\n',
        args=['-X', 'GET', 'localhost:80/api/test']
    )

    ## Markdown Handler Tests
    # GET request for a nonexistant file 
    tester.test_case_curl( 
        name="test_curl_MARKDOWN_GET_nonexistant",
        expected="",
        args=['-X', 'GET', 'localhost:80/markdown/dne.md']

    )

    # POST request to create a file 
    post_data = "markdown content"
    tester.test_case_curl(
        name="test_curl_MARKDOWN_POST_creation",
        expected="",
        args=['-X', 'POST', '-d', post_data, '-H', 'Content-Type: text/markdown', 'localhost:80/markdown/test.md']
    ) 

    # PUT request for a non markdown file should fail
    tester.test_case_curl(
        name="test_curl_MARKDOWN_PUT_not_markdown",
        expected="",
        args=['-X', 'PUT', '-d', put_data, '-H', 'Content-Type: text/markdown', 'localhost:80/markdown/not_md_file']
    ) 

    # GET request to check that test.md was created with proper content 
    tester.test_case_curl(
        name="test_curl_MARKDOWN_GET",
        expected="<p>markdown content</p>\n\n",
        args=['-X', 'GET', 'localhost:80/markdown/test.md']
    )

    # DELETE request to cleanup 
    tester.test_case_curl(
        name="test_curl_MARKDOWN_DELETE",
        expected="",
        args=['-X', 'DELETE', 'localhost:80/markdown/test.md']
    )

    # GET request to find test.md should no longer return content since it was deleted
    tester.test_case_curl(
        name="test_curl_MARKDOWN_GET_after_delete",
        expected="",
        args=['-X', 'GET', 'localhost:80/markdown/test.md']
    )

    # Finished test cases, close server
    tester.end_server()
    tester.print_results()

    if tester.num_tests != tester.num_tests_pass:
        sys.exit(1)

if __name__ == '__main__':  
    parser = argparse.ArgumentParser(prog = 'Integration Test')
    parser.add_argument('config')
    parser.add_argument('server')
    args = parser.parse_args()
    main(args.config, args.server)
