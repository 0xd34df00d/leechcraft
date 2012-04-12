/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#if not defined(Q_OS_WIN32)
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
		const auto& pair = GetOSNameSplit ();
		return pair.first + ' ' + pair.second;
	}

	QPair<QString, QString> GetOSNameSplit ()
	{
#if defined(Q_OS_MAC)
		QSysInfo::MacVersion v = QSysInfo::MacintoshVersion;
		if (v == QSysInfo::MV_10_3)
			return qMakePair ("Mac OS X", "10.3");
		else if(v == QSysInfo::MV_10_4)
			return qMakePair ("Mac OS X", "10.4");
		else if(v == QSysInfo::MV_10_5)
			return qMakePair ("Mac OS X", "10.5");
		else if(v == QSysInfo::MV_10_6)
			return qMakePair ("Mac OS X", "10.6");
		else
			return qMakePair ("Mac OS X", "Unknown version");
#elif defined(Q_OS_WIN32)
		QSysInfo::WinVersion v = QSysInfo::WindowsVersion;
		if (v == QSysInfo::WV_95)
			return qMakePair ("Windows", "95");
		else if (v == QSysInfo::WV_98)
			return qMakePair ("Windows", "98");
		else if (v == QSysInfo::WV_Me)
			return qMakePair ("Windows", "Me");
		else if (v == QSysInfo::WV_DOS_based)
			return qMakePair ("Windows", "9x/Me");
		else if (v == QSysInfo::WV_NT)
			return qMakePair ("Windows", "NT 4.x");
		else if (v == QSysInfo::WV_2000)
			return qMakePair ("Windows", "2000");
		else if (v == QSysInfo::WV_XP)
			return qMakePair ("Windows", "XP");
		else if (v == QSysInfo::WV_2003)
			return qMakePair ("Windows", "2003");
		else if (v == QSysInfo::WV_VISTA)
			return qMakePair ("Windows", "Vista");
		else if (v == QSysInfo::WV_WINDOWS7)
			return qMakePair ("Windows", "7");
		else if (v == QSysInfo::WV_NT_based)
			return qMakePair ("Windows", "NT-based");
#else
		QString osName;

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
				osName = ret.remove ('"').trimmed ();
		}

		if (osName.isEmpty ())
		{
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
						osName = data;
					else
						osName = QString ("%1 (%2)")
								.arg (osptr->name)
								.arg (data);
					break;
				}
				++osptr;
			}
		}

		utsname u;
		uname (&u);

		return qMakePair (osName.isEmpty () ? QString (u.sysname) : osName,
				QString ("%1 %2 %3").arg (u.machine, u.release, u.version));
#endif

		return qMakePair (QString ("Unknown OS"), QString ("Unknown version"));
	}
}
}
}
