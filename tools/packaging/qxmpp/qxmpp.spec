# LeechCraft CI build of qxmpp on top of Fedora's packaging (src.fedoraproject.org/rpms/qxmpp, 1.14.7),
# for the CI images while rawhide ships an older qxmpp than LeechCraft needs.
# qxmpp bumps its soname on every minor release.
%global sover 11

Name:           qxmpp
Version:        1.17.0
Release:        0.lc1%{?dist}
Summary:        Cross-platform C++ XMPP client and server library

# The library is under LGPL-2.1-or-later license
# the files in doc/ directory are CC0-1.0 license.
# the qxmpp logo is CC-BY-SA-4.0 license.
License:        LGPL-2.1-or-later AND CC0-1.0 AND CC-BY-SA-4.0
URL:            https://invent.kde.org/libraries/qxmpp
Source0:        %{url}/-/archive/v%{version}/qxmpp-v%{version}.tar.gz

BuildRequires:  gcc-c++
BuildRequires:  cmake
BuildRequires:  ninja-build
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Network)
BuildRequires:  cmake(Qt6Xml)
BuildRequires:  pkgconfig(openssl) >= 3.0
# QXmpp OMEMO module
BuildRequires:  pkgconfig(libomemo-c)
BuildRequires:  protobuf-c-devel
# calls
BuildRequires:  pkgconfig(gstreamer-1.0) >= 1.20

%global _description %{expand:
QXmpp is a cross-platform C++ XMPP client and server library. It is written in
C++ and uses Qt framework.

QXmpp strives to be as easy to use as possible, the underlying TCP socket, the
core XMPP RFCs (RFC6120 and RFC6121) and XMPP extensions have been nicely
encapsulated into classes. QXmpp is ready to build XMPP clients complying with
the XMPP Compliance Suites 2022 for IM and Advanced Mobile. It comes with full
API documentation, automatic tests and some examples.}

%description
%{_description}

%package        qt6
Summary:        QXmpp library for Qt6

%description    qt6
%{_description}

%package        qt6-devel
Summary:        Development Files for %{name}-qt6
Requires:       %{name}-qt6%{?_isa} = %{version}-%{release}

%description    qt6-devel
This package contains libraries and header files for developing applications
that use %{name}-qt6.

%prep
%autosetup -p1 -n qxmpp-v%{version}

%build
%cmake -GNinja \
    -DBUILD_TESTING=OFF \
    -DBUILD_DOCUMENTATION=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_OMEMO=ON \
    -DWITH_ENCRYPTION=ON \
    -DWITH_GSTREAMER=ON
%cmake_build

%install
%cmake_install

%files qt6
%license LICENSES/*
%doc README.md
%{_libdir}/libQXmppQt6.so.%{sover}
%{_libdir}/libQXmppQt6.so.%{version}
%{_libdir}/libQXmppOmemoQt6.so.%{sover}
%{_libdir}/libQXmppOmemoQt6.so.%{version}

%files qt6-devel
%{_libdir}/libQXmppQt6.so
%{_libdir}/libQXmppOmemoQt6.so
%{_includedir}/QXmppQt6/
%{_libdir}/cmake/QXmppQt6/
%{_libdir}/cmake/QXmppOmemoQt6/
%{_libdir}/pkgconfig/QXmppQt6.pc

%changelog
* Fri Oct 23 2026 LeechCraft CI <0xd34df00d@gmail.com> - 1.17.0-0.lc1
- Upstream 1.17.0 on top of Fedora's 1.14.7 packaging, for the LeechCraft CI
  images while rawhide ships an older qxmpp than LeechCraft needs
- Soname 11; the build options upstream renamed in 1.15: BUILD_TESTING, and
  WITH_ENCRYPTION (OpenSSL) instead of QCA
- No documentation, no tests
