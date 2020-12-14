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
#include <QFile>
#include <QSettings>

namespace LC::Util::SysInfo
{
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

			proc.start (QStringLiteral ("/bin/sh"),
						QStringList { "-c", "lsb_release -ds" },
						QIODevice::ReadOnly);
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
			static const auto osReleaseFile = QStringLiteral ("/etc/os-release");
			if (!QFile::exists (osReleaseFile))
				return {};

			QSettings relFile { osReleaseFile, QSettings::IniFormat };
			relFile.setIniCodec ("UTF-8");

			const auto& prettyName = relFile.value (QStringLiteral ("PRETTY_NAME")).toString ();
			const auto& name = relFile.value (QStringLiteral ("NAME")).toString ();
			const auto& version = relFile.value (QStringLiteral ("VERSION")).toString ();
			return !prettyName.isEmpty () ? prettyName : (name + " " + version);
		}

		QString GetEtcName ()
		{
			struct OsInfo
			{
				QString path;
				QString name;
			};
			static const auto osptr = std::to_array<OsInfo> ({
					{ QStringLiteral ("/etc/mandrake-release"), QStringLiteral ("Mandrake Linux") },
					{ QStringLiteral ("/etc/debian_version"), QStringLiteral ("Debian GNU/Linux") },
					{ QStringLiteral ("/etc/gentoo-release"), QStringLiteral ("Gentoo Linux") },
					{ QStringLiteral ("/etc/exherbo-release"), QStringLiteral ("Exherbo") },
					{ QStringLiteral ("/etc/arch-release"), QStringLiteral ("Arch Linux") },
					{ QStringLiteral ("/etc/slackware-version"), QStringLiteral ("Slackware Linux") },
					{ QStringLiteral ("/etc/pld-release"), {} },
					{ QStringLiteral ("/etc/lfs-release"), QStringLiteral ("LFS") },
					{ QStringLiteral ("/etc/SuSE-release"), QStringLiteral ("SuSE linux") },
					{ QStringLiteral ("/etc/conectiva-release"), QStringLiteral ("Connectiva") },
					{ QStringLiteral ("/etc/.installed"), {} },
					{ QStringLiteral ("/etc/redhat-release"), {} },
				});
			for (const auto& os : osptr)
			{
				QFile f (os.path);
				if (f.open (QIODevice::ReadOnly))
				{
					QString data = QString (f.read (1024)).trimmed ();
					return os.name.isEmpty () ?
							data :
							QStringLiteral ("%1 (%2)").arg (os.name, data);
				}
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

			static const auto nameMarker = QStringLiteral ("NAME=");
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
			return OSInfo { .Arch_ = "x86_64", .Name_ = "Mac OS X", .Version_ = version };
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
				.Arch_ = QSysInfo::WordSize == 64 ? "x86_64" : "x86",
				.Name_ = "Windows",
				.Version_ = version
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
			.Arch_ = u.machine,
			.Name_ = osName.isEmpty () ? u.sysname : osName,
			.Version_ = QString ("%1 %2 %3").arg (u.machine, u.release, u.version),
			.Flavour_ = u.sysname,
		};
#endif

		return { .Arch_ = "Unknown arch", .Name_ = "Unknown OS", .Version_ = "Unknown version" };
	}
}
