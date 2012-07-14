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

#include "actorbase.h"
#include <QNetworkAccessManager>
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
	ActorBase::ActorBase (Account *acc)
	: QObject (acc)
	, A_ (acc)
	, Mgr_ (new QNetworkAccessManager (this))
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
				SLOT (handleGotCookies (QList<QNetworkCookie>)),
				Qt::QueuedConnection);

		am->GetCookiesFor (acc->GetLogin (), acc->GetPassword ());

		emit statusChanged (tr ("Authenticating..."));
	}

	void ActorBase::handleGotCookies (const QList<QNetworkCookie>& cookies)
	{
		Mgr_->cookieJar ()->setCookiesFromUrl (cookies, UpURL);

		auto reply = MakeRequest ();
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void ActorBase::handleReplyFinished ()
	{
		qDebug () << Q_FUNC_INFO;
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		if (reply->error () != QNetworkReply::NoError)
		{
			emit gotError (tr ("Error: %1.")
					.arg (reply->errorString ()));
			emit finished ();
			return;
		}

		HandleReply (reply);
	}
}
}
}
