# Configure (once) and build LeechCraft in the Windows VM: the windows preset's plugins, built by
# clang-cl against the MSVC STL, the official Qt binaries, vcpkg (classic mode, x64-windows) for the
# other dependencies. -Install installs into $Prefix along with the icon themes in C:\lc\icons,
# -Test runs the tests afterwards.
# Output goes to $Logs\{configure,build,install,test}.log.
param (
	[string] $Qt = '6.11.3',
	[string] $BuildDir = "C:\lc\b\$Qt",
	[string] $Prefix = "C:\lc\inst\$Qt",
	[string] $Logs = 'C:\lc\logs',
	[string[]] $Configure = @(),
	[string[]] $Targets = @(),
	[switch] $Reconfigure,
	[switch] $Install,
	[switch] $Test
)
# Windows PowerShell turns redirected stderr lines of native commands into error records,
# which 'Stop' would make terminating: the native tools run through cmd's redirection instead.
$ErrorActionPreference = 'Continue'
New-Item -ItemType Directory -Force $Logs | Out-Null

# clang-cl and lld-link find the MSVC STL, the CRT and the Windows SDK through the
# environment vcvars sets up.
$vcvars = 'C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
cmd /c "`"$vcvars`" > nul && set" | ForEach-Object {
	if ($_ -match '^([^=]+)=(.*)$')
	{
		[Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
	}
}

function Invoke-Logged ([string] $log, [string] $commandLine)
{
	cmd /c "$commandLine > `"$log`" 2>&1"
	return $LASTEXITCODE
}

# Qt, the dependencies provision.ps1 built against it, and GStreamer's own installation,
# whose pkg-config files pkg_check_modules finds through these prefixes.
$qtDir = "C:\Qt\$Qt\msvc2022_64"
$deps = "C:\lc\deps\$Qt"
$prefixes = @($qtDir, $deps)
$gstreamer = [Environment]::GetEnvironmentVariable('GSTREAMER_1_0_ROOT_MSVC_X86_64', 'Machine')
if ($gstreamer)
{
	$prefixes += $gstreamer.TrimEnd('\')
}
$thirdPartyDlls = @($prefixes | ForEach-Object { "$_\bin" }) + 'C:\vcpkg\installed\x64-windows\bin'

if ($Reconfigure -or -not (Test-Path "$BuildDir\CMakeCache.txt"))
{
	$arguments = @(
		'-S', 'C:\lc\src', '-B', $BuildDir, '--preset', 'windows', '-G', 'Ninja',
		'-DCMAKE_BUILD_TYPE=RelWithDebInfo',
		'-DCMAKE_C_COMPILER=clang-cl', '-DCMAKE_CXX_COMPILER=clang-cl', '-DCMAKE_LINKER_TYPE=LLD',
		'-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake', '-DVCPKG_TARGET_TRIPLET=x64-windows',
		'-DPKG_CONFIG_EXECUTABLE=C:/vcpkg/installed/x64-windows/tools/pkgconf/pkgconf.exe',
		"`"-DCMAKE_PREFIX_PATH=$(($prefixes -replace '\\', '/') -join ';')`""
	)
	# CMake would ask git, but the guest has no repository: win-sync leaves the version here.
	if (Test-Path C:\lc\version.txt)
	{
		$arguments += "-DLEECHCRAFT_VERSION=$(Get-Content C:\lc\version.txt)"
	}
	$arguments += $Configure
	$code = Invoke-Logged "$Logs\configure.log" ("cmake " + ($arguments -join ' '))
	if ($code -ne 0)
	{
		"configure failed: $code"
		exit $code
	}
}

$code = Invoke-Logged "$Logs\build.log" ("ninja -C $BuildDir -k 0 " + ($Targets -join ' '))
"build exit code: $code"
if ($code -ne 0)
{
	exit $code
}

if ($Install)
{
	$code = Invoke-Logged "$Logs\install.log" "cmake --install $BuildDir --prefix $Prefix"
	"install exit code: $code"
	if ($code -ne 0)
	{
		exit $code
	}
	# Windows has no icon theme to offer, so the installation brings its own.
	if (Test-Path C:\lc\icons)
	{
		robocopy.exe C:\lc\icons "$Prefix\icons" /E /NFL /NDL /NJH /NJS /NP | Out-Null
	}
}

if ($Test)
{
	# Nothing collects the DLLs in the build tree: the tests find LeechCraft's next to the targets
	# that built them, the third-party ones in their install locations.
	$dllDirs = Get-ChildItem $BuildDir -Recurse -Filter 'leechcraft-*.dll' | ForEach-Object DirectoryName | Sort-Object -Unique
	$env:Path = (@($dllDirs) + $thirdPartyDlls + $env:Path) -join ';'
	$env:QT_QPA_PLATFORM = 'offscreen'
	$env:QT_FORCE_STDERR_LOGGING = '1'
	$code = Invoke-Logged "$Logs\test.log" "ctest --test-dir $BuildDir -j $env:NUMBER_OF_PROCESSORS --timeout 300 --output-on-failure"
	"test exit code: $code"
	exit $code
}
exit 0
