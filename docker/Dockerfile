### Build/test container ###
# Define builder stage
FROM fortnite-gamers:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake ..
RUN make
RUN ctest --output-on_failure


### Deploy container ###
# Define deploy stage
FROM ubuntu:jammy as deploy

COPY --from=builder /usr/src/project/build/bin/server .
COPY --from=builder /usr/src/project/docker_config .

EXPOSE 8080

# Use ENTRYPOINT to specify the binary name
# TODO(!): Update with real server name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
# TODO(!): Update with real server args
CMD ["docker_config"]
