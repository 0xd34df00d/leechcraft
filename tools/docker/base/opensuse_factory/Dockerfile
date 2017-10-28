FROM opensuse:tumbleweed

RUN zypper ar 'https://download.opensuse.org/source/tumbleweed/repo/oss/' oss-src

RUN zypper ref
RUN zypper dup -y

RUN zypper -n si -d leechcraft
RUN zypper -n in libqscintilla_qt5-devel git
