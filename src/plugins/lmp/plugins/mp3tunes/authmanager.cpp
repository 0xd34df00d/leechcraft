/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/xpc/passutils.h>
#include "consts.h"

namespace LC::LMP::MP3Tunes
{
	Util::ContextTask<AuthManager::ResultType> AuthManager::GetSID (const QString& login)
	{
		using enum CloudStorageError;

		if (Login2Sid_.contains (login))
			co_return Login2Sid_ [login];

		co_await Util::AddContextObject { *this };

		const auto& pass = Util::GetPassword ("org.LeechCraft.LMP.MP3Tunes.Account." + login,
				tr ("Enter password for MP3tunes account %1:").arg (login),
				GetProxyHolder (),
				!FailedAuth_.contains (login));
		if (pass.isEmpty ())
			co_return { Util::AsLeft, { NotAuthorized, tr ("empty password") } };

		const auto authUrl = "https://shop.mp3tunes.com/api/v1/login?output=xml&"
				"username=%1&password=%2&partner_token=%3"_qs
					.arg (login, pass, Consts::PartnerId);
		const auto response = co_await *GetProxyHolder ()->GetNetworkAccessManager ()->get (QNetworkRequest (authUrl));
		if (const auto err = response.IsError ())
			co_return { Util::AsLeft, { NetError, tr ("network error: %1").arg (err->ErrorText_) } };

		QDomDocument doc;
		if (!doc.setContent (response.GetReplyData ()))
			co_return { Util::AsLeft, { NetError, tr ("failed to parse response") } };

		const auto& docElem = doc.documentElement ();
		if (docElem.firstChildElement ("status"_qs).text () != "1"_qs)
		{
			FailedAuth_ << login;
			const auto& errorText = docElem.firstChildElement ("errorMessage"_qs).text ();
			co_return { Util::AsLeft, { NotAuthorized, tr ("authentication error: %1").arg (errorText) } };
		}

		FailedAuth_.remove (login);

		const auto& sid = docElem.firstChildElement ("session_id"_qs).text ();
		Login2Sid_ [login] = sid;

		co_return sid;
	}
}
