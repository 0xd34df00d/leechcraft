# Toolchain for building LeechCraft with clang-cl: MSVC STL/SDK from the VS Build Tools,
# LLVM, the official Qt binaries, vcpkg for the non-Qt dependencies. Idempotent; each
# step is skipped once its marker exists, so a failed run is simply started again.
# Another -Qt installs that Qt and builds the Qt-linked dependencies against it, next to
# the ones for the other Qt versions.
param (
	[string] $Qt = '6.11.3',
	[string[]] $Steps = @('defender', 'crashdumps', 'git', 'cmake', 'ninja', 'uv', 'llvm', 'buildtools', 'qt', 'vcpkg', 'vcpkgdeps', 'gstreamer', 'qxmpp')
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

$root = 'C:\lc'
$downloads = "$root\downloads"
$markers = "$root\provisioned"
$tools = 'C:\tools'

$versions = @{
	Git = '2.56.0.4'
	CMake = '4.4.5'
	Ninja = '1.13.2'
	Llvm = '23.1.4'
	GStreamer = '1.28.8'
	QXmpp = '1.17.0'
	# aqtinstall 3.3.0 predates the Qt 6.11 repository layout; master handles it.
	AqtInstall = '076e1659807d0b362a3ed684d54c2e9c775eb9c7'
}
$qtModules = 'qt5compat', 'qtimageformats', 'qtnetworkauth', 'qtscxml', 'qtwebengine', 'qtwebchannel', 'qtpositioning'
# Qt itself and what links to it exist once per Qt version, the latter in C:\lc\deps\<version>:
# a library built against one Qt may need symbols an older Qt lacks.
$perQtSteps = 'qt', 'qxmpp'
# libtorrent with the deprecated API, like distro builds (TORRENT_ABI_VERSION 2): BitTorrent still uses some of it.
$vcpkgPorts = 'boost-graph', 'boost-preprocessor', 'boost-spirit', 'boost-phoenix', 'boost-fusion', 'boost-serialization',
	'boost-asio', 'boost-container', 'boost-circular-buffer', 'boost-bimap', 'boost-variant', 'boost-function',
	'pkgconf', 'libtorrent[core,deprfun,webtorrent]', 'libmaxminddb', 'taglib', 'hunspell'

New-Item -ItemType Directory -Force $root, $downloads, $markers, $tools | Out-Null

function Get-Download ([string] $url, [string] $name)
{
	$path = Join-Path $downloads $name
	if (-not (Test-Path $path))
	{
		curl.exe -fsSL --retry 3 -o "$path.part" $url
		if ($LASTEXITCODE -ne 0)
		{
			throw "download of $url failed: $LASTEXITCODE"
		}
		Move-Item "$path.part" $path
	}
	return $path
}

function Invoke-Checked ([string] $exe, [string[]] $arguments, [int[]] $success = @(0))
{
	$proc = Start-Process $exe -ArgumentList $arguments -Wait -PassThru -NoNewWindow
	if ($success -notcontains $proc.ExitCode)
	{
		throw "$exe $arguments exited with $($proc.ExitCode)"
	}
}

function Add-MachinePath ([string] $dir)
{
	$path = [Environment]::GetEnvironmentVariable('Path', 'Machine')
	if (($path -split ';') -notcontains $dir)
	{
		[Environment]::SetEnvironmentVariable('Path', "$path;$dir", 'Machine')
	}
	if (($env:Path -split ';') -notcontains $dir)
	{
		$env:Path = "$env:Path;$dir"
	}
}

function Install-Msi ([string] $path, [string[]] $properties = @())
{
	Invoke-Checked msiexec.exe (@('/i', "`"$path`"", '/qn', '/norestart') + $properties) @(0, 3010)
}

function Import-VcVars
{
	cmd /c '"C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat" > nul && set' | ForEach-Object {
		if ($_ -match '^([^=]+)=(.*)$')
		{
			[Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], 'Process')
		}
	}
}

$actions = [ordered] @{
	defender = {
		Add-MpPreference -ExclusionPath $root, 'C:\Qt', 'C:\vcpkg', 'C:\BuildTools', 'C:\Program Files\LLVM', $tools
		Add-MpPreference -ExclusionProcess 'clang-cl.exe', 'clang.exe', 'clang++.exe', 'lld-link.exe', 'cl.exe', 'link.exe', 'lib.exe',
			'ninja.exe', 'cmake.exe', 'moc.exe', 'rcc.exe', 'uic.exe', 'qmlcachegen.exe', 'qmltyperegistrar.exe', 'vcpkg.exe', 'git.exe'
	}
	crashdumps = {
		# Windows Error Reporting keeps a full dump of every LeechCraft crash.
		$key = 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\leechcraft-qt6.exe'
		New-Item -Path $key -Force | Out-Null
		Set-ItemProperty -Path $key -Name DumpFolder -Value "$root\dumps" -Type ExpandString
		Set-ItemProperty -Path $key -Name DumpType -Value 2 -Type DWord
		Set-ItemProperty -Path $key -Name DumpCount -Value 5 -Type DWord
	}
	git = {
		$installer = Get-Download "https://github.com/git-for-windows/git/releases/download/v$($versions.Git -replace '\.(\d+)$', '.windows.$1')/Git-$($versions.Git)-64-bit.exe" "Git-$($versions.Git)-64-bit.exe"
		Invoke-Checked $installer '/VERYSILENT', '/NORESTART', '/NOCANCEL', '/SP-', '/SUPPRESSMSGBOXES', '/o:PathOption=Cmd', '/o:CRLFOption=CRLFCommitAsIs'
		Add-MachinePath 'C:\Program Files\Git\cmd'
		git config --system core.longpaths true
		if ($LASTEXITCODE -ne 0)
		{
			throw "git config failed: $LASTEXITCODE"
		}
	}
	cmake = {
		Install-Msi (Get-Download "https://github.com/Kitware/CMake/releases/download/v$($versions.CMake)/cmake-$($versions.CMake)-windows-x86_64.msi" "cmake-$($versions.CMake).msi") 'ADD_CMAKE_TO_PATH=System'
		Add-MachinePath 'C:\Program Files\CMake\bin'
	}
	ninja = {
		$zip = Get-Download "https://github.com/ninja-build/ninja/releases/download/v$($versions.Ninja)/ninja-win.zip" "ninja-$($versions.Ninja).zip"
		Expand-Archive $zip "$tools\ninja" -Force
		Add-MachinePath "$tools\ninja"
	}
	uv = {
		$zip = Get-Download 'https://github.com/astral-sh/uv/releases/latest/download/uv-x86_64-pc-windows-msvc.zip' 'uv.zip'
		Expand-Archive $zip "$tools\uv" -Force
		Add-MachinePath "$tools\uv"
	}
	llvm = {
		Install-Msi (Get-Download "https://github.com/llvm/llvm-project/releases/download/llvmorg-$($versions.Llvm)/LLVM-$($versions.Llvm)-win64.msi" "LLVM-$($versions.Llvm)-win64.msi")
		Add-MachinePath 'C:\Program Files\LLVM\bin'
	}
	buildtools = {
		$bootstrapper = Get-Download 'https://aka.ms/vs/18/stable/vs_buildtools.exe' 'vs_buildtools.exe'
		Invoke-Checked $bootstrapper '--quiet', '--wait', '--norestart', '--nocache', '--installPath', 'C:\BuildTools',
			'--add', 'Microsoft.VisualStudio.Workload.VCTools', '--includeRecommended' @(0, 3010)
	}
	qt = {
		$env:UV_TOOL_DIR = "$tools\uv-tools"
		$env:UV_PYTHON_INSTALL_DIR = "$tools\uv-python"
		$env:UV_CACHE_DIR = "$downloads\uv-cache"
		& "$tools\uv\uvx.exe" --from "aqtinstall @ git+https://github.com/miurahr/aqtinstall@$($versions.AqtInstall)" `
			aqt install-qt windows desktop $Qt win64_msvc2022_64 --outputdir C:\Qt -m @qtModules
		if ($LASTEXITCODE -ne 0)
		{
			throw "aqt failed: $LASTEXITCODE"
		}
	}
	vcpkg = {
		if (-not (Test-Path 'C:\vcpkg\.git'))
		{
			git clone https://github.com/microsoft/vcpkg C:\vcpkg
			if ($LASTEXITCODE -ne 0)
			{
				throw "vcpkg clone failed: $LASTEXITCODE"
			}
		}
		Invoke-Checked 'C:\vcpkg\bootstrap-vcpkg.bat' '-disableMetrics'
		[Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\vcpkg', 'Machine')
		# Inside the Defender-excluded tree and shared by whichever account runs vcpkg.
		New-Item -ItemType Directory -Force "$root\vcpkg-cache" | Out-Null
		[Environment]::SetEnvironmentVariable('VCPKG_DEFAULT_BINARY_CACHE', "$root\vcpkg-cache", 'Machine')
	}
	vcpkgdeps = {
		$env:VCPKG_DEFAULT_BINARY_CACHE = "$root\vcpkg-cache"
		& C:\vcpkg\vcpkg.exe install --triplet x64-windows --clean-after-build --recurse @vcpkgPorts
		if ($LASTEXITCODE -ne 0)
		{
			throw "vcpkg install failed: $LASTEXITCODE"
		}

		$installed = 'C:\vcpkg\installed\x64-windows'
		$config = Get-Content -Raw "$installed\include\libtorrent\config.hpp"
		if ($config -notmatch '#define TORRENT_LINKING_SHARED' -or $config -notmatch '#define TORRENT_ABI_VERSION')
		{
			throw "vcpkg's libtorrent/config.hpp no longer carries the ABI definitions libtorrent-rasterbar.pc relies on"
		}
		$version = [regex]::Match((Get-Content -Raw "$installed\include\libtorrent\version.hpp"),
			'#define LIBTORRENT_VERSION "([^"]+)"').Groups[1].Value
		$pc = @"
prefix=`${pcfiledir}/../..
libdir=`${prefix}/lib
includedir=`${prefix}/include

Name: libtorrent-rasterbar
Description: libtorrent as vcpkg builds it
Version: $version
Libs: -L`${libdir} -ltorrent-rasterbar
Cflags: -I`${includedir}
"@
		[IO.File]::WriteAllText("$installed\lib\pkgconfig\libtorrent-rasterbar.pc", ($pc -replace "`r", ''))
	}
	gstreamer = {
		$name = "gstreamer-1.0-msvc-x86_64-$($versions.GStreamer).exe"
		$url = "https://gstreamer.freedesktop.org/data/pkg/windows/$($versions.GStreamer)/msvc/$name"
		$installer = Get-Download $url $name
		$expected = ((Get-Content (Get-Download "$url.sha256sum" "$name.sha256sum")) -split '\s+')[0]
		if ((Get-FileHash $installer -Algorithm SHA256).Hash -ne $expected)
		{
			Remove-Item $installer
			throw "checksum mismatch for $name"
		}
		Invoke-Checked $installer '/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/TYPE=full', "/LOG=$root\gstreamer-install.log"
	}
	qxmpp = {
		$archive = Get-Download "https://invent.kde.org/libraries/qxmpp/-/archive/v$($versions.QXmpp)/qxmpp-v$($versions.QXmpp).tar.gz" "qxmpp-v$($versions.QXmpp).tar.gz"
		New-Item -ItemType Directory -Force "$root\deps-src" | Out-Null
		tar -xf $archive -C "$root\deps-src"
		Import-VcVars
		$build = "$root\deps-build\$Qt\qxmpp"
		cmake -S "$root\deps-src\qxmpp-v$($versions.QXmpp)" -B $build -G Ninja `
			-DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER_TYPE=LLD `
			-DCMAKE_PREFIX_PATH=C:/Qt/$Qt/msvc2022_64 -DCMAKE_INSTALL_PREFIX=C:/lc/deps/$Qt `
			-DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOCUMENTATION=OFF -DBUILD_OMEMO=OFF -DWITH_GSTREAMER=OFF
		if ($LASTEXITCODE -ne 0)
		{
			throw "qxmpp configure failed: $LASTEXITCODE"
		}
		cmake --build $build
		if ($LASTEXITCODE -ne 0)
		{
			throw "qxmpp build failed: $LASTEXITCODE"
		}
		cmake --install $build
		if ($LASTEXITCODE -ne 0)
		{
			throw "qxmpp install failed: $LASTEXITCODE"
		}
	}
}

Start-Transcript -Path "$root\provision.log" -Append
try
{
	foreach ($step in $Steps)
	{
		$marker = Join-Path $markers $(if ($perQtSteps -contains $step) { "$step-$Qt" } else { $step })
		if (Test-Path $marker)
		{
			Write-Output "== ${step}: already done"
			continue
		}
		Write-Output "== ${step}: starting at $(Get-Date -Format s)"
		& $actions[$step]
		Set-Content $marker (Get-Date -Format s)
		Write-Output "== ${step}: done at $(Get-Date -Format s)"
	}
	Write-Output '== all steps done'
}
catch
{
	Write-Output "== FAILED: $_"
	throw
}
finally
{
	Stop-Transcript
}
