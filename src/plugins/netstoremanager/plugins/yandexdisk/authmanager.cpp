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
#include <algorithm>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>
#include <QtDebug>
#include "account.h"
#include "urls.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	AuthManager::AuthManager (Account *acc)
	: QObject (acc)
	, A_ (acc)
	, Mgr_ (new QNetworkAccessManager (this))
	, RecurCount_ (0)
	{
	}

	void AuthManager::GetCookiesFor (const QString& login, const QString& pass, bool clear)
	{
		qDebug () << Q_FUNC_INFO << login;
		RecurCount_ = 0;
		CurLogin_ = login;
		CurPass_ = pass;

		Q_FOREACH (QNetworkReply *reply, PendingReplies_)
		{
			reply->abort ();
			delete reply;
		}
		PendingReplies_.clear ();

		if (clear)
			Cookies_.remove (qMakePair (login, pass));

		GetCookiesForImpl (login, pass, QString ());
	}

	void AuthManager::GetCookiesForImpl (const QString& login,
			const QString& pass, const QString& captcha)
	{
		qDebug () << Q_FUNC_INFO << login << captcha << RecurCount_;

		auto pair = qMakePair (login, pass);
		if (Cookies_.contains (pair))
		{
			emit gotCookies (Cookies_ [pair]);
			return;
		}

		if (!captcha.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "captcha support isn't implemented yet";
		}

		QByteArray post = "login=" + login.toUtf8 () + "&passwd=" + pass.toUtf8 ();
		post += "&twoweeks=yes";

		auto reply = Mgr_->post (A_->MakeRequest (AuthURL), post);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		PendingReplies_ << reply;
	}

	void AuthManager::handleFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		PendingReplies_.remove (reply);

		qDebug () << Q_FUNC_INFO << reply->url () << reply->rawHeaderList ();

		const QString& cookHdr = reply->rawHeader ("Set-Cookie");
		if (cookHdr.isEmpty ())
		{
			emit gotError (tr ("Unexpected server reply."));
			return;
		}

		const auto& cookies = Mgr_->cookieJar ()->cookiesForUrl (UpURL);
		auto pos = std::find_if (cookies.begin (), cookies.end (),
				[] (decltype (cookies.front ()) c)
					{ return c.name () == "yandex_login" && !c.value ().isEmpty (); });
		if (pos != cookies.end ())
		{
			Cookies_ [qMakePair (CurLogin_, CurPass_)] = cookies;
			emit gotCookies (cookies);
			return;
		}

		QRegExp rx ("<input type=\"?submit\"?[^>]+name=\"no\"");
		const QString& page = reply->readAll ();
		if (rx.indexIn (page) > 0)
		{
			rx.setPattern ("<input type=\"hidden\" name=\"idkey\" value=\"(\\S+)\"[^>]*>");
			if (rx.indexIn (page) > 0)
			{
				const QByteArray& post = "idkey=" + rx.cap (1).toAscii () + "&filled=yes";
				auto newRep = Mgr_->post (A_->MakeRequest (AuthURL), post);
				connect (newRep,
						SIGNAL (finished ()),
						this,
						SLOT (handleFinished ()));
			}
			else
			{
				rx.setPattern ("<input type=\"hidden\" name=\"idkey\" value=\"(\\S+)\" />");
				if (RecurCount_++ < 3 && rx.indexIn (page) > 0)
					GetCookiesForImpl (CurLogin_, CurPass_, rx.cap (1));
			}
		}
	}
}
}
}
