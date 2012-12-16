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
#include <QWebView>
#include <util/customcookiejar.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace TouchStreams
{
	const QUrl AuthURL = QUrl::fromEncoded ("https://oauth.vk.com/authorize?client_id=3298289&redirect_uri=http%3A%2F%2Foauth.vk.com%2Fblank.html&response_type=token&scope=8&state=");
	AuthManager::AuthManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, AuthNAM_ (new QNetworkAccessManager (this))
	, Cookies_ (new Util::CustomCookieJar)
	, ValidFor_ (0)
	, IsRequesting_ (false)
	{
		AuthNAM_->setCookieJar (Cookies_);

		const auto& cookies = XmlSettingsManager::Instance ()
				.property ("Cookies").toByteArray ();
		if (!cookies.isEmpty ())
			Cookies_->Load (cookies);
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

	void AuthManager::Reauth ()
	{
		auto view = new QWebView;
		view->setWindowTitle (tr ("VK.com authentication"));
		view->setWindowFlags (Qt::Window);
		view->resize (800, 600);
		view->page ()->setNetworkAccessManager (AuthNAM_);
		view->show ();

		view->setUrl (AuthURL);

		connect (view,
				SIGNAL (urlChanged (QUrl)),
				this,
				SLOT (handleViewUrlChanged (QUrl)));
	}

	void AuthManager::HandleError ()
	{
		IsRequesting_ = false;
	}

	void AuthManager::RequestURL (const QUrl& url)
	{
		qDebug () << Q_FUNC_INFO << url;
		auto reply = AuthNAM_->get (QNetworkRequest (url));
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

		RequestURL (AuthURL);
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
		if (location.isEmpty ())
		{
			Reauth ();
			return;
		}

		if (CheckIsBlank (location))
			return;

		RequestURL (location);
	}

	void AuthManager::handleFormFetchError ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		qWarning () << Q_FUNC_INFO
				<< reply->errorString ();

		HandleError ();
	}

	void AuthManager::handleViewUrlChanged (const QUrl& url)
	{
		if (!CheckIsBlank (url))
			return;

		XmlSettingsManager::Instance ().setProperty ("Cookies", Cookies_->Save ());
		sender ()->deleteLater ();
	}
}
}
