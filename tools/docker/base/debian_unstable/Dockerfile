FROM     debian:unstable

RUN apt-get install -q -y eatmydata

RUN eatmydata apt-get update
RUN eatmydata apt-get dist-upgrade -q -y

RUN eatmydata apt install -q -y cmake debhelper libaspell-dev \
        libboost-system-dev libboost-date-time-dev libboost-filesystem-dev \
        libboost-program-options-dev libboost-regex-dev libboost-thread-dev libboost-locale-dev \
        libbz2-dev libcurl4-openssl-dev libdjvulibre-dev libgeoip-dev libguess-dev libhunspell-dev \
        liblastfm5-dev libmagic-dev libmtp-dev libotr5-dev libpcre3-dev \
        libpoppler-cpp-dev libpoppler-qt5-dev libqca-qt5-2-dev libqt5scintilla2-dev \
        qtbase5-dev libqt5webkit5-dev libqt5xmlpatterns5-dev libqt5svg5-dev qttools5-dev-tools qttools5-dev libqt5x11extras5-dev qtscript5-dev  \
        libqwt-qt5-dev libqxmpp-dev \
        libspeex-dev libtag1-dev libtidy-dev libtorrent-rasterbar-dev \
        libudev-dev \
		libx11-dev libxkbfile-dev libxcomposite-dev libxrender-dev libxcb-xkb-dev libxcb-render0-dev libxcb-composite0-dev libxcb-render-util0-dev \
		libxml2-dev po-debconf x11proto-randr-dev zlib1g-dev \
        libnl-3-dev witty-dev libnl-route-3-dev libpurple-dev libvlc-dev libimobiledevice-dev libgpod-dev \
        libavcodec-dev libavformat-dev libavdevice-dev libavutil-dev libswscale-dev libswresample-dev libpostproc-dev libchromaprint-dev \
        git g++-7 gcc-7 \
        libgstreamer1.0-dev libsensors4-dev
