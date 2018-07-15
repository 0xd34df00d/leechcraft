FROM opensuse/leap:15.0

RUN zypper ar 'https://download.opensuse.org/source/distribution/leap/15.0/repo/oss/' oss-src

RUN zypper ref
RUN zypper -n in eatmydata
RUN eatmydata zypper dup -y

RUN eatmydata zypper -n si -d leechcraft
RUN eatmydata zypper -n in libqscintilla_qt5-devel git
