/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authenticator.h"
#include <QNetworkAccessManager>
#include <QCryptographicHash>
#include <QDomDocument>
#include <QtDebug>
#include <ws.h>
#include <util/xpc/passutils.h>
#include <util/gui/util.h>
#include "xmlsettingsmanager.h"
#include "codes.h"

namespace LC
{
namespace Lastfmscrobble
{
	namespace
	{
		const QString ScrobblingSite = "https://ws.audioscrobbler.com/2.0/";

		QString AuthToken (const QString& username, const QString& password)
		{
			const QString& passHash = QCryptographicHash::hash (password.toLatin1 (),
					QCryptographicHash::Md5).toHex ();
			return QCryptographicHash::hash ((username + passHash).toLatin1 (),
					QCryptographicHash::Md5).toHex ();
		}

		QString ApiSig (const QString& api_key, const QString& authToken,
				const QString& method, const QString& username,
				const QString& secret)
		{
			const QString& str = QString ("api_key%1authToken%2method%3username%4%5")
					.arg (api_key)
					.arg (authToken)
					.arg (method)
					.arg (username)
					.arg (secret);
			return QCryptographicHash::hash (str.toLatin1 (),
					QCryptographicHash::Md5).toHex ();
		}
	}

	Authenticator::Authenticator (QNetworkAccessManager *nam,
			const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Proxy_ (proxy)
	, IsAuthenticated_ (false)
	{
	}

	void Authenticator::Init ()
	{
		XmlSettingsManager::Instance ().RegisterObject ("lastfm.login",
				this, "handleAuth");
		handleAuth ();
	}

	bool Authenticator::IsAuthenticated () const
	{
		return IsAuthenticated_;
	}

	void Authenticator::FeedPassword (bool authFailure)
	{
		const QString& login = XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ();
		lastfm::ws::Username = login;

		if (login.isEmpty ())
			return;

		const auto& text = tr ("Enter password for Last.fm account %1:")
					.arg (Util::FormatName (login));
		const auto& password = Util::GetPassword ("org.LeechCraft.Lastfmscrobble/" + login,
				text,
				Proxy_,
				!authFailure);
		if (password.isEmpty ())
			return;

		const QString& authToken = AuthToken (lastfm::ws::Username, password);

		const QString& api_sig = ApiSig (lastfm::ws::ApiKey, authToken,
				"auth.getMobileSession", lastfm::ws::Username,
				lastfm::ws::SharedSecret);
		const QString& url = QString ("%1?method=%2&username=%3&authToken=%4&api_key=%5&api_sig=%6")
				.arg (ScrobblingSite)
				.arg ("auth.getMobileSession")
				.arg (lastfm::ws::Username)
				.arg (authToken)
				.arg (lastfm::ws::ApiKey)
				.arg (api_sig);

		QNetworkReply *reply = NAM_->get (QNetworkRequest (QUrl (url)));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (getSessionKey ()));
	}

	bool Authenticator::CheckError (const QDomDocument& doc)
	{
		auto sub = doc.documentElement ().firstChildElement ("error");
		if (sub.isNull ())
			return false;

		const int code = sub.attribute ("code").toInt ();
		switch (code)
		{
		case ErrorCodes::AuthError:
			FeedPassword (true);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown error code"
					<< code;
			break;
		}

		return true;
	}

	void Authenticator::getSessionKey ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();
		QDomDocument doc;
		doc.setContent (QString::fromUtf8 (reply->readAll ()));
		if (CheckError (doc))
			return;

		const auto& domList = doc.documentElement ().elementsByTagName ("key");
		if (!domList.size ())
			return;

		IsAuthenticated_ = true;

		lastfm::ws::SessionKey = domList.at (0).toElement ().text ();
		emit authenticated ();
	}

	void Authenticator::handleAuth ()
	{
		IsAuthenticated_ = false;
		FeedPassword (false);
	}
}
}
