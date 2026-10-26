# CI

Three files drive every Jenkins job of the project:

- `Jenkinsfile`: the build logic. `PROFILE=quick` builds the primary distro with the primary compiler and, when
  green, triggers the job named by `DOWNSTREAM`. `PROFILE=full` builds every distro under `tools/docker/base`
  with every compiler in `COMPILERS` that the distro's image ships in at least the `MINIMUM` version, one cell
  at a time, and also publishes, per compiler, one warnings result over all its cells with repeats dropped, and one
  badge per cell.
  `SKIP_CELLS` leaves cells out; the build description lists what was skipped and why.
- `images.Jenkinsfile`: the image rebuilds. Every distro under `tools/docker/base`, or those in `DISTROS`, through
  `tools/docker/buildimage.sh`; an image whose packages changed is pushed to the registry and, when any changed,
  `DOWNSTREAM` runs against the new set.
- `jobs.groovy`: the job shells. Which jobs exist, which script and profile each runs, its triggers, parameters
  and retention. The `ci-seed` job applies it on every push.

A cell runs `tools/docker/runci.sh <distro> <gcc|clang> [build|tidy]` in the distro's `leechcraft/ci_<distro>`
image: a from-scratch build without network, in a tmpfs, on every core, needing only `sh`, `cmake`, `ninja` and
the compilers inside the image; with `mold` in the image the cell links through it, without it through the distro's
default linker, and says so. An uncached build runs moc one file at a time per target (`CMAKE_AUTOGEN_PARALLEL=1`),
since ninja already fills every core with the other steps; a cached build, whose compiles are cache hits, runs four at
a time, since the autogen steps of the largest targets are otherwise the longest thing in it. Every build in every job takes the `lc-build` lock around its compile, so only
one compile runs on the machine at a time and build times stay comparable; a quick build waits for at most one
cell of a running full build, or for one round of image rebuilds. The full profile compiles through ccache, with one cache for all cells at
`/var/cache/ci-ccache` on the agent, a partition dedicated to it that the cache may fill to nine tenths, and each
cell's log ending with that build's cache statistics; the quick profile does not, so that its build time stays a
measurement. An image without ccache builds without it and says so. A cell that fails to configure or to build
fails the run; the message says which, because on a
rolling distro a configure failure almost always means a dependency moved. The quick job mails `NOTIFY` on every
failure and on every status change, the full job on status changes only.

## Changing things

- A distro: add `tools/docker/base/<distro>/Dockerfile` and the `<distro>` preset in `src/CMakePresets.json`.
  Presets describe a distro only; the runner picks the compiler. The push triggers the image job for the new
  directory; the full build picks the distro up once it has built the image. A package manager other than dpkg,
  rpm, pacman, apk or Portage needs a branch in `buildimage.sh`; until then the image job stays unstable and
  treats the image as changed.
- A compiler: add it to `COMPILERS` in the Jenkinsfile, with its floor in `MINIMUM`. Cells appear by themselves
  on the distros whose image ships a new enough version, which `runci.sh <distro> <compiler> version` reports.
- The floors: `MINIMUM` says which compiler versions build the project. They change when the code starts using
  a language feature that older compilers lack, and nowhere else. The gcc floor doubles as the floor for the
  libstdc++ a clang cell uses, which `runci.sh <distro> clang stdlib` reports: clang brings no standard library
  of its own, so a distro whose gcc is too old loses its clang cell as well.
- clang-tidy: add a cell with `tidy: true` to the full profile. The images need `clang-tidy`; the rules come
  from the `.clang-tidy` file in the repository.
- Leaving a cell out: for one run, set `SKIP_CELLS` in Build with Parameters; for good, set the `skip` default
  of the job in `jobs.groovy`.
- A job or a trigger: edit `jobs.groovy`. Job DSL scripts loaded from SCM run unsandboxed, so every new version
  needs a one-time approval under Manage Jenkins, In-process Script Approval, unless script security for Job DSL
  is switched off in Configure Global Security.

## Notes

- Every run first picks its reference build with `discoverReferenceBuild`, the last build that was at least
  unstable; Warnings NG computes new and fixed issues only against such a reference.
- The quick job carries a quality gate: a warning its reference build did not have makes the build unstable, which
  still triggers the full job and mails once. The gate compares with that reference, the last build that was at least
  unstable, not with the last build that passed the gate (`ignoreQualityGate`), so a flagged warning is part of the
  baseline on the next build: one unstable build per push that adds warnings, and no weeks of yellow while a
  deprecation wave is worked through. Drop `ignoreQualityGate` for the sticky variant, or set the gate's
  `criticality` to `FAILURE` for red. The full job's results carry no gate.
- The quick job publishes under the id `gcc` with the gcc parser, as the matrix job it replaced did, so its
  warnings trend continues. For the same reason source paths are rewritten to the workspace's absolute path:
  Warnings NG would otherwise anchor relative paths to the build directory it spots in CMake's output, which
  only exists inside the container.
- The aggregated results concatenate the cells' logs per compiler and parse them once: Warnings NG drops an issue it
  already has only when the tool id matches too, so merging the per-cell results would count every repeat. The
  aggregate therefore has one result for gcc and one for clang, each with every distinct warning once.
- Every result is published under its cell's name (`all-gcc` and `all-clang` for the aggregates), both as the id
  in its URL and as the display name: Warnings NG would otherwise label each one after its parser, so the sidebar
  and the job's aggregated trend could not tell the cells apart.
- Badges: the full profile registers one badge configuration per cell, planned or skipped, under the cell's name,
  so `https://jenkins.leechcraft.org/job/leechcraft-full/badge/icon?build=lastCompleted&config=<cell>` shows that one
  cell: passing or failing, the reason for a skipped cell, pending for a cell the run never got to. `build=lastCompleted`
  matters, since the job's plain badge URL reads the running build, whose cells are still pending. The project README
  embeds them in a table; a new distro or compiler gets its row or column there by hand.
