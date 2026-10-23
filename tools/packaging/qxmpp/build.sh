#!/bin/sh
# Builds qxmpp as the running distro's own package format from the packaging next to this script:
# debian/ on Debian, qxmpp.spec on Fedora. The CI images of those distros use it while the distro ships
# a qxmpp older than LeechCraft needs (the pkg-config floor in src/plugins/azoth/plugins/xoox/CMakeLists.txt).
#
# Usage: build.sh <output directory for the packages>
#
# Fails on purpose once the distro itself ships a qxmpp satisfying LeechCraft: at that point the stage
# calling this script has to go, so the image goes back to testing the distro as it is.
set -eu

floor=1.16.0

here=$(cd "$(dirname "$0")" && pwd)
out=${1:?usage: build.sh <output directory>}
. /etc/os-release

tarball_url()
{
	echo "https://invent.kde.org/libraries/qxmpp/-/archive/v$1/qxmpp-v$1.tar.gz"
}

distro_caught_up()
{
	cat >&2 <<-MSG
		$PRETTY_NAME ships qxmpp $1, which satisfies LeechCraft's requirement of >= $floor.
		Drop the qxmpp stage from this image's Dockerfile under tools/docker/base/ and install the
		distro's qxmpp development package again.
	MSG
	exit 1
}

case $ID in
debian)
	version=$(sed -n '1s/^qxmpp (\([0-9.]*\)-[^)]*).*/\1/p' "$here/debian/changelog")
	apt-get update
	candidate=$(apt-cache policy libqxmppqt6-dev | sed -n 's/^ *Candidate: //p')
	case $candidate in
		''|'(none)') ;;
		*) dpkg --compare-versions "$candidate" lt "$floor" || distro_caught_up "$candidate" ;;
	esac
	apt-get install -y --no-install-recommends build-essential ca-certificates curl
	mkdir -p /build && cd /build
	curl -fsSL "$(tarball_url "$version")" -o "qxmpp_$version.orig.tar.gz"
	tar xzf "qxmpp_$version.orig.tar.gz"
	cd "qxmpp-v$version"
	cp -r "$here/debian" .
	apt-get build-dep -y --no-install-recommends ./
	dpkg-buildpackage -b -us -uc
	mkdir -p "$out"
	mv ../*.deb "$out"
	;;
fedora)
	version=$(sed -n 's/^Version:[[:space:]]*//p' "$here/qxmpp.spec")
	candidates=$(dnf -q repoquery --latest-limit=1 --queryformat='%{version}\n' qxmpp-qt6-devel)
	candidate=$(printf '%s\n' $candidates | sort -V | tail -n 1)
	if [ -n "$candidate" ] && [ "$(printf '%s\n%s\n' "$floor" "$candidate" | sort -V | head -n 1)" = "$floor" ]; then
		distro_caught_up "$candidate"
	fi
	dnf install -y rpm-build dnf5-plugins
	command -v curl >/dev/null || dnf install -y curl
	mkdir -p ~/rpmbuild/SOURCES
	curl -fsSL "$(tarball_url "$version")" -o ~/rpmbuild/SOURCES/"qxmpp-v$version.tar.gz"
	dnf builddep -y "$here/qxmpp.spec"
	rpmbuild -bb "$here/qxmpp.spec"
	mkdir -p "$out"
	find ~/rpmbuild/RPMS -name '*.rpm' ! -name '*-debug*' -exec mv {} "$out" \;
	;;
*)
	echo "no qxmpp packaging for $ID here" >&2
	exit 64
	;;
esac
