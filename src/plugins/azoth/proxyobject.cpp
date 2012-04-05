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

#include "proxyobject.h"
#include <QInputDialog>
#include <QtDebug>
#include <util/util.h>
#include <util/sysinfo.h>
#include "interfaces/azoth/iaccount.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"

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

	QObject* ProxyObject::GetSettingsManager ()
	{
		return &XmlSettingsManager::Instance ();
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

	QString ProxyObject::GetAccountPassword (QObject *accObj, bool useStored)
	{
		if (useStored)
		{
			const QString& result = GetPassword (accObj);
			if (!result.isNull ())
				return result;
		}

		IAccount *acc = qobject_cast<IAccount*> (accObj);

		QString result = QInputDialog::getText (0,
				"LeechCraft",
				tr ("Enter password for %1:").arg (acc->GetAccountName ()),
				QLineEdit::Password);
		if (!result.isNull ())
			SetPassword (result, accObj);
		return result;
	}

	QString ProxyObject::GetOSName ()
	{
		return Util::SysInfo::GetOSName ();
	}

	bool ProxyObject::IsAutojoinAllowed ()
	{
		return XmlSettingsManager::Instance ()
				.property ("IsAutojoinAllowed").toBool ();
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
			return Core::tr ("Not available");
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
		case ASContactRequested:
			return "Requested";
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown status"
					<< status;
			return "Unknown";
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

	QList<QObject*> ProxyObject::GetAllAccounts () const
	{
		QList<QObject*> result;
		Q_FOREACH (IAccount *acc, Core::Instance ().GetAccounts ())
			result << acc->GetObject ();
		return result;
	}

	QObject* ProxyObject::GetEntry (const QString& entryID, const QString&) const
	{
		return Core::Instance ().GetEntry (entryID);
	}

	void ProxyObject::OpenChat (const QString& entryID, const QString& accID,
			const QString& message, const QString& variant) const
	{
		ChatTabsManager *mgr = Core::Instance ().GetChatTabsManager ();

		ICLEntry *entry = qobject_cast<ICLEntry*> (GetEntry (entryID, accID));
		QWidget *chat = mgr->OpenChat (entry);

		QMetaObject::invokeMethod (chat,
				"prepareMessageText",
				Qt::QueuedConnection,
				Q_ARG (QString, message));
		QMetaObject::invokeMethod (chat,
				"selectVariant",
				Qt::QueuedConnection,
				Q_ARG (QString, variant));
	}

	QString ProxyObject::GetSelectedChatTemplate (QObject *entry, QWebFrame *frame) const
	{
		return Core::Instance ().GetSelectedChatTemplate (entry, frame);
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

	Util::ResourceLoader* ProxyObject::GetResourceLoader (IProxyObject::PublicResourceLoader loader) const
	{
		switch (loader)
		{
		case PRLClientIcons:
			return Core::Instance ().GetResourceLoader (Core::RLTClientIconLoader);
		case PRLStatusIcons:
			return Core::Instance ().GetResourceLoader (Core::RLTStatusIconLoader);
		case PRLSystemIcons:
			return Core::Instance ().GetResourceLoader (Core::RLTSystemIconLoader);
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown type"
					<< loader;
			return 0;
		}
	}

	QIcon ProxyObject::GetIconForState (State state) const
	{
		return Core::Instance ().GetIconForState (state);
	}
}
}