- The compiler cache is written by the build containers as root, so wiping it is `rm -rf /var/cache/ci-ccache/*`
  as root while no build runs; the next full build refills it.
- Every cell archives ninja's log of its build as `ninja-logs/<cell>.ninja_log`: when every step started and ended.
  `tools/ci/ninjalog.py <log>` reports the longest steps, the time by kind of step, how many steps ran at a time over
  the build and what was still running at the end; `--trace` writes a Chrome trace for ui.perfetto.dev. Read it before
  touching the build's parallelism: a cell that ends on one long step was waiting for that step, not working.
- `debian_experimental` installs from experimental wherever that suite has a package: `-t experimental` on the
  install step, `clang-24`, and `APT_TARGET_RELEASE=experimental` for the qxmpp stage so that the library is built
  against the same stack. Without `-t`, apt never picks experimental, since the suite is pinned below everything.
  The dist-upgrade stays plain so that apt chooses the experimental versions with the whole package set in view;
  a package that cannot follow makes the image build fail, which is the cue for a `package/unstable` exception.

## qxmpp in the Debian and Fedora images

Debian unstable and Fedora rawhide ship a qxmpp older than Xoox needs, so their images build their own. The
packaging lives in its own repository, [leechcraft-packaging](https://github.com/0xd34df00d/leechcraft-packaging):
Debian's and Fedora's own packaging of qxmpp, bumped, and a `build.sh` that builds a package of that repository
in the running distro's format. The first stage of each of those Dockerfiles fetches the repository at the commit
`tools/packaging.ref` names and runs `build.sh qxmpp`, which produces .deb or .rpm files; the final stage installs
them in place of the distro's development package, from a mount of the first stage's output, so nothing of the
packaging lands in the image. The package manager thus knows about the library, and a LeechCraft package built in
such an image depends on exactly these packages, which a repository of ours can ship next to it one day. The
debug symbol packages are built as well and kept in a `debug/` subdirectory, for that repository rather than for
the image; `docker build --target packages --output type=local,dest=<dir>` exports everything the stage produced.

`build.sh` fails on purpose once the distro's own qxmpp satisfies the floor the packaging holds: the image build
goes red and mails, and the fix is to delete the qxmpp stage from that Dockerfile and reinstate the distro's
development package. Until then the previously built image keeps serving the builds.

Bumping qxmpp happens in the packaging repository, as its README describes; then `tools/packaging.ref` moves to
the new commit, which rebuilds every image. The pin is a commit hash rather than a branch, so a daily rebuild
changes with the distro's packages only, never with the packaging; `buildimage.sh` refuses anything but a full
hash. The images are built with the distro's directory as the context, so a Dockerfile sees nothing of the
repository besides itself, and the only thing it fetches on its own is that commit.

## The image job

`leechcraft-images` runs `tools/docker/buildimage.sh <distro> <state dir>` for every distro under `tools/docker/base`
(or those in `DISTROS`), daily at 4 am US Central time and whenever the quick job sees a push touch
`tools/docker/base/<distro>`, `tools/packaging.ref` or the script itself; such a push rebuilds the touched distros
only, or every image when the pin or the script moved, and triggers the full build afterwards in any case
(`ALWAYS_TRIGGER`). The script rebuilds the image from scratch
(`--pull --no-cache`), lists the packages of the previous image before and of the new one after, and writes the list,
the diff, the versions of gcc, clang, Qt and Boost, as `old → new` for one the rebuild moved, and a verdict, `changed` or `unchanged`, into the state directory, which the job archives;
the Debian and Fedora images also get their `packages` stage exported there, of which everything but the debug symbol
packages is archived. The diff is the first thing to read when
a cell breaks after a rebuild. The distros build concurrently under one `lc-build` lock, so no compile shares the
machine with an image build, yet the lock is held for the longest image build rather than for all of them in turn;
a distro whose build fails keeps its previous image and fails the run without stopping the others. The packages are listed through dpkg, rpm, pacman, apk or the Portage database; an image with
none of them gets the verdict `unknown`, which counts as changed for the push and the trigger and keeps the run
unstable until `buildimage.sh` has learnt the package manager, so a new distro never hides behind `unchanged`.
An image whose packages changed is pushed to the registry (`PUSH`, with the credentials named by
`REGISTRY_CREDENTIALS`) as soon as it is built, while the others still build: a push is a minute of single-core
compression per image, so pushing them in a row after the last build cost as much again as the builds. The login of the
run lives in a docker config directory of the workspace and goes away with the run. When any image changed, or
`ALWAYS_TRIGGER` is set, `DOWNSTREAM` runs as soon as the last image is done. The run ends by pruning
what the rebuilds left untagged and the build cache that even a `--no-cache` build writes, and its description lists
every distro's verdict and toolchain versions. Mail goes to `NOTIFY` on a change of status only.

The matrix job `CI-base-images` did the same with a shell step edited in the UI; once the pipeline has had a green
run, disable or delete it, or both will rebuild every day.

## Bootstrapping the seed

The one job that is not created by `jobs.groovy` is the seed itself. Create it once, either by placing
`ci-seed.config.xml` at `$JENKINS_HOME/jobs/ci-seed/config.xml` and reloading the configuration from disk, or by
hand in the UI as a Freestyle job restricted to the `real` label, checking out this repository, with a
"Process Job DSLs" step that looks on the filesystem for `tools/ci/jobs.groovy`, the sandbox off and removed
jobs ignored. Its first run needs the script approval described above. From then on `jobs.groovy` maintains it.
