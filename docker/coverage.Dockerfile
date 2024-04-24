### Build/test container ###
# Define builder stage
FROM fortnite-gamers:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and tset 
RUN cmake ..
RUN make 
RUN ctest --output-on_failure 

# Build and test
WORKDIR /usr/src/project/build_coverage
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage
