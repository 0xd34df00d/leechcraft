[![Build Status](https://jenkins.leechcraft.org/job/leechcraft/badge/icon)](http://jenkins.leechcraft.org/job/leechcraft/)

<details>
<summary>Full build status</summary>

| gcc | clang |
| --- | --- |
| [![Alpine Edge / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=alpine_edge-gcc&subject=Alpine%20Edge%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![Alpine Edge / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=alpine_edge-clang&subject=Alpine%20Edge%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
| [![Arch Linux / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=arch_linux-gcc&subject=Arch%20Linux%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![Arch Linux / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=arch_linux-clang&subject=Arch%20Linux%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
| [![Debian Experimental / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=debian_experimental-gcc&subject=Debian%20Experimental%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![Debian Experimental / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=debian_experimental-clang&subject=Debian%20Experimental%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
| [![Debian Unstable / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=debian_unstable-gcc&subject=Debian%20Unstable%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![Debian Unstable / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=debian_unstable-clang&subject=Debian%20Unstable%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
| [![Fedora Rawhide / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=fedora_rawhide-gcc&subject=Fedora%20Rawhide%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![Fedora Rawhide / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=fedora_rawhide-clang&subject=Fedora%20Rawhide%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
| [![Gentoo Stable / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=gentoo_stable-gcc&subject=Gentoo%20Stable%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![Gentoo Stable / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=gentoo_stable-clang&subject=Gentoo%20Stable%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
| [![openSUSE Factory / gcc](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=opensuse_factory-gcc&subject=openSUSE%20Factory%20/%20gcc)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) | [![openSUSE Factory / clang](https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=opensuse_factory-clang&subject=openSUSE%20Factory%20/%20clang)](https://jenkins.leechcraft.org/job/leechcraft-full/lastCompletedBuild/) |
</details>

LeechCraft is a (C++ and Qt-based) modular live environment: a kind
of a desktop environment with a focus on the actual applications for
some typical desktop tasks and their interoperation. For instance,
LeechCraft allows one to:
* browse the web;
* chat via XMPP, IRC, Tox and a bunch of other protocols;
* read RSS feeds;
* listen to music in an Amarok-like audio player;
* view documents in PDF, ePub, FB2 and other formats;
* download BitTorrent files;
* share files via cloud;
* post to blogs;
* and much more.

Each function is provided by a separate module which can be omitted at build
or run time if not needed.

A more or less up-to-date list of plugins is available here:
https://leechcraft.org/plugins

## Getting LeechCraft

Getting precompiled LeechCraft for various OS is documented here:
https://leechcraft.org/download

## Building LeechCraft

In short:
```
git clone git://github.com/0xd34df00d/leechcraft.git
cd leechcraft
mkdir build && cd build
cmake ../src
make
```

Prerequisites and more details are available at: https://leechcraft.org/development-building-from-source

## Translations

LeechCraft uses hosted [Weblate](https://hosted.weblate.org/projects/leechcraft/)
for translations, kindly provided to opensource projects.
