/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "sysinfo.h"
#if defined(Q_WS_X11)
#include <sys/utsname.h>
#endif

#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>

namespace LeechCraft
{
namespace Util
{
namespace SysInfo
{
	QString GetOSName ()
	{
#if defined(Q_WS_X11)
		QProcess proc;
		proc.start (QString ("/bin/sh"),
					QStringList ("-c") << "lsb_release -ds", QIODevice::ReadOnly);
		if (proc.waitForStarted ())
		{
			QTextStream stream (&proc);
			QString ret;
			while (proc.waitForReadyRead ())
				ret += stream.readAll ();
			proc.close ();
			if (!ret.isEmpty ())
				return ret.remove ('"').trimmed ();
		}

		struct OsInfo_t
		{
			QString path;
			QString name;
		} OsInfo [] =
		{
			{ "/etc/mandrake-release", "Mandrake Linux" },
			{ "/etc/debian_version", "Debian GNU/Linux" },
			{ "/etc/gentoo-release", "Gentoo Linux" },
			{ "/etc/exherbo-release", "Exherbo" },
			{ "/etc/arch-release", "Arch Linux" },
			{ "/etc/slackware-version", "Slackware Linux" },
			{ "/etc/pld-release", "" },
			{ "/etc/lfs-release", "LFS" },
			{ "/etc/SuSE-release", "SuSE linux" },
			{ "/etc/conectiva-release", "Connectiva" },
			{ "/etc/.installed", "" },
			{ "/etc/redhat-release", "" },
			{ "", "" }
		};
		OsInfo_t *osptr = OsInfo;
		while (!osptr->path.isEmpty ())
		{
			QFileInfo fi (osptr->path);
			if (fi.exists ())
			{
				QFile f (osptr->path);
				f.open (QIODevice::ReadOnly);
				QString data = QString (f.read (1024)).trimmed ();
				if (osptr->name.isEmpty ())
					return data;
				else
					return QString ("%1 (%2)")
							.arg (osptr->name)
							.arg (data);
			}
			++osptr;
		}
		utsname u;
		uname (&u);
		return QString ("%1 %2 %3 %4")
			.arg (u.sysname)
			.arg (u.machine)
			.arg (u.release)
			.arg (u.version);
#endif
#if defined(Q_WS_MAC)
		QSysInfo::MacVersion v = QSysInfo::MacintoshVersion;
		if (v == QSysInfo::MV_10_3)
			return "Mac OS X 10.3";
		else if(v == QSysInfo::MV_10_4)
			return "Mac OS X 10.4";
		else if(v == QSysInfo::MV_10_5)
			return "Mac OS X 10.5";
#if QT_VERSION >= 0x040500
		else if(v == QSysInfo::MV_10_6)
			return "Mac OS X 10.6";
#endif
		else
			return "Mac OS X";
#endif

#if defined(Q_WS_WIN)
		QSysInfo::WinVersion v = QSysInfo::WindowsVersion;
		if (v == QSysInfo::WV_95)
			return "Windows 95";
		else if (v == QSysInfo::WV_98)
			return "Windows 98";
		else if (v == QSysInfo::WV_Me)
			return "Windows Me";
		else if (v == QSysInfo::WV_DOS_based)
			return "Windows 9x/Me";
		else if (v == QSysInfo::WV_NT)
			return "Windows NT 4.x";
		else if (v == QSysInfo::WV_2000)
			return "Windows 2000";
		else if (v == QSysInfo::WV_XP)
			return "Windows XP";
		else if (v == QSysInfo::WV_2003)
			return "Windows Server 2003";
		else if (v == QSysInfo::WV_VISTA)
			return "Windows Vista";
#if QT_VERSION >= 0x040500
		else if (v == QSysInfo::WV_WINDOWS7)
			return "Windows 7";
#endif
		else if (v == QSysInfo::WV_NT_based)
			return "Windows NT";
#endif

		return "Unknown OS";
	}
}
}
}
