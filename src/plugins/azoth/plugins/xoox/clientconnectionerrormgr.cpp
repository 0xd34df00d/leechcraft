/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clientconnectionerrormgr.h"
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/network/socketerrorstrings.h>
#include "clientconnection.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	const int ErrorLimit = 5;

	ClientConnectionErrorMgr::ClientConnectionErrorMgr (ClientConnection *conn)
	: QObject (conn)
	, ClientConn_ (conn)
	, Client_ (ClientConn_->GetClient ())
	, SocketErrorAccumulator_ (0)
	{
		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleError (QXmppClient::Error)));

		QTimer *decrTimer = new QTimer (this);
		connect (decrTimer,
				SIGNAL (timeout ()),
				this,
				SLOT (decrementErrAccumulators ()));
		decrTimer->start (15000);
	}

	void ClientConnectionErrorMgr::Whitelist (const QString& id, bool add)
	{
		if (add && !id.isEmpty ())
			WhitelistedErrors_ << id;
	}

	void ClientConnectionErrorMgr::SetErrorHandler (const QString& id, const ErrorHandler_f& handler)
	{
		ErrorHandlers_ [id] = handler;
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
			ErrorHandlers_.remove (iq.id ());
			break;
		}
	}

	void ClientConnectionErrorMgr::HandleMessage (const QXmppMessage& msg)
	{
		if (msg.type () != QXmppMessage::Error)
			return;

		const auto& error = msg.error ();
		const auto& condStr = HandleErrorCondition (error.condition ());

		const auto& text = error.text ().isEmpty () ?
				tr ("Error from %1: %2")
					.arg (msg.from ())
					.arg (condStr) :
				tr ("Error from %1: %2 (%3).")
					.arg (msg.from ())
					.arg (condStr)
					.arg (error.text ());
		const auto& e = Util::MakeNotification ("Azoth",
				text,
				Priority::Critical);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	QString ClientConnectionErrorMgr::HandleErrorCondition (QXmppStanza::Error::Condition condition)
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
			return tr ("Not authorized.");
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
		case QXmppStanza::Error::Gone:
			return tr ("The user or server cannot be contacted at this address.");
		case QXmppStanza::Error::UndefinedCondition:
			return tr ("Undefined condition.");
		case QXmppStanza::Error::UnexpectedRequest:
			return tr ("Unexpected request.");
		case QXmppStanza::Error::PolicyViolation:
			return tr ("Local server policy violation.");
		case QXmppStanza::Error::NoCondition:
			return {};
		}

		qWarning () << "unknown condition:" << condition;
		return tr ("Other error.");
	}

	void ClientConnectionErrorMgr::HandleError (const QXmppIq& iq)
	{
		if (const auto handler = ErrorHandlers_.take (iq.id ()))
			if (handler (iq))
				return;

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
				Priority::Critical);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);

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

	void ClientConnectionErrorMgr::handleError (QXmppClient::Error error)
	{
		if (ClientConn_->GetLastState ().State_ == SOffline)
		{
			qDebug () << Q_FUNC_INFO
					<< "killing stale error in disconnected state"
					<< error;

			if (IsDisconnecting_)
				return;

			IsDisconnecting_ = true;
			Client_->disconnectFromServer ();
			IsDisconnecting_ = false;
			return;
		}

		QString str;
		switch (error)
		{
		case QXmppClient::SocketError:
			if (SocketErrorAccumulator_ < ErrorLimit)
			{
				++SocketErrorAccumulator_;
				str = tr ("socket error: %1.")
						.arg (Util::GetSocketErrorString (Client_->socketError ()));
			}
			break;
		case QXmppClient::KeepAliveError:
			str = tr ("keep-alive error.");
			break;
		case QXmppClient::XmppStreamError:
			str = tr ("error while connecting: ");
			str += HandleErrorCondition (Client_->xmppStreamError ());
			switch (Client_->xmppStreamError ())
			{
			case QXmppStanza::Error::NotAuthorized:
				emit serverAuthFailed ();
				break;
			default:
				break;
			}
			break;
		case QXmppClient::NoError:
			str = tr ("no error.");
			break;
		}

		if (str.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "suppressed"
					<< str
					<< error
					<< Client_->socketError ()
					<< Client_->xmppStreamError ();
			return;
		}

		const Entity& e = Util::MakeNotification ("Azoth",
				tr ("Account %1:").arg (ClientConn_->GetOurJID ()) +
					' ' + str,
				Priority::Critical);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void ClientConnectionErrorMgr::decrementErrAccumulators ()
	{
		if (SocketErrorAccumulator_ > 0)
			--SocketErrorAccumulator_;
	}
}
}
}
