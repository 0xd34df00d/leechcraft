/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ljutils.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QNetworkAccessManager>
#include <util/sll/domchildrenrange.h>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/networkresult.h>

namespace LC::Util::LJ
{
	namespace
	{
		QByteArray GetChallengeRequestBody ()
		{
			return R"(<?xml version="1.0"?>
<methodCall>
  <methodName>LJ.XMLRPC.getchallenge</methodName>
</methodCall>
)";
		}

		std::optional<QString> GetChallenge (const QDomDocument& doc)
		{
			const auto& replyStruct = doc.documentElement ()
					.firstChildElement ("params"_qs)
					.firstChildElement ("param"_qs)
					.firstChildElement ("value"_qs)
					.firstChildElement ("struct"_qs);
			for (const auto& member : Util::DomChildren (replyStruct, "member"_qs))
				if (member.firstChildElement ("name"_qs).text () == "challenge")
					return member
							.firstChildElement ("value"_qs)
							.firstChildElement ("string"_qs)
							.text ();

			return {};
		}

		struct Tr
		{
			Q_DECLARE_TR_FUNCTIONS ("LC::Util::LJ")
		};
	}

	Task<RequestChallengeResult> RequestChallenge (RequestChallengeConfig config)
	{
		QNetworkRequest request { QUrl { "http://www.livejournal.com/interface/xmlrpc"_qs } };
		request.setRawHeader ("User-Agent", config.UserAgent_);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "text/xml");

		const auto reply = config.NAM_.post (request, GetChallengeRequestBody ());
		const auto response = co_await *reply;
		if (const auto err = response.IsError ())
		{
			qWarning () << *err;
			co_return RequestChallengeResult::Left ({ Tr::tr ("Network error: %1").arg (err->ErrorText_) });
		}

		const auto& data = response.GetReplyData ();

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << "failed to parse response from" << data;
			co_return RequestChallengeResult::Left ({ Tr::tr ("Failed to parse response") });
		}

		const auto& challenge = GetChallenge (doc);
		if (!challenge)
		{
			qWarning () << "failed to get challenge from\n" << doc.toByteArray (1).constData ();
			co_return RequestChallengeResult::Left ({ Tr::tr ("Failed to parse response") });
		}

		co_return RequestChallengeResult::Right (*challenge);
	}
}
