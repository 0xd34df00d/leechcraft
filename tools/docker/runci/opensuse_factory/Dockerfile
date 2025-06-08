FROM leechcraft/ci_opensuse_factory:latest

RUN git clone https://github.com/0xd34df00d/leechcraft.git && \
  cd leechcraft && \
  mkdir build && \
  cd build && \
  cmake ../src --preset opensuse_factory && \
  make -j$(nproc) -k
