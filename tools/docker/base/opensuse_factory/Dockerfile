FROM opensuse/tumbleweed

RUN zypper ar 'https://download.opensuse.org/source/tumbleweed/repo/oss/' oss-src && \
    zypper ref && \
    zypper -n in eatmydata && \
    eatmydata zypper dup -y && \
    eatmydata zypper -n si -d leechcraft && \
    eatmydata zypper -n in git gzip libboost_container-devel liblastfm-qt6-devel qt6-networkauth-devel
