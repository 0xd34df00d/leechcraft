/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "proxyobject.h"
#include <algorithm>
#include <QInputDialog>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/xpc/passutils.h>
#include <util/sys/sysinfo.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/sll/unreachable.h>
#include "interfaces/azoth/iaccount.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"
#include "coremessage.h"
#include "unreadqueuemanager.h"
#include "resourcesmanager.h"
#include "util.h"
#include "customstatusesmanager.h"

namespace LC
{
namespace Azoth
{
	FormatterProxyObject::FormatterProxyObject ()
	: LinkRegexp_ ("((?:(?:\\w+://)|(?:xmpp:|mailto:|www\\.|magnet:|irc:))[^\\s<]+)",
			Qt::CaseInsensitive)
	{
	}

	QList<QColor> FormatterProxyObject::GenerateColors (const QString& scheme, QColor bg) const
	{
		return Azoth::GenerateColors (scheme, bg);
	}

	QString FormatterProxyObject::GetNickColor (const QString& nick, const QList<QColor>& colors) const
	{
		return Azoth::GetNickColor (nick, colors);
	}

	QString FormatterProxyObject::FormatDate (QDateTime dt, QObject *obj) const
	{
		return Core::Instance ().FormatDate (dt, qobject_cast<IMessage*> (obj));
	}

	QString FormatterProxyObject::FormatNickname (QString nick, QObject *obj, const QString& color) const
	{
		return Core::Instance ().FormatNickname (nick, qobject_cast<IMessage*> (obj), color);
	}

	QString FormatterProxyObject::EscapeBody (QString body, IMessage::EscapePolicy escape) const
	{
		switch (escape)
		{
		case IMessage::EscapePolicy::NoEscape:
			return body;
		case IMessage::EscapePolicy::Escape:
			return body
					.replace ('&', "&amp;"_ql)
					.replace ('"', "&quot;"_ql)
					.replace ('<', "&lt;"_ql)
					.replace ('>', "&gt;"_ql)
					.replace (R"(\n)"_ql, "<br/>"_ql)
					.replace ('\\', R"(\\)"_ql)
					;
		}
	}

	QString FormatterProxyObject::FormatBody (QString body, QObject *obj, const QList<QColor>& coloring) const
	{
		return Core::Instance ().FormatBody (body, qobject_cast<IMessage*> (obj), coloring);
	}

