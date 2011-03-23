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
namespace Azoth
{
	ProxyObject::ProxyObject (QObject* parent)
	: QObject (parent)
	{
		SerializedStr2AuthStatus_ ["None"] = ASNone;
		SerializedStr2AuthStatus_ ["To"] = ASTo;
		SerializedStr2AuthStatus_ ["From"] = ASFrom;
		SerializedStr2AuthStatus_ ["Both"] = ASBoth;
	}

	QString ProxyObject::GetPassword (QObject *accObj)
	{
		IAccount *acc = qobject_cast<IAccount*> (accObj);
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
		IAccount *acc = qobject_cast<IAccount*> (accObj);
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

	QString ProxyObject::StateToString (State st) const
	{
		switch (st)
		{
		case SOnline:
			return Core::tr ("Online");
		case SChat:
			return Core::tr ("Free to chat");
		case SAway:
			return Core::tr ("Away");
		case SDND:
			return Core::tr ("Do not disturb");
		case SXA:
			return Core::tr ("Extended away");
		case SOffline:
			return Core::tr ("Offline");
		default:
			return Core::tr ("Error");
		}
	}

	QString ProxyObject::AuthStatusToString (AuthStatus status) const
	{
		switch (status)
		{
		case ASNone:
			return "None";
		case ASTo:
			return "To";
		case ASFrom:
			return "From";
		case ASBoth:
			return "Both";
		}
	}

	AuthStatus ProxyObject::AuthStatusFromString (const QString& str) const
	{
		return SerializedStr2AuthStatus_.value (str, ASNone);;
	}
	
	QObject* ProxyObject::GetAccount (const QString& accID) const
	{
		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			if (acc->GetAccountID () == accID)
				return acc->GetObject ();

		return 0;
	}
	
	QObject* ProxyObject::GetEntry (const QString& entryID, const QString&) const
	{
		return Core::Instance ().GetEntry (entryID);
	}
	
	QString ProxyObject::GetSelectedChatTemplate (QObject *entry) const
	{
		return Core::Instance ().GetSelectedChatTemplate (entry);
	}
	
	void ProxyObject::AppendMessageByTemplate (QWebFrame *frame,
			QObject *msg, const QString& color, bool isHighlight, bool isActive) const
	{
		Core::Instance ().AppendMessageByTemplate (frame, msg, color, isHighlight, isActive);
	}
	
	QList<QColor> ProxyObject::GenerateColors (const QString& scheme) const
	{
		return Core::Instance ().GenerateColors (scheme);
	}
	
	QString ProxyObject::GetNickColor (const QString& nick, const QList<QColor>& colors) const
	{
		return Core::Instance ().GetNickColor (nick, colors);
	}
	
	QString ProxyObject::FormatDate (QDateTime dt, QObject *obj) const
	{
		return Core::Instance ().FormatDate (dt, qobject_cast<IMessage*> (obj));
	}
	
	QString ProxyObject::FormatNickname (QString nick, QObject *obj, const QString& color) const
	{
		return Core::Instance ().FormatNickname (nick, qobject_cast<IMessage*> (obj), color);
	}
	
	QString ProxyObject::FormatBody (QString body, QObject *obj) const
	{
		return Core::Instance ().FormatBody (body, qobject_cast<IMessage*> (obj));
	}
	
	void ProxyObject::PreprocessMessage (QObject *msgObj)
	{
		if (msgObj->property ("Azoth/DoNotPreprocess").toBool ())
			return;

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "message"
					<< msgObj
					<< "is not an IMessage";
			return;
		}
		
		switch (msg->GetMessageSubType ())
		{
		case IMessage::MSTParticipantStatusChange:
		{
			const QString& nick = msgObj->property ("Azoth/Nick").toString ();
			const QString& state = msgObj->property ("Azoth/TargetState").toString ();
			const QString& text = msgObj->property ("Azoth/StatusText").toString ();
			if (!nick.isEmpty () && !state.isEmpty ())
			{
				const QString& newBody = text.isEmpty () ?
						tr ("%1 changed status to %2")
							.arg (nick)
							.arg (state) :
						tr ("%1 changed status to %2 (%3)")
							.arg (nick)
							.arg (state)
							.arg (text);
				msg->SetBody (newBody);
			}
			break;
		}
		default:
			break;
		}
	}
}
}
