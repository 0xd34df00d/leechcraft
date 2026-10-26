#!/usr/bin/env sh
# Usage: [IMAGE_PREFIX=leechcraft/ci_] buildimage.sh <distro> <state directory>
#
# Rebuilds the CI image of a distro from scratch (no layer cache, base image pulled afresh) from
# tools/docker/base/<distro>/Dockerfile, with the distro's directory as the build context, and tags it
# <IMAGE_PREFIX><distro>:latest. Before the build it lists the packages of the image that holds the tag so far, after
# the build those of the new one, and writes into the state directory:
#   <distro>.manifest   the new image's packages, one "name version" per line
#   <distro>.diff       what changed against the previous image (empty for none, whole list for a first image)
#   <distro>.toolchain  the versions of gcc, clang, Qt and Boost in the image, or "missing"; "name old → new" for one
#                       the previous image had in another version
#   <distro>.status     changed | unchanged | unknown: whether the package list differs from the previous image's;
#                       unknown when a list could not be taken, which counts as changed downstream
#   packages/<distro>/  what the Dockerfile's `packages` stage produces, for Dockerfiles that have one
# The diff is what to read when a cell breaks the morning after a rebuild.
# Exit status: 0 when the image was built, 1 when the build failed (the previous image keeps its tag), 3 when the
# image was built but listing or exporting failed, 64 on bad arguments.
set -eu

distro=${1:?usage: buildimage.sh <distro> <state directory>}
state=${2:?usage: buildimage.sh <distro> <state directory>}
# the state directory is relative to the caller, so it is resolved before the script moves to tools/
mkdir -p "$state"
state=$(cd "$state" && pwd)
cd "$(dirname "$0")/.."
dockerfile=docker/base/$distro/Dockerfile
[ -f "$dockerfile" ] || { echo "no such distro: $distro ($dockerfile missing)" >&2; exit 64; }
# The Debian and Fedora Dockerfiles fetch the qxmpp packaging from its own repository at this commit; see the CI README.
packaging_ref=$(cat packaging.ref)
printf '%s' "$packaging_ref" | grep -Eq '^[0-9a-f]{40}$' \
	|| { echo "packaging.ref must hold the full hash of a leechcraft-packaging commit, not '$packaging_ref'" >&2; exit 64; }
# passed only to the Dockerfiles that declare it, so the other builds do not warn about an unused build argument
build_args=
if grep -q '^ARG PACKAGING_REF$' "$dockerfile"; then
	build_args="--build-arg PACKAGING_REF=$packaging_ref"
fi
image=${IMAGE_PREFIX:-leechcraft/ci_}$distro:latest

