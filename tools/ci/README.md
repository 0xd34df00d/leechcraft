# CI

Two files drive every Jenkins job of the project:

- `Jenkinsfile`: the build logic. `PROFILE=quick` builds the primary distro with the primary compiler and, when
  green, triggers the job named by `DOWNSTREAM`. `PROFILE=full` builds every distro under `tools/docker/base`
  with every compiler in `COMPILERS` that the distro's image ships in at least the `MINIMUM` version, one cell
  at a time, and also publishes one warnings result aggregated over all cells. `SKIP_CELLS` leaves cells out;
  the build description lists what was skipped and why.
- `jobs.groovy`: the job shells. Which jobs exist, which profile each runs, its triggers, parameters and
  retention. The `ci-seed` job applies it on every push.

A cell runs `tools/docker/runci.sh <distro> <gcc|clang> [build|tidy]` in the distro's `leechcraft/ci_<distro>`
image: a from-scratch build without network, in a tmpfs, on every core, needing only `sh`, `cmake`, `ninja` and
the compilers inside the image. Every build in every job takes the `lc-build` lock around its compile, so only
one compile runs on the machine at a time and build times stay comparable; a quick build waits for at most one
cell of a running full build. The full profile compiles through ccache, with one cache for all cells at
`/var/cache/ci-ccache` on the agent, a partition dedicated to it that the cache may fill to nine tenths, and each
cell's log ending with that build's cache statistics; the quick profile does not, so that its build time stays a
measurement. An image without ccache builds without it and says so. A cell that fails to configure or to build
fails the run; the message says which, because on a
rolling distro a configure failure almost always means a dependency moved. The quick job mails `NOTIFY` on every
failure and on every status change, the full job on status changes only.

## Changing things

- A distro: add `tools/docker/base/<distro>/Dockerfile` and the `<distro>` preset in `src/CMakePresets.json`.
  Presets describe a distro only; the runner picks the compiler. The full build picks the distro up once
  `CI-base-images` has built the image.
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
- The quick job publishes under the id `gcc` with the gcc parser, as the matrix job it replaced did, so its
  warnings trend continues. For the same reason source paths are rewritten to the workspace's absolute path:
  Warnings NG would otherwise anchor relative paths to the build directory it spots in CMake's output, which
  only exists inside the container.
- The compiler cache is written by the build containers as root, so wiping it is `rm -rf /var/cache/ci-ccache/*`
  as root while no build runs; the next full build refills it.
- `debian_experimental` installs from experimental wherever that suite has a package: `-t experimental` on the
  install step, `clang-24`, and `APT_TARGET_RELEASE=experimental` for the qxmpp stage so that the library is built
  against the same stack. Without `-t`, apt never picks experimental, since the suite is pinned below everything.
  The dist-upgrade stays plain so that apt chooses the experimental versions with the whole package set in view;
  a package that cannot follow makes the image build fail, which is the cue for a `package/unstable` exception.

## qxmpp in the Debian and Fedora images

Debian unstable and Fedora rawhide ship a qxmpp older than Xoox needs, so their images build their own from
`tools/packaging/qxmpp`: Debian's and Fedora's own packaging of qxmpp, bumped. The first stage of each of those
Dockerfiles runs `build.sh`, which produces .deb or .rpm files; the final stage installs them in place of the
distro's development package, from a mount of the first stage's output, so nothing of the packaging lands in
the image. The package manager thus knows about the library, and a LeechCraft package built in such an image
depends on exactly these packages, which a repository of ours can ship next to it one day. The debug symbol
packages are built as well and kept in a `debug/` subdirectory, for that repository rather than for the image;
`docker build --target packages --output type=local,dest=<dir>` exports everything the stage produced.

`build.sh` fails on purpose once the distro's own qxmpp satisfies the floor it holds: the image build goes red and
mails, and the fix is to delete the qxmpp stage from that Dockerfile and reinstate the distro's development
package. Until then the previously built image keeps serving the builds.

Bumping qxmpp: a new `debian/changelog` entry (version `X.Y.Z-0lcN`; the revision sorts below any Debian upload
of the same version), `Version`, `Release` and `%changelog` in `qxmpp.spec`, and, since qxmpp bumps its soname on
every minor release, `%global sover` and the `N` in the names of the `libqxmppqt6-N` and `libqxmppomemoqt6-N`
packages across `debian/control` and the install and lintian-overrides files. The floor in `build.sh` mirrors the
Xoox CMakeLists.

The Dockerfiles copy the packaging from the build context, so `CI-base-images` builds with `tools` as the context
and `tools/.dockerignore` limits what gets sent to `docker` and `packaging`. Its shell step:

    cd tools
    docker build --pull --no-cache --network=host -f docker/base/${distro}/Dockerfile -t leechcraft/ci_${distro}:latest .
    echo "$REGISTRY_PASSWORD" | docker login -u "$REGISTRY_USER" --password-stdin
    docker push leechcraft/ci_${distro}:latest
    docker image prune -f
    docker builder prune -f

`--pull` refreshes the base image; the two `prune`s drop what the previous build left untagged and the build
cache that even a `--no-cache` build writes.

## Bootstrapping the seed

The one job that is not created by `jobs.groovy` is the seed itself. Create it once, either by placing
`ci-seed.config.xml` at `$JENKINS_HOME/jobs/ci-seed/config.xml` and reloading the configuration from disk, or by
hand in the UI as a Freestyle job restricted to the `real` label, checking out this repository, with a
"Process Job DSLs" step that looks on the filesystem for `tools/ci/jobs.groovy`, the sandbox off and removed
jobs ignored. Its first run needs the script approval described above. From then on `jobs.groovy` maintains it.
