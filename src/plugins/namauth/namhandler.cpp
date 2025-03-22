/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "namhandler.h"
#include <QNetworkAccessManager>
#include <QAuthenticator>
#include <QNetworkReply>
#include <QNetworkProxy>
#include "authenticationdialog.h"
#include "sqlstoragebackend.h"

namespace LC::NamAuth
{
	NamHandler::NamHandler (SQLStorageBackend *sb, QNetworkAccessManager *nam)
	: QObject { nam }
	, SB_ { sb }
	, NAM_ { nam }
	{
		connect (nam,
				&QNetworkAccessManager::authenticationRequired,
				this,
				[this] (QNetworkReply *reply, QAuthenticator *auth)
				{
					DoCommonAuth (reply->url ().host (), auth);
				});
		connect (nam,
				&QNetworkAccessManager::proxyAuthenticationRequired,
				this,
				[this] (const QNetworkProxy& proxy, QAuthenticator *auth)
				{
					DoCommonAuth (proxy.hostName (), auth);
				});
	}

	void NamHandler::DoCommonAuth (const QString& context, QAuthenticator *authen) const
	{
		const auto& realm = authen->realm ();
		const auto& msg = tr ("%1 (%2) requires authentication.")
				.arg (realm, "<em>" + context + "</em>");

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
}
