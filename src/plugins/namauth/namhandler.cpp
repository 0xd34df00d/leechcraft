/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "namhandler.h"
#include <QNetworkAccessManager>
#include <QFontMetrics>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QApplication>
#include "authenticationdialog.h"
#include "sqlstoragebackend.h"

namespace LC
{
namespace NamAuth
{
	NamHandler::NamHandler (SQLStorageBackend *sb, QNetworkAccessManager *nam)
	: QObject { nam }
	, SB_ { sb }
	, NAM_ { nam }
	{
		connect (nam,
				SIGNAL (authenticationRequired (QNetworkReply*, QAuthenticator*)),
				this,
				SLOT (handleAuthentication (QNetworkReply*, QAuthenticator*)));
		connect (nam,
				SIGNAL (proxyAuthenticationRequired (QNetworkProxy, QAuthenticator*)),
				this,
				SLOT (handleAuthentication (QNetworkProxy, QAuthenticator*)));
	}

	void NamHandler::DoCommonAuth (const QString& msg, const QString& context, QAuthenticator *authen)
	{
		const auto& realm = authen->realm ();

		auto suggestedUser = authen->user ();
		auto suggestedPassword = authen->password ();

		if (suggestedUser.isEmpty ())
			if (const auto& data = SB_->GetAuth (realm, context))
			{
				suggestedUser = data->Login_;
				suggestedPassword = data->Password_;
			}

		AuthenticationDialog dia (msg, suggestedUser, suggestedPassword, qApp->activeWindow ());
		if (dia.exec () == QDialog::Rejected)
			return;

		const auto& login = dia.GetLogin ();
		const auto& password = dia.GetPassword ();
		authen->setUser (login);
		authen->setPassword (password);

		if (dia.ShouldSave ())
			SB_->SetAuth ({ realm, context, login, password });
	}

	void NamHandler::handleAuthentication (QNetworkReply *reply,
			QAuthenticator *authen)
	{
		const auto& msg = tr ("%1 (%2) requires authentication.")
				.arg (authen->realm ())
				.arg ("<em>" + reply->url ().toString () + "</em>");

		DoCommonAuth (msg, reply->url ().host (), authen);
	}

	void NamHandler::handleAuthentication (const QNetworkProxy& proxy,
			QAuthenticator *authen)
	{
		const auto& msg = tr ("%1 (%2) requires authentication.")
				.arg (authen->realm ())
				.arg ("<em>" + proxy.hostName () + "</em>");

		DoCommonAuth (msg, proxy.hostName (), authen);
	}
}
}
