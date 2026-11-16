#!/bin/sh
# The LeechCraft Windows build VM, run on the VM host by the account owning $LC_WIN_VMDIR.
#   win-vm.sh prepare            create the disk and the unattend ISO (first install only)
#   win-vm.sh start [--install]  boot it; --install also attaches the Windows and unattend media
#   win-vm.sh stop | status | screenshot <out.png> | qmp <cmd> [json] | qga <cmd> [json]
#   win-vm.sh exec [script|-]   run PowerShell in the guest through the agent (works without the network)
set -eu

newest () { ls "$@" 2> /dev/null | sort -V | tail -n 1; }

VMDIR=${LC_WIN_VMDIR:?the VM directory}
TOOLS=$(dirname "$(readlink -f "$0")")
# prepare needs pycdlib in it, the other commands only the standard library.
PY=${LC_WIN_PY:-python3}
RUN=$VMDIR/run
DISK=$VMDIR/disk/server2025.qcow2
DISK_SIZE=${LC_WIN_DISK_SIZE:-150G}
WIN_ISO=${LC_WIN_ISO:-$(newest "$VMDIR"/iso/SERVER_EVAL_*.iso)}
VIRTIO_ISO=${LC_WIN_VIRTIO_ISO:-$(newest "$VMDIR"/iso/virtio-win-*.iso)}
UNATTEND_ISO=$VMDIR/unattend.iso
AUTHORIZED_KEYS=${LC_WIN_AUTHORIZED_KEYS:-$VMDIR/authorized_keys}
ADMIN_PASSWORD=$VMDIR/secrets/admin-password
SSH_PORT=${LC_WIN_SSH_PORT:-2223}
CPUS=${LC_WIN_CPUS:-16}
MEM=${LC_WIN_MEM:-32G}

tool () { "$PY" "$TOOLS/wintool.py" "$@"; }

running () { [ -f "$RUN/qemu.pid" ] && kill -0 "$(cat "$RUN/qemu.pid")" 2>/dev/null; }