# One "name version" line per installed package, whichever package manager the image has. The listing is captured
# before sorting, so that a failing docker run fails the function rather than vanishing behind the pipe; an empty
# listing fails it too, since no image that can build anything has no packages.
manifest()
{
	listing=$(docker run --rm --network=none "$1" sh -c '
		if command -v dpkg-query >/dev/null; then dpkg-query -W -f "\${binary:Package} \${Version}\n"
		elif command -v rpm >/dev/null; then rpm -qa --qf "%{NAME} %{EVR}.%{ARCH}\n"
		elif command -v pacman >/dev/null; then pacman -Q
		elif command -v apk >/dev/null; then apk info -v 2>/dev/null
		elif [ -d /var/db/pkg ]; then ls -d /var/db/pkg/*/* | sed -E "s|^/var/db/pkg/||; s/-([0-9][^-]*(-r[0-9]+)?)\$/ \1/"
		else echo "no known package manager (dpkg, rpm, pacman, apk, portage)" >&2; exit 1
		fi') || return 1
	[ -n "$listing" ] || { echo "empty package list" >&2; return 1; }
	printf '%s\n' "$listing" | sort
}

# The versions that decide what a build sees, one "name version" per line. Asked from the tools and headers rather than
# from the package manager: the package names differ per distro, and a package revision shipping the same version is no
# change here.
toolchain()
{
	docker run --rm --network=none "$1" sh -c '
		for c in gcc:g++ clang:clang++; do
			name=${c%%:*}; bin=${c#*:}
			if command -v $bin >/dev/null; then echo "$name $($bin -dumpfullversion -dumpversion 2>/dev/null || $bin -dumpversion)"
			else echo "$name missing"
			fi
		done
		qt=$(pkg-config --modversion Qt6Core 2>/dev/null || qmake6 -query QT_VERSION 2>/dev/null)
		echo "Qt ${qt:-missing}"
		boost=$(sed -n "s/^#define BOOST_VERSION //p" /usr/include/boost/version.hpp 2>/dev/null)
		if [ -n "$boost" ]; then echo "Boost $((boost / 100000)).$((boost / 100 % 1000)).$((boost % 100))"
		else echo "Boost missing"
		fi'
}

# Rewrites "name version" lines from stdin as "name old → new" where the file given, the previous image's versions,
# has the name with another version.
mark_changed()
{
	while read -r name version; do
		[ -n "$name" ] || continue
		old=$(sed -n "s/^$name //p" "$1")
		if [ -n "$old" ] && [ "$old" != "$version" ]; then
			echo "$name $old → $version"
		else
			echo "$name $version"
		fi
	done
}

previous=$(docker image inspect --format '{{.Id}}' "$image" 2>/dev/null || true)
# A listing that fails leaves the verdict unknown, which the job treats as changed and, through the exit status, as
# unstable, so that a package manager this script does not know gets noticed rather than hidden behind "unchanged".
comparable=true
: > "$state/$distro.previous"
: > "$state/$distro.toolchain.previous"
if [ -n "$previous" ]; then
	manifest "$previous" > "$state/$distro.previous" || comparable=false
	toolchain "$previous" > "$state/$distro.toolchain.previous" || : > "$state/$distro.toolchain.previous"
fi

docker build --pull --no-cache --network=host $build_args -f "$dockerfile" -t "$image" "docker/base/$distro" || exit 1

status=0
manifest "$image" > "$state/$distro.manifest" || comparable=false
current=$(toolchain "$image") || status=3
printf '%s\n' "$current" | mark_changed "$state/$distro.toolchain.previous" > "$state/$distro.toolchain"
if $comparable; then
	if cmp -s "$state/$distro.previous" "$state/$distro.manifest"; then
		verdict=unchanged
	else
		verdict=changed
	fi
	diff "$state/$distro.previous" "$state/$distro.manifest" > "$state/$distro.diff" || true
else
	verdict=unknown
	status=3
	: > "$state/$distro.diff"
fi
echo $verdict > "$state/$distro.status"
rm -f "$state/$distro.previous" "$state/$distro.toolchain.previous"

toolchain_summary=$(tr '\n' ',' < "$state/$distro.toolchain" | sed 's/,$//; s/,/, /g')
if [ $verdict = unknown ]; then
	echo "=== $distro: the packages could not be listed, so the image counts as changed; $toolchain_summary"
else
	echo "=== $distro: $(wc -l < "$state/$distro.manifest") packages, $verdict since the previous image; $toolchain_summary"
fi
if [ -z "$previous" ]; then
	echo "=== $distro: no previous image to compare with"
elif [ -s "$state/$distro.diff" ]; then
	echo "=== $distro: package changes (< previous image, > new image)"
	cat "$state/$distro.diff"
fi

# Anything the `packages` stage produces (the qxmpp packages of the Debian and Fedora images) is exported for the
# record. The stage was built a moment ago, so this is a cache hit, not a second build.
if grep -q '^FROM .* AS packages$' "$dockerfile"; then
	rm -rf "$state/packages/$distro"
	docker build --network=host $build_args -f "$dockerfile" --target packages --output "type=local,dest=$state/packages/$distro" "docker/base/$distro" \
		|| { echo "=== $distro: exporting the packages stage failed" >&2; status=3; }
fi
exit $status
