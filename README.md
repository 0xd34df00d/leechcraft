[![Build Status](http://leechcraft.org:8080/job/leechcraft/badge/icon)](http://leechcraft.org:8080/job/leechcraft/)

LeechCraft is a (C++ and Qt-based) modular live environment: a kind
of a desktop environment with a focus on the actual applications for
some typical desktop tasks and their interoperation. For instance,
LeechCraft allows one to:
* browse the web;
* chat via XMPP, IRC, Tox and a bunch of other protocols;
* read RSS feeds;
* listen to music in an Amarok-like audio player;
* view documents in PDF, FB2 and other formats;
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
