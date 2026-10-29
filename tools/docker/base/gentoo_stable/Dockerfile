FROM gentoo/portage:latest AS portage

FROM gentoo/stage3:desktop

# The official binhost holds the stable tree only, each package in a few USE combinations of its own:
# --binpkg-respect-use=n takes them whatever this profile's USE, and a USE dependency below asks for a feature
# LeechCraft needs. qtwebengine comes from the binhost or not at all, so that a binhost lagging behind a version bump
# fails the build, which keeps the previous image, rather than compiling Chromium. The packages the binhost lacks are
# small: qxmpp, libtorrent-rasterbar, tox, libotr, djvu, liblastfm, qwt and mold.
# The tree is mounted rather than copied, so that it stays out of the image.
RUN --mount=type=bind,from=portage,source=/var/db/repos/gentoo,target=/var/db/repos/gentoo \
    getuto && \
    echo 'app-text/poppler qt6' > /etc/portage/package.use/leechcraft && \
    export MAKEOPTS="-j$(nproc)" \
        EMERGE_DEFAULT_OPTS="--getbinpkg --binpkg-respect-use=n --noreplace --jobs=8 --load-average=$(nproc)" && \
    emerge --getbinpkgonly dev-qt/qtwebengine && \
    emerge --usepkg \
        dev-build/cmake dev-build/ninja dev-util/ccache sys-devel/mold dev-vcs/git \
        dev-qt/qtbase \
        dev-qt/qttools \
        dev-qt/qtsvg \
        dev-qt/qtdeclarative \
        dev-qt/qtscxml \
        dev-qt/qtnetworkauth \
        dev-libs/boost \
        x11-libs/libX11 \
        x11-libs/libxkbfile \
        x11-libs/libXcomposite \
        x11-libs/libXrender \
        x11-libs/libXdamage \
        x11-libs/libxcb \
        x11-libs/xcb-util-renderutil \
        x11-libs/libXScrnSaver \
        net-libs/libtorrent-rasterbar \
        dev-libs/libmaxminddb \
        net-libs/qxmpp \
        app-crypt/qca \
        net-libs/libotr \
        net-libs/tox \
        app-text/hunspell \
        media-libs/taglib \
        media-libs/gstreamer \
        media-libs/chromaprint \
        media-video/ffmpeg \
        media-libs/libmtp \
        'app-text/poppler[qt6]' \
        app-text/djvu \
        dev-libs/quazip \
        app-text/htmltidy \
        net-dns/libidn \
        media-gfx/qrencode \
        sys-apps/lm-sensors \
        virtual/libudev \
        dev-libs/libnl \
        media-libs/liblastfm \
        x11-libs/qscintilla \
        x11-libs/qtermwidget \
        x11-libs/qwt \
    && rm -rf /var/cache/binpkgs/* /var/cache/distfiles/*
