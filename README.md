# Contributor Documentation<a id="contributor-documentation"></a>

## Source Code Layout<a id="source-code-layout"></a>

\[ Dependency Graph ]

\[ Code Flow ]


## How to Build, Test, and Run the code<a id="how-to-build-test-and-run-the-code"></a>

### Building:<a id="building"></a>

**From the base directory** of the repository, **in the dev environment**, run the following for a clean build:

    rm -rf build
    mkdir build
    cd build
    cmake ..
    make


### Testing<a id="testing"></a>

After building, **from the build directory**, **in the dev env**, run:

    make test

This, together with the build step above, can be run **from the base directory**, **in the dev env**, with:

    ./clean_build.sh


### Coverage<a id="coverage"></a>

To check test coverage, go to the **base directory of the repository** **in the dev env**, and run: 

    ./clean_coverage.sh

The coverage results can be found in “build\_coverage/”, and you can view results in your browser via “build\_coverage/index.html”


### CMake<a id="cmake"></a>

## Adding a Request Handler<a id="adding-a-request-handler"></a>

### For the Config:<a id="for-the-config"></a>

In “`include/constants.h`”:

1. under “// types of request handlers”, add a `const std::string` for your new handler type. The name of this string variable can be whatever you like, but the string itself must be whatever you want to use as the 2nd argument to “`location`” in the config.

2. be sure to also add this string to `VALID_HANDLERS`, in the same file.

If your new handler needs a new keyword/argument (kind of like how StaticHandler needs `root`), then in “`include/constants.h`”, do the following. Otherwise, skip to step 7:

3. under “// directive/context keywords”, add a `const std::string` for your new argument. The name of the string variable can be whatever you like, but the string itself must be whatever keyword will appear in the config file when specifying this argument.

4. Add this string to `EXPECTED_ARG_COUNTS`, along with the number of arguments this directive should have

5. Add the string to `VALID_PARENT_CONTEXTS`, along with the name of the block in which it will appear. If it doesn’t appear in a block, its parent context is `MAIN`.

6. If the new keyword specifies a directive (i.e. `root <filepath>;`), then add the string to `VALID_DIRECTIVES`. Otherwise, it specifies a context (i.e. `location…{}`), and you should add it to `VALID_CONTEXTS`. 

In “`src/config_parser.cc`”:

7. Towards the end of `NginxConfig::findLocations()`, you’ll find\
   `if(handler == STATIC_HANDLER)
   ...
   else if(handler == ECHO_HANDLER)...`

Add another `else if (handler == <YOUR_NEW_HANDLER>)`

8. In the body of this if statement, add handling based on how your new handler interacts with the `root` keyword. For reference, `ECHO_HANDLER` shows what should happen if a handler doesn’t want a root argument, and `STATIC_HANDLER` shows what should happen if a handler wants exactly one root argument.

   1. The `findRoot()` function returns a `std::optional<std::string>`.\
      \- If no `root` directives are found, it returns an empty string\
      \- If exactly one `root` directive is found, it returns the specified root\
      \- If multiple are found, it returns an empty optional

If your new handler needs a new keyword/argument (i.e., if you did steps 3-6), then in “`src/config_parser.cc`”, do the following. If you skipped steps 3-6, you can skip these as well:

9. You’ll want to write a new function similar to `NginxConfig::findRoot()`. 

   1. Notice that `NginxConfig::findDirectives(directiveName)` can be used to find a directive, like with `findDirectives(ROOT)`, or can be used to find context labels, like with `findDirectives(LOCATION)` 

10. Use your function in each of the `handler ==` cases to extract your new argument. This should be similar to how you used `findRoot()` in step 7, but you’ll be using the new function that you just wrote

    1. Be sure to add this for _all_ handlers, not just your new one. This ensures that `ECHO_HANDLER` and `STATIC_HANDLER` won’t accidentally accept & ignore your new keyword, if they don’t need it. 

11. Be sure to add the extracted information to `location_data.arg_map_` for your new handler. For an example, see how the `ROOT` data is added under the `STATIC_HANDLER` case


#### Example (StaticHandler):<a id="example-statichandler"></a>

\[ describe how one could follow the steps to add StaticHandler, if only EchoHandler exists ]


### For the Factory<a id="for-the-factory"></a>

\[ anywhere else stuff needs to be added ]
