FROM leechcraft/ci_opensuse_factory:latest

RUN eatmydata git clone https://github.com/0xd34df00d/leechcraft.git && \
  cd leechcraft && \
  mkdir build && \
  cd build && \
  eatmydata cmake ../src --preset opensuse_factory && \
  eatmydata cmake --build . -j$(nproc) -- -k
