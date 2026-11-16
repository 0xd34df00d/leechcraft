#!/usr/bin/env python3
"""Helpers for the Windows build VM: QMP and guest agent access, the unattend ISO, install.wim editions."""

import io
import json
import os
import socket
import struct
import sys
import zlib


def _json_lines (path, timeout):
	sock = socket.socket (socket.AF_UNIX)
	sock.settimeout (timeout)
	sock.connect (path)
	return sock, sock.makefile ('rwb')


def _send (stream, command, arguments = None):
	message = { 'execute': command }
	if arguments:
		message ['arguments'] = arguments
	stream.write (json.dumps (message).encode () + b'\n')
	stream.flush ()


def _reply (stream):
	while True:
		line = stream.readline ()
		if not line:
			raise RuntimeError ('connection closed')
		message = json.loads (line)
		if 'event' in message:
			continue
		if 'error' in message:
			raise RuntimeError (message ['error'].get ('desc', message ['error']))
		return message.get ('return')


def qmp (path, command, arguments = None, timeout = 30):
	sock, stream = _json_lines (path, timeout)
	with sock:
		json.loads (stream.readline ())
		_send (stream, 'qmp_capabilities')
		_reply (stream)
		_send (stream, command, arguments)
		return _reply (stream)


def qga (path, command, arguments = None, timeout = 10):
	sock, stream = _json_lines (path, timeout)
	with sock:
		# The agent keeps no session state, but a stale half-read reply from an earlier
		# client may still sit in the channel: the delimited sync skips past it.
		token = int.from_bytes (os.urandom (4), 'little') & 0x7fffffff
		stream.write (b'\xff')
		_send (stream, 'guest-sync-delimited', { 'id': token })
		while True:
			byte = stream.read (1)
			if not byte:
				raise RuntimeError ('connection closed')
			if byte == b'\xff':
				break
		if _reply (stream) != token:
			raise RuntimeError ('guest agent sync failed')
		_send (stream, command, arguments)
		if command in ('guest-shutdown',):
			return None
		return _reply (stream)


def guest_powershell (path, script, timeout = 600):
	import base64
	import time

	encoded = base64.b64encode (script.encode ('utf-16-le')).decode ()
	pid = qga (path, 'guest-exec', {
			'path': 'powershell.exe',
			'arg': [ '-NoProfile', '-NonInteractive', '-EncodedCommand', encoded ],
			'capture-output': True,
		}) ['pid']
	deadline = time.monotonic () + timeout
	while time.monotonic () < deadline:
		status = qga (path, 'guest-exec-status', { 'pid': pid })
		if status.get ('exited'):
			for stream, key in ((sys.stdout, 'out-data'), (sys.stderr, 'err-data')):
				stream.write (base64.b64decode (status.get (key, '')).decode ('utf-8', 'replace'))
			return status.get ('exitcode', 0)
		time.sleep (1)
	raise RuntimeError (f'guest process {pid} still running after {timeout} s')


def _ppm_to_png (ppm, out):
	data = ppm.split (b'\n', 3)
	width, height = map (int, data [1].split ())
	pixels = data [3]
	stride = width * 3
	raw = b''.join (b'\x00' + pixels [y * stride:(y + 1) * stride] for y in range (height))

	def chunk (kind, body):
		return struct.pack ('>I', len (body)) + kind + body + struct.pack ('>I', zlib.crc32 (kind + body) & 0xffffffff)

	header = struct.pack ('>IIBBBBB', width, height, 8, 2, 0, 0, 0)
	with open (out, 'wb') as f:
		f.write (b'\x89PNG\r\n\x1a\n' + chunk (b'IHDR', header) + chunk (b'IDAT', zlib.compress (raw, 6)) + chunk (b'IEND', b''))


def screenshot (path, out):
	out = os.path.abspath (out)
	try:
		qmp (path, 'screendump', { 'filename': out, 'format': 'png' })
	except RuntimeError:
		ppm = out + '.ppm'
		qmp (path, 'screendump', { 'filename': ppm })
		with open (ppm, 'rb') as f:
			_ppm_to_png (f.read (), out)
		os.remove (ppm)


def make_iso (out, files):
	import pycdlib

	iso = pycdlib.PyCdlib ()
	iso.new (interchange_level = 3, joliet = 3, vol_ident = 'LC_UNATTEND')
	for path in files:
		name = os.path.basename (path)
		with open (path, 'rb') as f:
			data = f.read ()
		stem, _, ext = name.upper ().replace ('-', '_').partition ('.')
		iso.add_fp (io.BytesIO (data), len (data), f'/{stem}.{ext};1', joliet_path = '/' + name)
	iso.write (out)
	iso.close ()


def wim_images (iso_path):
	import pycdlib
	import xml.etree.ElementTree as ET

	iso = pycdlib.PyCdlib ()
	iso.open (iso_path)
	try:
		with iso.open_file_from_iso (udf_path = '/sources/install.wim') as f:
			header = f.read (208)
			if header [:8] != b'MSWIM\0\0\0':
				raise RuntimeError ('not a WIM file')
			xml_size = int.from_bytes (header [72:79], 'little')
			xml_offset = int.from_bytes (header [80:88], 'little')
			f.seek (xml_offset)
			xml = f.read (xml_size).decode ('utf-16')
	finally:
		iso.close ()
	for image in ET.fromstring (xml).findall ('IMAGE'):
		yield image.get ('INDEX'), image.findtext ('NAME'), image.findtext ('DISPLAYNAME') or image.findtext ('DESCRIPTION')


def main (argv):
	command, args = argv [1], argv [2:]
	if command in ('qmp', 'qga'):
		arguments = json.loads (args [2]) if len (args) > 2 else None
		result = (qmp if command == 'qmp' else qga) (args [0], args [1], arguments)
		print (json.dumps (result, indent = 1))
	elif command == 'exec':
		script = sys.stdin.read () if args [1] == '-' else args [1]
		sys.exit (guest_powershell (args [0], script))
	elif command == 'screenshot':
		screenshot (args [0], args [1])
	elif command == 'mkiso':
		make_iso (args [0], args [1:])
	elif command == 'wiminfo':
		for index, name, display in wim_images (args [0]):
			print (f'{index}\t{name}\t{display}')
	else:
		sys.exit (f'unknown command {command}')


if __name__ == '__main__':
	main (sys.argv)
