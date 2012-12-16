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

#include "authmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>

namespace LeechCraft
{
namespace TouchStreams
{
	AuthManager::AuthManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, ValidFor_ (0)
	, IsRequesting_ (false)
	{
	}

	void AuthManager::GetAuthKey ()
	{
		if (Token_.isEmpty () ||
				ReceivedAt_.secsTo (QDateTime::currentDateTime ()) > ValidFor_)
		{
			RequestAuthKey ();
			return;
		}

		emit gotAuthKey (Token_);
	}

	void AuthManager::HandleError ()
	{
		IsRequesting_ = false;
	}

	void AuthManager::RequestURL (const QUrl& url)
	{
		qDebug () << Q_FUNC_INFO << url;
		auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotForm ()));
		connect (reply,
				SIGNAL(error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleFormFetchError ()));
	}

	void AuthManager::RequestAuthKey ()
	{
		if (IsRequesting_)
			return;

		const auto& url = QUrl::fromEncoded ("https://oauth.vk.com/authorize?client_id=3298289&redirect_uri=http%3A%2F%2Foauth.vk.com%2Fblank.html&response_type=token&scope=8&state=&display=wap");
		RequestURL (url);
	}

	bool AuthManager::CheckIsBlank (QUrl location)
	{
		if (location.path () != "/blank.html")
			return false;

		location = QUrl::fromEncoded (location.toEncoded ().replace ('#', '?'));
		Token_ = location.queryItemValue ("access_token");
		ValidFor_ = location.queryItemValue ("expires_in").toInt ();
		IsRequesting_ = false;
		return true;
	}

	void AuthManager::handleGotForm ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& location = reply->header (QNetworkRequest::LocationHeader).toUrl ();
		if (!location.isEmpty ())
		{
			if (CheckIsBlank (location))
				return;

			RequestURL (location);
			return;
		}

		const auto& data = reply->readAll ();
		const int formPos = data.indexOf ("<form");
		const int urlPos = data.indexOf ("action=", formPos) + QByteArray ("action=\"").size ();
		const int end = data.indexOf ('"', urlPos + 1);
		const auto url = QUrl::fromEncoded (data.mid (urlPos, end - urlPos));

		reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (url));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotForm ()));
		connect (reply,
				SIGNAL(error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleFormFetchError ()));
	}

	void AuthManager::handleFormFetchError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		HandleError ();
	}
}
}
