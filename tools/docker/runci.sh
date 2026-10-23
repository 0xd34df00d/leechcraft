#!/usr/bin/env sh
# Usage: runci.sh <distro> <gcc|clang> [build|tidy|version]
#
# build and tidy build the checkout in the current directory inside the leechcraft/ci_<distro> image: from
# scratch, in a tmpfs, without network access, on every core. Exit status: 0 on success, 1 when the build
# fails, 2 when configure fails (on a rolling distro that almost always means a dependency moved), 64 on bad
# arguments. version prints the major version of that compiler in the image, or `missing`.
set -eu

distro=${1:?usage: runci.sh <distro> <gcc|clang> [build|tidy|version]}
case ${2:?compiler} in
	gcc) cc=gcc; cxx=g++ ;;
	clang) cc=clang; cxx=clang++ ;;
	*) echo "unknown compiler: $2" >&2; exit 64 ;;
esac
image=leechcraft/ci_$distro:latest

case ${3:-build} in
	build) cmake_extra= ;;
	tidy) cmake_extra=-DCMAKE_CXX_CLANG_TIDY=clang-tidy ;;
	version)
		v=$(docker run --rm --network=none "$image" sh -c \
			"command -v $cxx >/dev/null || { echo missing; exit 0; }; $cxx -dumpfullversion -dumpversion 2>/dev/null || $cxx -dumpversion")
		echo "${v%%.*}"
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
	"$image" \
	bash -lc "
		cd /work/build &&
		{ eatmydata cmake /work/leechcraft/src --preset $distro -G Ninja \
			-DCMAKE_C_COMPILER=$cc -DCMAKE_CXX_COMPILER=$cxx $cmake_extra || exit 2; } &&
		{ eatmydata cmake --build . -j$(nproc) -- -k0 || exit 1; }
	"
