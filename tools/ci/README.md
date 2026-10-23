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
cell of a running full build. A cell that fails to configure or to build fails the run; the message says which,
because on a rolling distro a configure failure almost always means a dependency moved. The quick job mails
`NOTIFY` on every failure and on every status change, the full job on status changes only.

## Changing things

- A distro: add `tools/docker/base/<distro>/Dockerfile` and the `<distro>` preset in `src/CMakePresets.json`.
  Presets describe a distro only; the runner picks the compiler. The full build picks the distro up once
  `CI-base-images` has built the image.
- A compiler: add it to `COMPILERS` in the Jenkinsfile, with its floor in `MINIMUM`. Cells appear by themselves
  on the distros whose image ships a new enough version, which `runci.sh <distro> <compiler> version` reports.
- The floors: `MINIMUM` says which compiler versions build the project. They change when the code starts using
  a language feature that older compilers lack, and nowhere else.
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
## Bootstrapping the seed

The one job that is not created by `jobs.groovy` is the seed itself. Create it once, either by placing
`ci-seed.config.xml` at `$JENKINS_HOME/jobs/ci-seed/config.xml` and reloading the configuration from disk, or by
hand in the UI as a Freestyle job restricted to the `real` label, checking out this repository, with a
"Process Job DSLs" step that looks on the filesystem for `tools/ci/jobs.groovy`, the sandbox off and removed
jobs ignored. Its first run needs the script approval described above. From then on `jobs.groovy` maintains it.
