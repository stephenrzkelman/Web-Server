### Base environment container ###
# Get the base Ubuntu image from Docker Hub
FROM ubuntu:jammy as base

ARG DEBIAN_FRONTEND=noninteractive

# Update the base image and install build environment
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    gcovr \
    git \
    httpie \
    libboost-log-dev \
    libboost-regex-dev \
    libboost-system-dev \
    libgmock-dev \
    libgtest-dev \
    netcat \
    python3 \
    pkg-config

# install cmark
RUN git clone https://github.com/commonmark/cmark.git && \
    cd cmark && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install
