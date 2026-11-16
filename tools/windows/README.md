# Windows

LeechCraft for Windows is built with clang-cl against the MSVC ABI: MinGW has no Qt WebEngine, which Azoth and
Poshuku need. The build runs in a Windows Server 2025 VM on a Linux KVM host and is driven over SSH, by hand for
now. The scripts live where they run:

- `win-ssh`, `win-scp`, `win-sync`: on your machine. They reach the guest through the VM host, which forwards the
  guest's sshd to its own `127.0.0.1`. `LC_WIN_HOST` names the VM host as an ssh destination (required),
  `LC_WIN_SSH_PORT` the forwarded port (default 2223), `LC_WIN_SSH_OPTS` adds ssh options for both hops, such as
  `-i <key>`, and `LC_WIN_KNOWN_HOSTS` keeps the guest's host key out of your `known_hosts`. `win-scp` names the
  guest with a `win:` prefix, as in `win-scp win:/C:/lc/logs/build.log .`. `win-sync` mirrors this tree's `src`
  and `tools/windows/guest` into the guest's `C:\lc`, deleting there what is gone here and keeping the mtimes of
  what didn't change, so Ninja rebuilds only what did. Since the guest has no repository for CMake to take the
  version from, `win-sync` also leaves this tree's `git describe` in `C:\lc\version.txt`.
- `vm/`: on the VM host, by the account owning the VM directory, which `LC_WIN_VMDIR` names (required).
  `win-vm.sh` creates, starts and stops the VM, takes screenshots, and runs PowerShell in the guest through the QEMU
  guest agent when SSH is down. `wintool.py` does the QMP, guest agent, ISO and `install.wim` work for it.
- `guest/`: in the guest, as `C:\lc\tools\windows\guest\*` once synced. `provision.ps1` installs the toolchain and
  the dependencies, `build.ps1` builds, `run-lc.cmd` starts the result, `dllcheck.ps1` explains a DLL that won't load.

## Setting up

