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
COPY --from=builder /usr/src/project/server_config_deploy .
COPY --from=builder /usr/src/project/files /usr/src/projects/fortnite-gamers/files
EXPOSE 8080

# Use ENTRYPOINT to specify the binary name
ENTRYPOINT ["./server"]

# Use CMD to specify arguments to ENTRYPOINT
CMD ["server_config_deploy"]
