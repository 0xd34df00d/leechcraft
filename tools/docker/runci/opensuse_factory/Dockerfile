FROM leechcraft/ci_opensuse_factory:latest

WORKDIR /work

RUN --mount=type=bind,source=.,target=/work/leechcraft,rw \
    --mount=type=tmpfs,target=/work/build \
  cd /work/build && \
  eatmydata cmake /work/leechcraft/src --preset opensuse_factory_clang && \
  eatmydata cmake --build . -j$(nproc) -- -k
