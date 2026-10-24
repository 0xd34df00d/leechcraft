#!/usr/bin/env sh
# Usage: [CCACHE_HOST_DIR=<absolute path>] runci.sh <distro> <gcc|clang> [build|tidy|version|stdlib]
#
# build and tidy build the checkout in the current directory inside the leechcraft/ci_<distro> image: from
# scratch, in a tmpfs, without network access, on every core. With CCACHE_HOST_DIR set, the compilers run through
# ccache with that directory as the cache, which may grow to nine tenths of the filesystem holding it, and the
# output ends with the cache statistics of the build; an image without ccache builds without it and says so.
# Exit status: 0 on success, 1 when the build fails, 2 when
# configure fails (on a rolling distro that almost always means a dependency moved), 64 on bad arguments; 126
# and 127 from a tool missing inside the image are passed through. version prints the major version of that
# compiler in the image, or `missing`; stdlib prints the release of the libstdc++ that compiler uses, or `missing`.
set -eu

distro=${1:?usage: runci.sh <distro> <gcc|clang> [build|tidy|version|stdlib]}
case ${2:?compiler} in
	gcc) cc=gcc; cxx=g++ ;;
	clang) cc=clang; cxx=clang++ ;;
	*) echo "unknown compiler: $2" >&2; exit 64 ;;
esac
image=leechcraft/ci_$distro:latest

# the filesystem holding the cache is meant for it; the tenth left over is headroom for ccache's temporary files
ccache_opts=
if [ -n "${CCACHE_HOST_DIR:-}" ]; then
	cap_mb=$(( $(df -Pk "$CCACHE_HOST_DIR" | awk 'NR == 2 { print $2 }') / 1024 * 9 / 10 ))
	ccache_opts="--mount type=bind,src=$CCACHE_HOST_DIR,dst=/work/ccache --env CCACHE_DIR=/work/ccache --env CCACHE_MAXSIZE=${cap_mb}M"
fi

case ${3:-build} in
	build) cmake_extra= ;;
	tidy) cmake_extra=-DCMAKE_CXX_CLANG_TIDY=clang-tidy ;;
	version)
		v=$(docker run --rm --network=none "$image" sh -c \
			"command -v $cxx >/dev/null || { echo missing; exit 0; }; $cxx -dumpfullversion -dumpversion 2>/dev/null || $cxx -dumpversion")
		echo "${v%%.*}"
		exit 0
		;;
	stdlib)
		v=$(docker run --rm --network=none "$image" sh -c \
			"command -v $cxx >/dev/null || exit 0; echo '#include <version>' | $cxx -x c++ -E -dM - 2>/dev/null | sed -n 's/^#define _GLIBCXX_RELEASE //p'")
		echo "${v:-missing}"
		exit 0
		;;
	*) echo "unknown mode: $3" >&2; exit 64 ;;
esac

exec docker run \
	--rm \
	--network=none \
	--workdir /work \
	--mount type=bind,src="$PWD",dst=/work/leechcraft \
	--mount type=tmpfs,dst=/work/build \
	$ccache_opts \
	"$image" \
	sh -lc "
		cd /work/build &&
		launcher= &&
		if [ -n \"\${CCACHE_DIR:-}\" ]; then
			if command -v ccache >/dev/null; then
				launcher='-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache' && ccache -z
			else
				echo 'ccache requested, but the image has none: building without it'
			fi
		fi &&
		{ cmake /work/leechcraft/src --preset $distro -G Ninja -DCMAKE_C_COMPILER=$cc -DCMAKE_CXX_COMPILER=$cxx $cmake_extra \$launcher \
			|| { s=\$?; [ \$s -ge 126 ] && exit \$s; exit 2; }; } &&
		{ cmake --build . -j$(nproc) -- -k0 && s=0 || s=1; [ -z \"\$launcher\" ] || ccache -s; exit \$s; }
	"
