FROM opensuse/tumbleweed

RUN zypper ar 'https://download.opensuse.org/source/tumbleweed/repo/oss/' oss-src

RUN zypper ref
RUN zypper -n in eatmydata
RUN eatmydata zypper dup -y

RUN eatmydata zypper -n si -d leechcraft
RUN eatmydata zypper -n in libqscintilla_qt5-devel git
