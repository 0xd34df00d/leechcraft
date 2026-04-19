FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
        cmake ninja clang git pkgconf \
        qt6-base \
        qt6-tools \
        qt6-svg \
        qt6-declarative \
        qt6-webengine \
        qt6-scxml \
        qt6-networkauth \
        boost \
        libx11 \
        libxkbfile \
        libxcomposite \
        libxrender \
        libxdamage \
        libxcb \
        xcb-util-renderutil \
        libxss \
        libtorrent-rasterbar \
        libmaxminddb \
        qxmpp \
        qca-qt6 \
        libotr \
        toxcore \
        hunspell \
        taglib \
        gstreamer \
        chromaprint \
        ffmpeg \
        libmtp \
        poppler-qt6 \
        djvulibre \
        quazip-qt6 \
        tidy \
        libidn \
        qrencode \
        lm_sensors \
        systemd \
        libnl \
        libutf8proc \
        qscintilla-qt6 \
        qtermwidget \
    && pacman -Scc --noconfirm