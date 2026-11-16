# Why does LoadLibrary fail with "The specified procedure could not be found"? Walks a DLL's
# import graph the way the loader resolves it (application directory, system directories, PATH)
# and lists imported symbols that the DLL actually picked up doesn't export. The default PATH is
# the one run-lc.cmd starts LeechCraft with.
param (
	[Parameter(Mandatory)] [string] $Dll,
	[string] $Qt = '6.11.3',
	[string] $AppDir = "C:\lc\inst\$Qt",
	[string[]] $Path = @("C:\Qt\$Qt\msvc2022_64\bin", 'C:\vcpkg\installed\x64-windows\bin', "C:\lc\deps\$Qt\bin",
		"$([Environment]::GetEnvironmentVariable('GSTREAMER_1_0_ROOT_MSVC_X86_64', 'Machine'))\bin") + ($env:Path -split ';')
)
$ErrorActionPreference = 'Stop'
$readobj = 'C:\Program Files\LLVM\bin\llvm-readobj.exe'
$searchDirs = @($AppDir, "$env:SystemRoot\System32", $env:SystemRoot) + ($Path | Where-Object { $_ })

$exportsCache = @{}
function Get-Exports ([string] $file)
{
	if (-not $exportsCache.ContainsKey($file))
	{
		$names = [System.Collections.Generic.HashSet[string]]::new()
		foreach ($line in & $readobj --coff-exports $file)
		{
			if ($line -match '^\s+Name: (.+)$')
			{
				[void] $names.Add($Matches[1])
			}
		}
		$exportsCache[$file] = $names
	}
	return $exportsCache[$file]
}

function Get-Imports ([string] $file)
{
	$imports = @{}
	$current = $null
	$delayed = $false
	foreach ($line in & $readobj --coff-imports $file)
	{
		# Delay-loaded imports are resolved by their first call, not by LoadLibrary.
		if ($line -match '^(Import|DelayImport) \{')
		{
			$delayed = $Matches[1] -eq 'DelayImport'
			$current = $null
		}
		elseif (-not $delayed -and $line -match '^\s+Name: (.+)$')
		{
			$current = $Matches[1]
			$imports[$current] = [System.Collections.Generic.List[string]]::new()
		}
		elseif ($current -and $line -match '^\s+Symbol: (.+) \(\d+\)$')
		{
			$imports[$current].Add($Matches[1])
		}
	}
	return $imports
}

function Resolve-Dll ([string] $name)
{
	foreach ($dir in $searchDirs)
	{
		$candidate = Join-Path $dir $name
		if (Test-Path $candidate)
		{
			return (Resolve-Path $candidate).Path
		}
	}
	return $null
}

$visited = @{}
$queue = [System.Collections.Generic.Queue[string]]::new()
$queue.Enqueue((Resolve-Path $Dll).Path)
$problems = 0
while ($queue.Count)
{
	$file = $queue.Dequeue()
	if ($visited.ContainsKey($file))
	{
		continue
	}
	$visited[$file] = $true
	$imports = Get-Imports $file
	foreach ($dllName in $imports.Keys)
	{
		# API sets are virtual and resolved by the loader itself.
		if ($dllName -match '^(api|ext)-ms-')
		{
			continue
		}
		$resolved = Resolve-Dll $dllName
		if (-not $resolved)
		{
			"MISSING DLL  $dllName  (needed by $file)"
			$problems++
			continue
		}
		$exports = Get-Exports $resolved
		$missing = @($imports[$dllName] | Where-Object { -not $exports.Contains($_) })
		if ($missing.Count)
		{
			"MISSING SYMBOLS in $resolved  (needed by $file):"
			$missing | Select-Object -First 8 | ForEach-Object { "    $_" }
			$problems++
		}
		if ($resolved -notmatch '\\Windows\\')
		{
			$queue.Enqueue($resolved)
		}
	}
}
"checked $($visited.Count) DLLs, $problems problem(s)"