1. On the VM host: QEMU with KVM, Python 3 with `pycdlib` for `prepare` (point `LC_WIN_PY` at it), and the VM
   directory. Put the Windows Server 2025 evaluation ISO (`SERVER_EVAL_*.iso`) and the
   [virtio-win](https://github.com/virtio-win/virtio-win-pkg-scripts) ISO into its `iso/`, and the public keys that
   may log into the guest into its `authorized_keys`.
2. `vm/win-vm.sh prepare`: creates the disk and an ISO with the answer file, `vm/unattend/bootstrap.ps1` and the
   keys. The Administrator password is generated into `secrets/admin-password`.
3. `vm/win-vm.sh start --install`: Setup runs unattended, and the first logon runs the bootstrap, which installs the
   guest agent, enables sshd for the keys only, opens it in the firewall, turns automatic updates off and keeps
   the autologon session awake. The VM runs at idle priority with 16 vCPUs and 32 GB (`LC_WIN_CPUS`, `LC_WIN_MEM`),
   so it never takes CPU from anything else on the host. `vm/win-vm.sh start` later on, `stop` to shut it down.
4. `win-sync`, then `win-ssh 'C:\lc\tools\windows\guest\provision.ps1'`, quoted, or your shell eats the
   backslashes: the VS Build Tools, LLVM, CMake, Ninja, Git, Qt through aqtinstall, vcpkg with the non-Qt
   dependencies, GStreamer, and QXmpp built from source. Every step leaves a marker in `C:\lc\provisioned` and is
   skipped once it has one, so a failed run is just started again. The transcript goes to `C:\lc\provision.log`.
5. An icon theme in `C:\lc\icons`: Windows has none for LeechCraft to use, so `build.ps1 -Install` copies this one
   into the installation. From a Linux box, for instance:
   `tar -C /usr/share/icons -chf - oxygen | win-ssh 'mkdir -Force C:\lc\icons; tar -xf - -C C:/lc/icons'`.

## Building and running

- `win-sync`, then `win-ssh 'C:\lc\tools\windows\guest\build.ps1 -Install -Test'`: configures the first time with the
  `windows` preset from `src/CMakePresets.json`, which lists the plugins ported so far, builds, installs into
  `C:\lc\inst\<Qt version>` and runs the tests. The logs are in `C:\lc\logs`. `-Targets` builds only those,
  `-Reconfigure` configures again, also taking up a newer `version.txt`, and `-Configure -DX=Y` passes more
  options to that configure.
- A GUI needs the console session, which an SSH login doesn't have, so LeechCraft starts from a scheduled task that
  runs in it: once `schtasks /create /tn lc-run /tr "C:\lc\tools\windows\guest\run-lc.cmd" /sc once /st 00:00 /it /ru Administrator /f`,
  then `schtasks /run /tn lc-run` every time. Its output goes to `C:\lc\logs\lc-run-<Qt version>.log`,
  LeechCraft's own logs to `C:\Users\Administrator\.leechcraft`. `vm/win-vm.sh screenshot <file.png>` shows the
  screen; `vm/win-vm.sh qmp send-key` types into it.
- Another Qt: `provision.ps1 -Qt <version>`, then `build.ps1 -Qt <version>`, and `run-lc.cmd <version>` in the task.
  Everything Qt-specific lives in directories of its own, so versions don't get in each other's way.

## Notes

- Qt is 6.11.3. The official 6.12.0 MSVC WebEngine package was built months before 6.12.0's Qt GUI and imports
  `QAccessibleInterface`'s constructor as a public member, while the released Qt GUI exports it as a protected one.
  MSVC mangles the access into the symbol name, so nothing that links WebEngine loads.
- QXmpp is built once per Qt version, into `C:\lc\deps\<Qt version>`: a library built against one Qt may use symbols an
  older one lacks.
- aqtinstall's last release predates the Qt 6.11 repository layout, so `provision.ps1` pins a commit of its master.
- vcpkg's libtorrent is built with its deprecated API, as distributions do; BitTorrent still uses some of it.
  vcpkg's `libmaxminddb.pc` has relative paths, so `ENABLE_BITTORRENT_GEOIP` is off.
- vcpkg's libtorrent comes with only its CMake package, while BitTorrent finds libtorrent through pkg-config, so
  `provision.ps1` writes a `libtorrent-rasterbar.pc` next to vcpkg's own. The include directory and the import
  library describe libtorrent fully: vcpkg writes the definitions its ABI depends on into the installed
  `config.hpp`. The package's other usage requirements don't matter here: LeechCraft calls no OpenSSL itself,
  and Boost.Asio links the Winsock libraries through `#pragma comment (lib)`. The file describes the release
  build, the one `build.ps1` links. LeechCraft doesn't use the CMake package anywhere: it hands
  `TORRENT_USE_ASSERTS`, which changes libtorrent's ABI, to every Debug consumer, which breaks Debug builds
  against the release libtorrent distributions ship.
- Windows PowerShell 5.1, the guest's SSH shell, turns a redirected stderr line of a native command into an error
  record, which `$ErrorActionPreference = 'Stop'` makes fatal. `build.ps1` runs the tools through `cmd`'s
  redirection instead.
- The executables are GUI subsystem ones: without a console, Qt logs to the debugger. `run-lc.cmd` sets
  `QT_FORCE_STDERR_LOGGING=1` so the log file gets it, and `build.ps1 -Test` sets it so that ctest gets a failing
  test's output.
- Until packaging collects the third-party DLLs, `run-lc.cmd` and the tests find them through `PATH`, in Qt's, vcpkg's,
  GStreamer's and `C:\lc\deps`' `bin` directories, and the tests find LeechCraft's own next to the targets that built them.
- "The specified procedure could not be found" means a DLL imports a symbol its dependency doesn't export.
  `dllcheck.ps1 <dll>` walks the imports the way the loader resolves them and names the missing symbols.
- A crash leaves a full dump in `C:\lc\dumps`. LLVM's `lldb` reads it, finding LeechCraft's PDBs in the build tree,
  but needs a `python3.dll` on `PATH`, such as the one uv installed for aqtinstall:
  `$env:Path = (Split-Path (Get-ChildItem C:\tools\uv-python -Recurse -Filter python3.dll)[0].FullName) + ";$env:Path"`,
  then `lldb -b -c <dump> -o "thread backtrace"`.
- The VM uses SeaBIOS rather than UEFI, since Windows Setup's UEFI loader waits for a key press on a blank disk, and
  an AHCI disk with an e1000e NIC, which need no drivers during Setup. It hides SVM from the guest, so Windows
  doesn't turn on virtualization-based security and run itself under Hyper-V.
- Updates are off in the guest, so that no build gets interrupted by a reboot; install them by hand between
  builds. The evaluation edition expires 180 days after installation.
