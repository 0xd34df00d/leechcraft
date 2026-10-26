FROM fedora:rawhide AS qxmpp
COPY packaging/qxmpp /build/packaging/qxmpp
RUN /build/packaging/qxmpp/build.sh /out

FROM scratch AS packages
COPY --from=qxmpp /out /

FROM fedora:rawhide

RUN --mount=type=bind,from=qxmpp,source=/out,target=/tmp/qxmpp \
    dnf install -y \
        cmake ninja-build ccache mold clang git pkgconfig \
        qt6-qtbase-devel qt6-qtbase-private-devel \
        qt6-qttools-devel \
        qt6-qtsvg-devel \
        qt6-qtdeclarative-devel \
        qt6-qtwebengine-devel \
        qt6-qtscxml-devel \
        qt6-qtnetworkauth-devel \
        boost-devel \
        libX11-devel \
        libxkbfile-devel \
        libXcomposite-devel \
        libXrender-devel \
        libXdamage-devel \
        libxcb-devel \
        xcb-util-renderutil-devel \
        libXScrnSaver-devel \
        rb_libtorrent-devel \
        libmaxminddb-devel \
        /tmp/qxmpp/*.rpm \
        qca-qt6-devel \
        libotr-devel \
        toxcore-devel \
        hunspell-devel \
        taglib-devel \
        gstreamer1-devel \
        libchromaprint-devel \
        ffmpeg-free-devel \
        libmtp-devel \
        poppler-qt6-devel \
        djvulibre-devel \
        quazip-qt6-devel \
        libidn-devel \
        qrencode-devel \
        lm_sensors-devel \
        systemd-devel \
        libnl3-devel \
        liblastfm-qt6-devel \
        qscintilla-qt6-devel \
        qtermwidget-devel \
        qwt-qt6-devel \
    && dnf clean all