prepare ()
{
	[ -f "$WIN_ISO" ] || { echo "no Windows ISO: put one into $VMDIR/iso or set LC_WIN_ISO" >&2; exit 1; }
	[ -f "$VIRTIO_ISO" ] || { echo "no virtio-win ISO: put one into $VMDIR/iso or set LC_WIN_VIRTIO_ISO" >&2; exit 1; }
	[ -s "$AUTHORIZED_KEYS" ] || { echo "no public keys for the guest's sshd in $AUTHORIZED_KEYS" >&2; exit 1; }
	mkdir -p "$VMDIR/disk" "$VMDIR/secrets" "$RUN"
	chmod 700 "$VMDIR/secrets"
	[ -f "$DISK" ] || qemu-img create -q -f qcow2 "$DISK" "$DISK_SIZE"
	if [ ! -f "$ADMIN_PASSWORD" ]; then
		(umask 077; "$PY" -c 'import secrets, string
alphabet = string.ascii_letters + string.digits
while True:
	pw = "".join (secrets.choice (alphabet) for _ in range (24))
	if any (c.islower () for c in pw) and any (c.isupper () for c in pw) and any (c.isdigit () for c in pw):
		print (pw)
		break' > "$ADMIN_PASSWORD")
	fi
	index=$(tool wiminfo "$WIN_ISO" | awk -F '\t' '$2 ~ /SERVERSTANDARD$/ { print $1 }')
	[ -n "$index" ] || { echo "no Standard (Desktop Experience) image in $WIN_ISO" >&2; exit 1; }
	media=$(mktemp -d)
	trap 'rm -rf "$media"' EXIT
	sed -e "s/@IMAGE_INDEX@/$index/" -e "s/@ADMIN_PASSWORD@/$(cat "$ADMIN_PASSWORD")/" \
		"$TOOLS/unattend/autounattend.xml.in" > "$media/autounattend.xml"
	cp "$TOOLS/unattend/bootstrap.ps1" "$media/"
	cp "$AUTHORIZED_KEYS" "$media/authorized_keys.txt"
	(umask 077; tool mkiso "$UNATTEND_ISO" "$media/autounattend.xml" "$media/bootstrap.ps1" "$media/authorized_keys.txt")
	echo "disk $DISK, image index $index, unattend $UNATTEND_ISO"
}

start ()
{
	running && { echo "already running" >&2; exit 1; }
	mkdir -p "$RUN"
	# The guest agent and the virtio drivers get installed from it at the first logon.
	virtio_media=
	if [ -f "$VIRTIO_ISO" ]; then
		virtio_media="-drive if=none,id=virtio-cd,media=cdrom,readonly=on,file=$VIRTIO_ISO
			-device ide-cd,bus=ide.2,drive=virtio-cd"
	fi
	install_media=
	if [ "${1-}" = --install ]; then
		install_media="-drive if=none,id=win-cd,media=cdrom,readonly=on,file=$WIN_ISO
			-device ide-cd,bus=ide.1,drive=win-cd,bootindex=0
			-drive if=none,id=unattend-cd,media=cdrom,readonly=on,file=$UNATTEND_ISO
			-device ide-cd,bus=ide.3,drive=unattend-cd"
	fi
	# SCHED_IDLE: the VM only gets cycles no CI cell wants, so CI timings stay undisturbed.
	# hv-relaxed keeps Windows from bugchecking when its vCPUs starve meanwhile.
	# -svm hides nested virtualization, so Windows does not turn on VBS and run itself under Hyper-V.
	# shellcheck disable=SC2086
	chrt --idle 0 nice -n 19 qemu-system-x86_64 \
		-name lc-win,process=lc-win \
		-machine q35,accel=kvm,hpet=off \
		-cpu host,-svm,topoext=on,hv-relaxed,hv-vapic,hv-spinlocks=0x1fff,hv-vpindex,hv-runtime,hv-synic,hv-stimer,hv-stimer-direct,hv-reset,hv-frequencies,hv-tlbflush,hv-ipi,hv-time \
		-smp "$CPUS",sockets=1,cores=$((CPUS / 2)),threads=2 \
		-m "$MEM" \
		-rtc base=utc,driftfix=slew \
		-global kvm-pit.lost_tick_policy=discard \
		-drive if=none,id=sys,file="$DISK",format=qcow2,cache=none,aio=native,discard=unmap,detect-zeroes=unmap \
		-device ide-hd,bus=ide.0,drive=sys,rotation_rate=1,bootindex=1 \
		$virtio_media \
		$install_media \
		-netdev user,id=net0,hostfwd=tcp:127.0.0.1:"$SSH_PORT"-:22 \
		-device e1000e,netdev=net0 \
		-device virtio-serial-pci \
		-chardev socket,id=qga0,path="$RUN/qga.sock",server=on,wait=off \
		-device virtserialport,chardev=qga0,name=org.qemu.guest_agent.0 \
		-device qemu-xhci -device usb-tablet \
		-vga std \
		-display none \
		-vnc unix:"$RUN/vnc.sock" \
		-qmp unix:"$RUN/qmp.sock",server=on,wait=off \
		-pidfile "$RUN/qemu.pid" \
		-daemonize
	# Other members of the kvm group may watch the console through VNC.
	chgrp kvm "$RUN/vnc.sock" 2> /dev/null && chmod 660 "$RUN/vnc.sock" || true
}

stop ()
{
	running || { echo "not running"; return 0; }
	if tool qga "$RUN/qga.sock" guest-ping > /dev/null 2>&1; then
		tool qga "$RUN/qga.sock" guest-shutdown > /dev/null
	else
		tool qmp "$RUN/qmp.sock" system_powerdown > /dev/null
	fi
	for _ in $(seq 180); do
		running || return 0
		sleep 1
	done
	echo "still running after 180 s, pulling the plug" >&2
	tool qmp "$RUN/qmp.sock" quit > /dev/null
}

case "${1-}" in
prepare) prepare ;;
start) shift; start "$@" ;;
stop) stop ;;
status)
	if running; then
		echo "running (pid $(cat "$RUN/qemu.pid")), $(tool qmp "$RUN/qmp.sock" query-status | tr -d '\n ')"
	else
		echo "stopped"
	fi
	;;
screenshot) tool screenshot "$RUN/qmp.sock" "$2" ;;
qmp) shift; tool qmp "$RUN/qmp.sock" "$@" ;;
qga) shift; tool qga "$RUN/qga.sock" "$@" ;;
exec) tool exec "$RUN/qga.sock" "${2:--}" ;;
*) sed -n '2,5p' "$0" >&2; exit 2 ;;
esac
