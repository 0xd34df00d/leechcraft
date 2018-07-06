FROM leechcraft/ci_debian_unstable:latest

RUN git clone https://github.com/0xd34df00d/leechcraft.git && \
	cd leechcraft && \
	mkdir build && \
	cd build && \
	../tools/scripts/config_jenkins.sh debian_unstable && \
	make -j$(nproc) -k
