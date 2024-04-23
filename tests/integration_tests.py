import subprocess
from typing import List

DOCKER_CONFIG = 'docker_config'

class IntegrationTest(): 
    def __init__(self, config = 'docker_config') -> None:
        self.start_server(config)
        self.is_server_on = True # assumes that start_server always successfully starts the server, shoudl add error handling
        self.num_tests = 0
        self.num_tests_pass = 0

        # TODO, nice2have: add variables to track a count of test cases run, number succeeded, etc.

    def start_server(self, config: str) -> subprocess.Popen: 
        '''
        start the web server using params defined in config
        
        config (str): file name for nginx config file 
        return: Popen object
        '''

        # TODO: create some way to generate a config file automatically instead of statically encoding one? potentailly create an overloaded start_server() function
        # TODO: error handle server start. if the port is already in use, try running on a different port or kill? can also terminate script early
        self.server = subprocess.Popen(['../build/bin/server', config])
    
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
        nc_process = subprocess.Popen(command, 
                                      stdin=subprocess.PIPE, 
                                      stdout=subprocess.PIPE, 
                                      text=True)
        response = ""

        # send netcat request
        nc_process.stdin.write(content_to_send)

        # flush and close stdin to end netcat
        nc_process.stdin.flush()
        nc_process.stdin.close()

        # read stdout 
        while True: 
            line = nc_process.stdout.readline()
            response += line
            if not line: 
                break 
        
        # kill process and return response
        nc_process.kill()
        return response
    
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

def main():
    tester = IntegrationTest(DOCKER_CONFIG)

    # A GET request sent through curl should receive a response
    tester.test_case_curl(name = "test_curl_basic",
                          expected = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n", 
                          args = ['localhost:80'])
    
    # a 400 bad request response should be sent to an invalid request
    tester.test_case_nc(name = "test_nc_not_1.1", 
                        expected = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 0\n\n", 
                        args = ['localhost', '80'],
                        content_to_send='hey there\r\n')
    
    # sending a valid HTTP request through netcat should receive an echo'd response appended to the response header
    tester.test_case_nc(name = "test_nc_basic",
                        expected="HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 73\n\nGET / HTTP/1.1\nHost: 127.0.0.1\nUser-Agent: curl/7.81.0\nAccept: */*\n\n",
                        args = ["localhost", "80"],
                        content_to_send="GET / HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: curl/7.81.0\r\nAccept: */*\r\n\r\n")
    
    # Finished test cases, close server
    tester.end_server()
    tester.print_results()

if __name__ == '__main__': 
    main()

