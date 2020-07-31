/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <util/xpc/passutils.h>
#include "consts.h"

namespace LC
{
namespace LMP
{
namespace MP3Tunes
{
	AuthManager::AuthManager (QNetworkAccessManager *nam, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Proxy_ (proxy)
	{
	}

	QString AuthManager::GetSID (const QString& login)
	{
		if (Login2Sid_.contains (login))
			return Login2Sid_ [login];

		const auto& pass = Util::GetPassword ("org.LeechCraft.LMP.MP3Tunes.Account." + login,
				tr ("Enter password for MP3tunes account %1:").arg (login),
				Proxy_,
				!FailedAuth_.contains (login));
		if (pass.isEmpty ())
		{
			emit sidError (login, tr ("Empty password."));
			return QString ();
		}

		const auto authUrl = QString ("https://shop.mp3tunes.com/api/v1/login?output=xml&"
				"username=%1&password=%2&partner_token=%3")
					.arg (login)
					.arg (pass)
					.arg (Consts::PartnerId);
		auto reply = NAM_->get (QNetworkRequest (authUrl));
		reply->setProperty ("Login", login);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAuthReplyFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleAuthReplyError ()));

		return QString ();
	}

	void AuthManager::handleAuthReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& login = reply->property ("Login").toString ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			emit sidError (login, tr ("Unable to parse authentication reply."));
			return;
		}

		const auto& docElem = doc.documentElement ();
		if (docElem.firstChildElement ("status").text () != "1")
		{
			FailedAuth_ << login;
			emit sidError (login, docElem.firstChildElement ("errorMessage").text ());
			return;
		}

		Login2Sid_ [login] = docElem.firstChildElement ("session_id").text ();
		FailedAuth_.remove (login);

		emit sidReady (login);
	}

	void AuthManager::handleAuthReplyError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		emit sidError (reply->property ("Login").toString (),
				tr ("Unable to parse authentication reply."));
	}
}
}
}
