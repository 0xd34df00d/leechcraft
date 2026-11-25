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
#include <util/sll/util.h>
#include <util/threads/coro/context.h>
#include <util/threads/coro/signal.h>
#include "authenticationdialog.h"

namespace LC::NamAuth
{
	NamHandler::NamHandler (SQLStorageBackend *sb, QNetworkAccessManager *nam)
	: QObject { nam }
	, SB_ { sb }
	{
		connect (nam,
				&QNetworkAccessManager::authenticationRequired,
				this,
				[this] (QNetworkReply *reply, QAuthenticator *auth)
				{
					DoCommonAuth (reply->url ().host (), { auth->user (), auth->password () }, auth);
				});
		connect (nam,
				&QNetworkAccessManager::proxyAuthenticationRequired,
				this,
				[this] (const QNetworkProxy& proxy, QAuthenticator *auth)
				{
					// Qt's SOCKS5 engine passes a blank authenticator every time and keeps what it tried in the proxy.
					const auto& refused = proxy.type () == QNetworkProxy::Socks5Proxy ?
							Credentials { proxy.user (), proxy.password () } :
							Credentials { auth->user (), auth->password () };
					DoCommonAuth (proxy.hostName (), refused, auth);
				});
	}

	void NamHandler::DoCommonAuth (const QString& context, const Credentials& refused, QAuthenticator *authen)
	{
		const Site site { authen->realm (), context };

		if (const auto maybeKnown = GetCredentials (site);
			maybeKnown && (maybeKnown->Login_ != refused.Login_ || maybeKnown->Password_ != refused.Password_))
		{
			authen->setUser (maybeKnown->Login_);
			authen->setPassword (maybeKnown->Password_);
			return;
		}

		if (!OpenQuestions_.contains (site))
			Ask (site, refused.Login_, refused.Password_);
	}

	std::optional<SQLStorageBackend::AuthRecord> NamHandler::GetCredentials (const Site& site) const
	{
		if (const auto pos = SessionCredentials_.find (site);
			pos != SessionCredentials_.end ())
			return *pos;

		return SB_->GetAuth (site.Realm_, site.Context_);
	}

	Util::ContextTask<void> NamHandler::Ask (Site site, QString login, QString password)
	{
		co_await Util::AddContext { CoroContext_ };

		OpenQuestions_ << site;
		const auto guard = Util::MakeScopeGuard ([this, site] { OpenQuestions_.remove (site); });

		const auto& msg = tr ("%1 (%2) requires authentication.")
				.arg (site.Realm_.toHtmlEscaped (), "<em>" + site.Context_.toHtmlEscaped () + "</em>");
		AuthenticationDialog dia { msg, login, password };
		dia.show ();
		co_await Util::Signal { dia, &QDialog::finished };

		if (dia.result () != QDialog::Accepted)
			co_return;

		const SQLStorageBackend::AuthRecord record { site.Realm_, site.Context_, dia.GetLogin (), dia.GetPassword () };
		SessionCredentials_ [site] = record;
		if (dia.ShouldSave ())
			SB_->SetAuth (record);
	}
}
