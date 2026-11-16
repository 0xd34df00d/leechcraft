# First-logon provisioning of the LeechCraft Windows build VM: everything needed
# for the build host to take over over SSH. Runs once, from the unattend CD.
$ErrorActionPreference = 'Stop'
$media = Split-Path -Parent $MyInvocation.MyCommand.Path
Start-Transcript -Path C:\lc-bootstrap.log -Append

function Find-OnDrives ([string] $relative)
{
	foreach ($drive in Get-PSDrive -PSProvider FileSystem)
	{
		$candidate = Join-Path $drive.Root $relative
		if (Test-Path $candidate)
		{
			return $candidate
		}
	}
	return $null
}

function Install-Msi ([string] $path)
{
	$proc = Start-Process msiexec.exe -ArgumentList '/i', "`"$path`"", '/qn', '/norestart' -Wait -PassThru
	if ($proc.ExitCode -notin 0, 3010)
	{
		throw "msiexec $path failed: $($proc.ExitCode)"
	}
}

# The guest agent gives the host a way in that does not depend on the network.
$virtioTools = Find-OnDrives 'virtio-win-gt-x64.msi'
if ($virtioTools)
{
	Install-Msi $virtioTools
}
$guestAgent = Find-OnDrives 'guest-agent\qemu-ga-x86_64.msi'
if ($guestAgent)
{
	Install-Msi $guestAgent
}

$sshServer = Get-WindowsCapability -Online -Name 'OpenSSH.Server*'
if ($sshServer.State -ne 'Installed')
{
	Add-WindowsCapability -Online -Name $sshServer.Name | Out-Null
}
Set-Service sshd -StartupType Automatic
# The first start generates the host keys and C:\ProgramData\ssh\sshd_config.
Start-Service sshd

# sshd ignores ~\.ssh\authorized_keys for members of Administrators and only
# accepts this file if nobody but Administrators and SYSTEM can write it.
$adminKeys = 'C:\ProgramData\ssh\administrators_authorized_keys'
Copy-Item (Join-Path $media 'authorized_keys.txt') $adminKeys -Force
icacls.exe $adminKeys /inheritance:r /grant '*S-1-5-32-544:F' /grant '*S-1-5-18:F' | Out-Null

$sshdConfig = 'C:\ProgramData\ssh\sshd_config'
(Get-Content $sshdConfig) -replace '^#?PasswordAuthentication .*', 'PasswordAuthentication no' |
	Set-Content $sshdConfig -Encoding ascii
New-ItemProperty -Path 'HKLM:\SOFTWARE\OpenSSH' -Name DefaultShell `
	-Value "$env:SystemRoot\System32\WindowsPowerShell\v1.0\powershell.exe" -PropertyType String -Force | Out-Null
Restart-Service sshd

# Server 2025 ships this rule for the Private profile only, while QEMU's user network comes up as Public.
if (Get-NetFirewallRule -Name 'OpenSSH-Server-In-TCP' -ErrorAction SilentlyContinue)
{
	Set-NetFirewallRule -Name 'OpenSSH-Server-In-TCP' -Profile Any
}
else
{
	New-NetFirewallRule -Name 'OpenSSH-Server-In-TCP' -DisplayName 'OpenSSH Server (sshd)' `
		-Direction Inbound -Protocol TCP -LocalPort 22 -Action Allow -Profile Any | Out-Null
}

# A build VM must not install updates and reboot on its own in the middle of a build;
# updates are applied by hand between build sessions.
$updatePolicy = 'HKLM:\SOFTWARE\Policies\Microsoft\Windows\WindowsUpdate\AU'
New-Item -Path $updatePolicy -Force | Out-Null
Set-ItemProperty -Path $updatePolicy -Name NoAutoUpdate -Value 1 -Type DWord

# The host stops the VM through the guest agent or ACPI; Server's shutdown reason
# dialog would otherwise block that while the autologon session is open.
$reliability = 'HKLM:\SOFTWARE\Policies\Microsoft\Windows NT\Reliability'
New-Item -Path $reliability -Force | Out-Null
Set-ItemProperty -Path $reliability -Name ShutdownReasonOn -Value 0 -Type DWord

# GUI smoke tests need the autologon console session to stay around, unlocked and lit.
Remove-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon' -Name AutoLogonCount -ErrorAction SilentlyContinue
powercfg.exe /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c
powercfg.exe /change monitor-timeout-ac 0
powercfg.exe /change standby-timeout-ac 0
powercfg.exe /hibernate off
Set-ItemProperty -Path 'HKCU:\Control Panel\Desktop' -Name ScreenSaveActive -Value 0

New-Item -ItemType File -Path C:\lc-bootstrap.done -Force | Out-Null
Stop-Transcript
