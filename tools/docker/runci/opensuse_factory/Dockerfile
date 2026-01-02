FROM leechcraft/ci_opensuse_factory:latest

WORKDIR /work

RUN --mount=type=bind,source=.,target=/work/leechcraft,ro \
  mkdir build && \
  cd build && \
  eatmydata cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ /work/leechcraft/src --preset opensuse_factory && \
  eatmydata cmake --build . -j$(nproc) -- -k
