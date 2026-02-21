FROM debian:unstable

RUN apt-get update && \
    apt-get install -q -y eatmydata && \
    eatmydata apt-get dist-upgrade -q -y && \
    eatmydata apt-get install -q -y \
        cmake ninja-build clang git pkg-config \
        qt6-base-dev qt6-base-private-dev \
        qt6-tools-dev qt6-tools-dev-tools \
        qt6-svg-dev \
        qt6-declarative-dev qt6-declarative-private-dev \
        qt6-5compat-dev \
        qt6-webchannel-dev \
        qt6-webengine-dev \
        qt6-scxml-dev \
        qt6-networkauth-dev \
        qt6-positioning-dev \
        libboost-dev \
        libboost-date-time-dev \
        libboost-filesystem-dev \
        libboost-program-options-dev \
        libboost-thread-dev \
        libboost-locale-dev \
        libboost-container-dev \
        libx11-dev \
        libx11-xcb-dev \
        libxkbfile-dev \
        libxcomposite-dev \
        libxrender-dev \
        libxdamage-dev \
        libxcb-xkb-dev \
        libxcb-render0-dev \
        libxcb-composite0-dev \
        libxcb-render-util0-dev \
        libxcb-damage0-dev \
        libxss-dev \
        libtorrent-rasterbar-dev \
        libmaxminddb-dev \
        libqxmppqt6-dev \
        libqca-qt6-dev \
        libotr5-dev \
        libpurple-dev \
        libtoxcore-dev \
        libhunspell-dev \
        libtag-dev \
        libgstreamer1.0-dev \
        libchromaprint-dev \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libmtp-dev \
        libpoppler-qt6-dev \
        libdjvulibre-dev \
        libquazip1-qt6-dev \
        libtidy-dev \
        libidn-dev \
        libqrencode-dev \
        libsensors-dev \
        libmagic-dev \
        libudev-dev \
        liblastfm6-dev \
        libqtermwidget-dev \
        libutf8proc-dev \
        libqscintilla2-qt6-dev \
    && apt-get clean