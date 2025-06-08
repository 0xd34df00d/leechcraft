FROM opensuse/tumbleweed

RUN <<EOF
zypper ar 'https://download.opensuse.org/source/tumbleweed/repo/oss/' oss-src

zypper ref
zypper -n in eatmydata
eatmydata zypper dup -y

eatmydata zypper -n si -d leechcraft
eatmydata zypper -n in git gzip
eatmydata zypper -n in libboost_container-devel
eatmydata zypper -n in liblastfm-qt6-devel
EOF
