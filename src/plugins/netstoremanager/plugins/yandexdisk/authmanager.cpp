/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
