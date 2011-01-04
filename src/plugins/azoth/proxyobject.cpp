/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "proxyobject.h"

#if defined(Q_WS_X11)
#include <sys/utsname.h>
#endif

#include <QProcess>
#include <QTextStream>
#include <QtDebug>
#include <plugininterface/util.h>
#include "interfaces/iaccount.h"
#include "core.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
	ProxyObject::ProxyObject (QObject* parent)
	: QObject (parent)
	{
		SerializedStr2AuthStatus_ ["None"] = Plugins::ASNone;
		SerializedStr2AuthStatus_ ["NoneOut"] = Plugins::ASNoneOut;
		SerializedStr2AuthStatus_ ["NoneIn"] = Plugins::ASNoneIn;
		SerializedStr2AuthStatus_ ["NoneOutIn"] = Plugins::ASNoneOutIn;
		SerializedStr2AuthStatus_ ["To"] = Plugins::ASTo;
		SerializedStr2AuthStatus_ ["ToIn"] = Plugins::ASToIn;
		SerializedStr2AuthStatus_ ["From"] = Plugins::ASFrom;
		SerializedStr2AuthStatus_ ["FromIn"] = Plugins::ASFromIn;
		SerializedStr2AuthStatus_ ["Both"] = Plugins::ASBoth;
	}

	QString ProxyObject::GetPassword (QObject *accObj)
	{
		Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount"
					<< accObj;
			return QString ();
		}

		QList<QVariant> keys;
		keys << "org.LeechCraft.Azoth.PassForAccount/" + acc->GetAccountID ();
		const QVariantList& result =
				Util::GetPersistentData (keys, &Core::Instance ());
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}

	void ProxyObject::SetPassword (const QString& password, QObject *accObj)
	{
		Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount"
					<< accObj;
			return;
		}

		QList<QVariant> keys;
		keys << "org.LeechCraft.Azoth.PassForAccount/" + acc->GetAccountID ();

		QList<QVariant> passwordVar;
		passwordVar << password;
		QList<QVariant> values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		Core::Instance ().SendEntity (e);
	}

	QString ProxyObject::GetOSName ()
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

		return tr ("Unknown OS");
	}

	QString ProxyObject::StateToString (Plugins::State st) const
	{
		switch (st)
		{
		case Plugins::SOnline:
			return Core::tr ("Online");
		case Plugins::SChat:
			return Core::tr ("Free to chat");
		case Plugins::SAway:
			return Core::tr ("Away");
		case Plugins::SDND:
			return Core::tr ("Do not disturb");
		case Plugins::SXA:
			return Core::tr ("Extended away");
		case Plugins::SOffline:
			return Core::tr ("Offline");
		default:
			return Core::tr ("Error");
		}
	}

	QString ProxyObject::AuthStatusToString (Plugins::AuthStatus status) const
	{
		switch (status)
		{
		case Plugins::ASNone:
			return "None";
		case Plugins::ASNoneOut:
			return "NoneOut";
		case Plugins::ASNoneIn:
			return "NoneIn";
		case Plugins::ASNoneOutIn:
			return "NoneOutIn";
		case Plugins::ASTo:
			return "To";
		case Plugins::ASToIn:
			return "ToIn";
		case Plugins::ASFrom:
			return "From";
		case Plugins::ASFromIn:
			return "FromIn";
		case Plugins::ASBoth:
			return "Both";
		}
	}

	Plugins::AuthStatus ProxyObject::AuthStatusFromString (const QString& str) const
	{
		return SerializedStr2AuthStatus_.value (str, Plugins::ASNone);;
	}
}
}
}
