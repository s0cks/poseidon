ARG ALPINE_VERSION=3.17.1
FROM alpine:${ALPINE_VERSION}
ARG CMAKE_BUILD_TYPE=release

RUN apk update \
 && apk upgrade \
 && apk add --no-cache \
    clang \
    clang-dev \
    alpine-sdk \
    dpkg \
    make \
    cmake \
    ccache \
    python3 \
    gtest-dev \
    glog-dev \
    gflags-dev \
    libunwind-dev

RUN ln -sf /usr/bin/clang /usr/bin/cc \
  && ln -sf /usr/bin/clang++ /usr/bin/c++ \
  && update-alternatives --install /usr/bin/cc cc /usr/bin/clang 10\
  && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 10\
  && update-alternatives --auto cc \
  && update-alternatives --auto c++ \
  && update-alternatives --display cc \
  && update-alternatives --display c++ \
  && ls -l /usr/bin/cc /usr/bin/c++ \
  && cc --version \
  && c++ --version

WORKDIR /usr/src/poseidon/
COPY Sources/ /usr/src/poseidon/Sources
COPY Tests/ /usr/src/poseidon/Tests
COPY CMakeLists.txt /usr/src/poseidon

RUN ls -lash . \
 && mkdir build/ \
 && cd build/ \
 && export CC=/usr/bin/clang \
 && export CXX=/usr/bin/clang++ \
 && cmake \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    .. \
 && cmake --build . \
 && cd "build/${CMAKE_BUILD_TYPE}/Tests" \
 && ctest