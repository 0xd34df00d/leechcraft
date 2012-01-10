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

#include "simpleactor.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include "account.h"
#include "authmanager.h"
#include "urls.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	SimpleActor::SimpleActor (const QUrl& url, const QByteArray& post, Account *acc)
	: QObject (acc)
	, A_ (acc)
	, Mgr_ (new QNetworkAccessManager (this))
	, URL_ (url)
	, Post_ (post)
	{
		connect (this,
				SIGNAL (finished ()),
				this,
				SLOT (deleteLater ()),
				Qt::QueuedConnection);
		auto am = acc->GetAuthManager ();
		connect (am,
				SIGNAL (gotCookies (QList<QNetworkCookie>)),
				this,
				SLOT (handleGotCookies (QList<QNetworkCookie>)));

		am->GetCookiesFor (acc->GetLogin (), acc->GetPassword ());

		emit statusChanged (tr ("Authenticating..."));
	}

	void SimpleActor::handleGotCookies (const QList<QNetworkCookie>& cookies)
	{
		qDebug () << Q_FUNC_INFO;
		Mgr_->cookieJar ()->setCookiesFromUrl (cookies, UpURL);

		auto reply = Mgr_->post (A_->MakeRequest (URL_), Post_);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));

		emit statusChanged (tr ("Getting storage..."));
	}

	void SimpleActor::handleFinished ()
	{
		qDebug () << Q_FUNC_INFO;
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		if (reply->error () != QNetworkReply::NoError)
		{
			emit gotError (tr ("Error performing action: %1.")
					.arg (reply->errorString ()));
			emit finished ();
			return;
		}

		emit finished ();
	}
}
}
}
