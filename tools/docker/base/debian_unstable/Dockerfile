FROM debian:unstable AS qxmpp
COPY packaging/qxmpp /build/packaging/qxmpp
RUN /build/packaging/qxmpp/build.sh /out

FROM scratch AS packages
COPY --from=qxmpp /out /

FROM debian:unstable

RUN --mount=type=bind,from=qxmpp,source=/out,target=/tmp/qxmpp \
    apt-get update && \
    apt-get install -q -y eatmydata && \
    eatmydata apt-get dist-upgrade -q -y && \
    eatmydata apt-get install -q -y \
        cmake ninja-build ccache mold g++ clang-23 git pkg-config \
        qt6-base-dev qt6-base-private-dev \
        qt6-tools-dev qt6-tools-dev-tools \
        qt6-svg-dev \
        qt6-declarative-dev qt6-declarative-private-dev \
        qt6-webengine-dev \
        qt6-scxml-dev \
        qt6-networkauth-dev \
        libboost-dev \
        libboost-program-options-dev \
        libboost-container-dev \
        libx11-dev \
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
        /tmp/qxmpp/*.deb \
        libqca-qt6-dev \
        libotr5-dev \
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
        libudev-dev \
        liblastfm6-dev \
        libqtermwidget-dev \
        # qtermwidget6's CMake target links libutf8proc.so by path, and libqtermwidget-dev forgets to depend on it
        libutf8proc-dev \
        libqscintilla2-qt6-dev \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-23 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-23 100 \
    && apt-get clean