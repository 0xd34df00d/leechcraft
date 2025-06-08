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

			const QSettings relFile { osReleaseFile, QSettings::IniFormat };
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
#if defined(Q_OS_MAC) || defined(Q_OS_WIN32)
		return OSInfo
		{
			.Arch_ = QSysInfo::currentCpuArchitecture (),
			.Name_ = QSysInfo::productType (),
			.Version_ = QSysInfo::productVersion (),
		};
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

		return { .Arch_ = "Unknown arch", .Name_ = "Unknown OS", .Version_ = "Unknown version", .Flavour_ = {} };
	}
}
