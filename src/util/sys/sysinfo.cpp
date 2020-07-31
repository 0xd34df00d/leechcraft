/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sysinfo.h"
#if !defined(Q_OS_WIN32)
#include <sys/utsname.h>
#endif

#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QFile>
#include <QSettings>

namespace LC
{
namespace Util
{
namespace SysInfo
{
	OSInfo::OSInfo (const QString& arch, const QString& name, const QString& version)
	: OSInfo { arch, name, name, version }
	{
	}

	OSInfo::OSInfo (const QString& arch, const QString& flavour,
			const QString& name, const QString& version)
	: Name_ { name }
	, Version_ { version }
	, Flavour_ { flavour }
	, Arch_ { arch }
	{
	}

	QString GetOSName ()
	{
		const auto& info = GetOSInfo ();
		return info.Name_ + ' ' + info.Version_;
	}

	namespace Linux
	{
		QString GetLSBName ()
		{
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

			return {};
		}

		QString GetEtcOsName ()
		{
			if (!QFile::exists ("/etc/os-release"))
				return {};

			QSettings relFile { "/etc/os-release", QSettings::IniFormat };
			relFile.setIniCodec ("UTF-8");

			const auto& prettyName = relFile.value ("PRETTY_NAME").toString ();
			const auto& name = relFile.value ("NAME").toString ();
			const auto& version = relFile.value ("VERSION").toString ();
			return !prettyName.isEmpty () ? prettyName : (name + " " + version);
		}

		QString GetEtcName ()
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
				QFile f (osptr->path);
				if (f.open (QIODevice::ReadOnly))
				{
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

			return {};
		}
	}

	namespace
	{
#ifndef Q_OS_MAC
		void Normalize (QString& osName)
		{
			auto trimQuotes = [&osName]
			{
				if (osName.startsWith ('"') && osName.endsWith ('"'))
					osName = osName.mid (1, osName.size () - 1);
			};

			trimQuotes ();

			const QString nameMarker ("NAME=");
			if (osName.startsWith (nameMarker))
				osName = osName.mid (nameMarker.size ());

			trimQuotes ();
		}
#endif
	}

	OSInfo GetOSInfo ()
	{
#if defined(Q_OS_MAC)
		const auto retVer = [] (const QString& version)
		{
			// LC only supports building on OS X 10.7 and higher, which all work only on x86_64.
			return OSInfo { "x86_64", "Mac OS X", version };
		};

		for (auto minor = 7; minor < 16; ++minor)
			if (QSysInfo::MacintoshVersion == Q_MV_OSX (10, minor))
				return retVer ("10." + QString::number (minor));

		return retVer ("Unknown version");
#elif defined(Q_OS_WIN32)
		const auto retVer = [] (const QString& version)
		{
			return OSInfo
			{
				QSysInfo::WordSize == 64 ? "x86_64" : "x86",
				"Windows",
				version
			};
		};

		switch (QSysInfo::WindowsVersion)
		{
		case QSysInfo::WV_95:
			return retVer ("95");
		case QSysInfo::WV_98:
			return retVer ("98");
		case QSysInfo::WV_Me:
			return retVer ("Me");
		case QSysInfo::WV_DOS_based:
			return retVer ("9x/Me");
		case QSysInfo::WV_NT:
			return retVer ("NT 4.x");
		case QSysInfo::WV_2000:
			return retVer ("2000");
		case QSysInfo::WV_XP:
			return retVer ("XP");
		case QSysInfo::WV_2003:
			return retVer ("2003");
		case QSysInfo::WV_VISTA:
			return retVer ("Vista");
		case QSysInfo::WV_WINDOWS7:
			return retVer ("7");
		case 0x00a0:
			return retVer ("8");
		case 0x00b0:
			return retVer ("8.1");
		case 0x00c0:
			return retVer ("10");
		case QSysInfo::WV_NT_based:
			return retVer ("NT-based");
		}
#else
		auto osName = Linux::GetEtcOsName ();

		if (osName.isEmpty ())
			osName = Linux::GetEtcName ();

		if (osName.isEmpty ())
			osName = Linux::GetLSBName ();

		Normalize (osName);

		utsname u;
		uname (&u);

		return
		{
			u.machine,
			u.sysname,
			osName.isEmpty () ? u.sysname : osName,
			QString ("%1 %2 %3").arg (u.machine, u.release, u.version)
		};
#endif

		return { "Unknown arch", "Unknown OS", "Unknown version" };
	}
}
}
}
