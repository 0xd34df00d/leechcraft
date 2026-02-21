FROM opensuse/leap:16.0

RUN zypper ar -G 'https://download.opensuse.org/repositories/network/16.0/network.repo' && \
    zypper ar -G 'https://download.opensuse.org/repositories/devel:/tools:/compiler/16.0/devel:tools:compiler.repo' && \
    zypper ref && \
    zypper -n si -d leechcraft && \
    zypper -n in git gzip libboost_container-devel liblastfm-qt6-devel qt6-networkauth-devel c-toxcore-devel && \
    zypper -n in clang ninja && \
    zypper clean -a
