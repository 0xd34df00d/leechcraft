/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "authmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <util/passutils.h>
#include "consts.h"

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	AuthManager::AuthManager (QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	{
	}

	QString AuthManager::GetSID (const QString& login)
	{
		if (Login2Sid_.contains (login))
			return Login2Sid_ [login];

		const auto& pass = Util::GetPassword ("org.LeechCraft.LMP.MP3Tunes.Account." + login,
				tr ("Enter password for MP3tunes account %1:").arg (login),
				this,
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
