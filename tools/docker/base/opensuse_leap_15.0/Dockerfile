FROM opensuse/leap:15.0

RUN zypper -n ar 'https://download.opensuse.org/source/distribution/leap/15.0/repo/oss/' oss-src
RUN zypper -n ar -G 'https://download.opensuse.org/repositories/network/openSUSE_Leap_15.0/network.repo'

RUN zypper ref
RUN zypper -n in eatmydata
RUN eatmydata zypper dup -y

RUN eatmydata zypper -n si -d leechcraft
RUN eatmydata zypper -n in libqscintilla_qt5-devel git gzip