	void FormatterProxyObject::PreprocessMessage (QObject *msgObj)
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
		case IMessage::SubType::ParticipantStatusChange:
		{
			const QString& nick = msgObj->property ("Azoth/Nick").toString ();
			const QString& state = msgObj->property ("Azoth/TargetState").toString ();
			const QString& text = msgObj->property ("Azoth/StatusText").toString ();
			if (!nick.isEmpty () && !state.isEmpty ())
			{
				const auto& newBody = text.isEmpty () ?
						ProxyObject::tr ("%1 changed status to %2")
							.arg (nick)
							.arg (state) :
						ProxyObject::tr ("%1 changed status to %2 (%3)")
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

	const auto MaxBodySize4Links = 10 * 1024;

	void FormatterProxyObject::FormatLinks (QString& body)
	{
		if (body.size () > MaxBodySize4Links)
			return;

		int pos = 0;
		while ((pos = LinkRegexp_.indexIn (body, pos)) != -1)
		{
			const auto& link = LinkRegexp_.cap (1);
			if (pos > 0 &&
					(body.at (pos - 1) == '"' ||
						body.at (pos - 1) == '=' ||
						body.at (pos - 1) == '\''))
			{
				pos += link.size ();
				continue;
			}

			auto trimmed = link.trimmed ();
			if (trimmed.startsWith ("www."))
				trimmed.prepend ("http://");

			auto shortened = trimmed;
			const auto length = XmlSettingsManager::Instance ()
					.property ("ShortenURLLength").toInt ();
			if (shortened.size () > length)
				shortened = trimmed.left (length / 2) + "..." + trimmed.right (length / 2);

			const auto& str = "<a href=\"" + trimmed + "\" title=\"" + trimmed + "\">" + shortened + "</a>";
			body.replace (pos, link.length (), str);

			pos += str.length ();
		}
	}

	QStringList FormatterProxyObject::FindLinks (const QString& body)
	{
		QStringList result;

		if (body.size () > MaxBodySize4Links)
			return result;

		int pos = 0;
		while ((pos = LinkRegexp_.indexIn (body, pos)) != -1)
		{
			const auto& link = LinkRegexp_.cap (1);
			if (pos > 0 &&
					(body.at (pos - 1) == '"' || body.at (pos - 1) == '='))
			{
				pos += link.size ();
				continue;
			}

			result << link.trimmed ();
			pos += link.size ();
		}

		return result;
	}

	ProxyObject::ProxyObject (IAvatarsManager *am, QObject* parent)
	: QObject { parent }
	, SerializedStr2AuthStatus_
		{
			{ "None", ASNone },
			{ "To", ASTo },
			{ "From", ASFrom },
			{ "Both", ASBoth },
		}
	, AvatarsManager_ { am }
	{
	}

	QObject* ProxyObject::GetSettingsManager ()
	{
		return &XmlSettingsManager::Instance ();
	}

	void ProxyObject::SetPassword (const QString& password, QObject *accObj)
	{
		const auto acc = qobject_cast<IAccount*> (accObj);
		const auto& key = "org.LeechCraft.Azoth.PassForAccount/" + acc->GetAccountID ();
		Util::SavePassword (password, key, Core::Instance ().GetProxy ());
	}

	QString ProxyObject::GetAccountPassword (QObject *accObj, bool useStored)
	{
		const auto acc = qobject_cast<IAccount*> (accObj);
		const auto& key = "org.LeechCraft.Azoth.PassForAccount/" + acc->GetAccountID ();
		return Util::GetPassword (key,
				tr ("Enter password for %1:")
						.arg (acc->GetAccountName ()),
				Core::Instance ().GetProxy (),
				useStored);
	}

	bool ProxyObject::IsAutojoinAllowed ()
	{
		return XmlSettingsManager::Instance ()
				.property ("IsAutojoinAllowed").toBool ();
	}

	QString ProxyObject::StateToString (State st) const
	{
		return Azoth::StateToString (st);
	}

	QByteArray ProxyObject::AuthStatusToString (AuthStatus status) const
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

	AuthStatus ProxyObject::AuthStatusFromString (const QByteArray& str) const
	{
		return SerializedStr2AuthStatus_.value (str, ASNone);
	}

	QObject* ProxyObject::GetAccount (const QString& accID) const
	{
		for (const auto acc : Core::Instance ().GetAccounts ())
			if (acc->GetAccountID () == accID)
				return acc->GetQObject ();

		return nullptr;
	}

	QList<QObject*> ProxyObject::GetAllAccounts () const
	{
		return Util::Map (Core::Instance ().GetAccounts (), &IAccount::GetQObject);
	}

	QObject* ProxyObject::GetEntry (const QString& entryID, const QString&) const
	{
		return Core::Instance ().GetEntry (entryID);
	}

	void ProxyObject::OpenChat (const QString& entryID, const QString& accID,
			const QString& message, const QString& variant) const
	{
		const auto mgr = Core::Instance ().GetChatTabsManager ();

		const auto entry = qobject_cast<ICLEntry*> (GetEntry (entryID, accID));
		const auto chat = mgr->OpenChat (entry, true);

		chat->insertMessageText (message);
		chat->selectVariant (variant);
	}

	QWidget* ProxyObject::FindOpenedChat (const QString& entryID, const QByteArray&) const
	{
		const auto mgr = Core::Instance ().GetChatTabsManager ();
		return mgr->GetChatTab (entryID);
	}

	Util::ResourceLoader* ProxyObject::GetResourceLoader (IProxyObject::PublicResourceLoader loader) const
	{
		switch (loader)
		{
		case PRLClientIcons:
			return ResourcesManager::Instance ().GetResourceLoader (ResourcesManager::RLTClientIconLoader);
		case PRLStatusIcons:
			return ResourcesManager::Instance ().GetResourceLoader (ResourcesManager::RLTStatusIconLoader);
		case PRLSystemIcons:
			return ResourcesManager::Instance ().GetResourceLoader (ResourcesManager::RLTSystemIconLoader);
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown type"
				<< loader;

		Util::Unreachable ();
	}

	QIcon ProxyObject::GetIconForState (State state) const
	{
		return ResourcesManager::Instance ().GetIconForState (state);
	}

	QObject* ProxyObject::CreateCoreMessage (const QString& body,
			const QDateTime& date,
			IMessage::Type type, IMessage::Direction dir,
			QObject *other, QObject *parent)
	{
		return new CoreMessage (body, date, type, dir, other, parent);
	}

	QString ProxyObject::ToPlainBody (QString body)
	{
		body.replace ("<li>", "\n * ");
		auto pos = 0;
		while ((pos = body.indexOf ('<', pos)) != -1)
		{
			const auto endPos = body.indexOf ('>', pos + 1);
			body.remove (pos, endPos - pos + 1);
		}

		return body;
	}

	bool ProxyObject::IsMessageRead (QObject *msgObj)
	{
		return Core::Instance ().GetUnreadQueueManager ()->IsMessageRead (msgObj);
	}

	void ProxyObject::MarkMessagesAsRead (QObject *entryObj)
	{
		Core::Instance ().GetUnreadQueueManager ()->clearMessagesForEntry (entryObj);
	}

	QString ProxyObject::PrettyPrintDateTime (const QDateTime& dt)
	{
		return Azoth::PrettyPrintDateTime (dt);
	}

	std::optional<CustomStatus> ProxyObject::FindCustomStatus (const QString& name) const
	{
		const auto mgr = Core::Instance ().GetCustomStatusesManager ();
		const auto& statuses = mgr->GetStates ();

		const auto pos = std::find_if (statuses.begin (), statuses.end (),
				[&name] (const CustomStatus& status)
					{ return !QString::compare (status.Name_, name, Qt::CaseInsensitive); });
		if (pos == statuses.end ())
			return {};
		return *pos;
	}

	QStringList ProxyObject::GetCustomStatusNames () const
	{
		const auto mgr = Core::Instance ().GetCustomStatusesManager ();

		QStringList result;
		for (const auto& status : mgr->GetStates ())
			result << status.Name_;
		return result;
	}

	QImage ProxyObject::GetDefaultAvatar (int size) const
	{
		return ResourcesManager::Instance ().GetDefaultAvatar (size);
	}

	void ProxyObject::RedrawItem (QObject *entryObj) const
	{
		Core::Instance ().UpdateItem (entryObj);
	}

	QObject* ProxyObject::GetFirstUnreadMessage (QObject* entryObj) const
	{
		return Core::Instance ().GetUnreadQueueManager ()->GetFirstUnreadMessage (entryObj);
	}

	IFormatterProxyObject& ProxyObject::GetFormatterProxy ()
	{
		return Formatter_;
	}

	IAvatarsManager* ProxyObject::GetAvatarsManager ()
	{
		return AvatarsManager_;
	}
}
}
