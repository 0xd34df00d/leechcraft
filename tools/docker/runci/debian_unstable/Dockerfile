FROM leechcraft/ci_debian_unstable:latest

RUN git clone https://github.com/0xd34df00d/leechcraft.git && \
	cd leechcraft && \
	mkdir build && \
	cd build && \
	cmake ../src \
		-DEXTRA_WARNINGS=True \
		-DENABLE_BLASQ=True \
		-DENABLE_BLASQ_SPEGNERSI=False \
		-DENABLE_AZOTH_WOODPECKER=False \
		-DENABLE_OTLOZHU=True \
		-DENABLE_OTLOZHU_SYNC=False \
		-DENABLE_SYNCER=False \
		-DENABLE_MUSICZOMBIE=True \
		-DENABLE_AZOTH_OTROID=True \
		-DENABLE_AZOTH_VELVETBIRD=True \
		-DENABLE_AZOTH_ZHEET=False \
		-DENABLE_LMP_MTPSYNC=True \
		-DENABLE_LMP_JOS=True \
		-DENABLE_POPISHU=False \
		-DENABLE_VROOBY_UDISKS2=True \
		-DENABLE_MEDIACALLS=False \
		-DENABLE_POLEEMERY=False \
		-DENABLE_AGGREGATOR_WEBACCESS=False \
		-DENABLE_OTZERKALU=True \
		-DENABLE_AZOTH_ASTRALITY=False \
		-DENABLE_ELEEMINATOR=False \
		-DENABLE_POSHUKU_DCAC_TESTS=False \
		-DENABLE_XPROXY=True \
		-DENABLE_MELLONETRAY=False \
		-DENABLE_FONTIAC=True && \
	make -j$(nproc) -k
