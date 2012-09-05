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

#include "clientconnectionerrormgr.h"
#include <util/util.h>
#include "clientconnection.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	ClientConnectionErrorMgr::ClientConnectionErrorMgr (ClientConnection *conn)
	: QObject (conn)
	, ClientConn_ (conn)
	{
	}

	void ClientConnectionErrorMgr::Whitelist (const QString& id, bool add)
	{
		if (add && !id.isEmpty ())
			WhitelistedErrors_ << id;
	}

	void ClientConnectionErrorMgr::HandleIq (const QXmppIq& iq)
	{
		switch (iq.error ().type ())
		{
		case QXmppStanza::Error::Cancel:
		case QXmppStanza::Error::Continue:
		case QXmppStanza::Error::Modify:
		case QXmppStanza::Error::Auth:
		case QXmppStanza::Error::Wait:
			HandleError (iq);
			break;
		default:
			WhitelistedErrors_.remove (iq.id ());
			break;
		}
	}

	QString ClientConnectionErrorMgr::HandleErrorCondition (const QXmppStanza::Error::Condition& condition)
	{
		switch (condition)
		{
		case QXmppStanza::Error::BadRequest:
			return tr ("Bad request.");
		case QXmppStanza::Error::Conflict:
			return tr ("Conflict (possibly, resource conflict).");
		case QXmppStanza::Error::FeatureNotImplemented:
			return tr ("Feature not implemented.");
		case QXmppStanza::Error::Forbidden:
			return tr ("Forbidden.");
			//case QXmppStanza::Error::Gone:
		case QXmppStanza::Error::InternalServerError:
			return tr ("Internal server error.");
		case QXmppStanza::Error::ItemNotFound:
			return tr ("Item not found.");
		case QXmppStanza::Error::JidMalformed:
			return tr ("JID is malformed.");
		case QXmppStanza::Error::NotAcceptable:
			return tr ("Data is not acceptable.");
		case QXmppStanza::Error::NotAllowed:
			return tr ("Action is not allowed.");
		case QXmppStanza::Error::NotAuthorized:
			emit serverAuthFailed ();
			return tr ("Not authorized.");
		case QXmppStanza::Error::PaymentRequired:
			return tr ("Payment required.");
		case QXmppStanza::Error::RecipientUnavailable:
			return tr ("Recipient unavailable.");
		case QXmppStanza::Error::Redirect:
			return tr ("Got redirect.");
		case QXmppStanza::Error::RegistrationRequired:
			return tr ("Registration required.");
		case QXmppStanza::Error::RemoteServerNotFound:
			return tr ("Remote server not found.");
		case QXmppStanza::Error::RemoteServerTimeout:
			return tr ("Timeout contacting remote server.");
		case QXmppStanza::Error::ResourceConstraint:
			return tr ("Error due to resource constraint.");
		case QXmppStanza::Error::ServiceUnavailable:
			return tr ("Service is unavailable at the moment.");
		case QXmppStanza::Error::SubscriptionRequired:
			return tr ("Subscription is required to perform this action.");
			//case QXmppStanza::Error::UndefinedCondition:
			//case QXmppStanza::Error::UnexpectedRequest:
		default:
			return tr ("Other error.");
		}
	}

	void ClientConnectionErrorMgr::HandleError (const QXmppIq& iq)
	{
		const QXmppStanza::Error& error = iq.error ();
		if (!WhitelistedErrors_.remove (iq.id ()))
			switch (error.condition ())
			{
			case QXmppStanza::Error::FeatureNotImplemented:
			case QXmppStanza::Error::ItemNotFound:
			case QXmppStanza::Error::ServiceUnavailable:
				return;
			default:
				break;
			}

		QString typeText;
		if (!iq.from ().isEmpty ())
			typeText = tr ("Error from %1: ")
					.arg (iq.from ());
		typeText += HandleErrorCondition (error.condition ());

		if (!error.text ().isEmpty ())
			typeText += " " + tr ("Error text: %1.")
					.arg (error.text ());

		const Entity& e = Util::MakeNotification ("Azoth",
				typeText,
				PCritical_);
		Core::Instance ().SendEntity (e);

		const bool dontTryFurther = error.type () == QXmppStanza::Error::Cancel ||
			(error.type () == QXmppStanza::Error::Auth &&
			 error.condition () != QXmppStanza::Error::NotAuthorized);
		if (dontTryFurther && !ClientConn_->GetClient ()->isConnected ())
		{
			GlooxAccountState state =
			{
				SOffline,
				QString (),
				0
			};
			ClientConn_->SetState (state);
		}
	}
}
}
}
