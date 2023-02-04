ARG GCC_VERSION=9.5.0
ARG CMAKE_BUILD_TYPE=Debug
FROM gcc:${GCC_VERSION}

RUN apt-get update -y \
 && apt-get install -y \
    cmake \
    libgflags-dev \
    libgoogle-glog-dev \
    libgtest-dev \
 && mkdir -p /usr/share/poseidon/

WORKDIR /usr/share/poseidon/
COPY * .
RUN mkdir build/ \
 && cd build/ \
 && cmake -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE .. \
 && cmake build .