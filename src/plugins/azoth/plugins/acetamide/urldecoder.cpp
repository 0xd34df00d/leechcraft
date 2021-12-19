/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urldecoder.h"
#include <QRegularExpression>
#include <QUrl>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Azoth::Acetamide
{
	namespace
	{
		QRegularExpression MkRx ()
		{
			auto with = [] (QByteArray str, std::initializer_list<QPair<QByteArray, QByteArray>> subs)
			{
				for (const auto& [key, value] : subs)
					str.replace (key, value);
				return str;
			};

			const QByteArray subdomain { R"( [a-zA-Z]\w+ )" };

			const auto host = with (R"( {subdomain}(\.{subdomain})* )", { { "{subdomain}", subdomain } });
			const auto authority = with (R"( (?<host>{host})(:(?<port>\d+))? )", { {  "{host}", host } });

			const auto nonwhite = "[^ ,\r\n@]";

			const auto nick = R"( (\w | [\[\]\`^{|}-])+ )";

			const auto userinfo = with (R"( (?<userinfo_user>{nonwhite}+)@(?<userinfo_servername>{host}) )",
					{
						{ "{nonwhite}", nonwhite },
						{ "{host}", host },
					});
			const auto nickinfo = with (R"( (?<nickinfo_nick>{nick})!(?<nickinfo_user>({nonwhite}+))@(?<nickinfo_hostmask>{nonwhite}+) )",
					{
						{ "{nick}", nick },
						{ "{nonwhite}", nonwhite },
					});
			const auto nickonly = with (R"( (?<nickonly_nick>{nick}) )", { { "{nick}", nick } });

			const auto target = with (R"(
						(
							(?<nickinfo>{nickinfo})
							| (?<userinfo>{userinfo})
							| (?<nickonly>{nickonly})
						),isnick
						| ((?<channel>{nonwhite}+)(?<needkey>,needkey)?)
					)",
					{
						{ "{nickinfo}", nickinfo },
						{ "{userinfo}", userinfo },
						{ "{nickonly}", nickonly },
						{ "{nonwhite}", nonwhite },
					});

			auto pat = with (R"(
					irc:(//)?
						({authority})?
						/?
						({target})?
						(?<needpass>,needpass)?
					)",
					{
						{ "{authority}", authority },
						{ "{target}", target },
					});

			return QRegularExpression
			{
				QString { pat }.remove ('\n').remove ('\t').remove (' '),
				QRegularExpression::DontCaptureOption
			};
		}
	}

	std::optional<DecodedUrl> DecodeUrl (const QUrl& url)
	{
		static const auto rx = MkRx ();
		const auto& matchRes = rx.match (url.toString ());
		if (!matchRes.hasMatch ())
			return {};

		ServerOptions so
		{
			.ServerName_ = matchRes.captured (u"host"_qsv),
			.ServerPort_ = matchRes.capturedView (u"port"_qsv).toInt (),
		};

		Target targetVar;
		if (const auto& channel = matchRes.captured (u"channel"_qsv);
			!channel.isEmpty ())
			targetVar = ChannelTarget
			{
				.Opts_ = ChannelOptions
				{
					.ServerName_ = so.ServerName_,
					.ChannelName_ = channel,
				},
				.HasPassword_ = matchRes.capturedLength (u"needkey") > 0,
			};
		else if (matchRes.capturedLength (u"nickonly"_qsv) > 0)
			targetVar = NickOnly { matchRes.captured (u"nickonly_nick"_qsv) };
		else if (matchRes.capturedLength (u"nickinfo"_qsv) > 0)
			targetVar = NickInfo
			{
				.Nick_ = matchRes.captured (u"nickinfo_nick"),
				.User_ = matchRes.captured (u"nickinfo_user"),
				.HostMask_ = matchRes.captured (u"nickinfo_hostmask"),
			};
		else if (matchRes.capturedLength (u"userinfo") > 0)
			targetVar = UserInfo
			{
				.User_ = matchRes.captured (u"userinfo_user"),
				.ServerName_ = matchRes.captured (u"userinfo_servername"),
			};
		else
			targetVar = NoTarget {};

		return DecodedUrl
		{
			.Server_ = so,
			.HasServerPassword_ = matchRes.capturedLength (u"needpass"_qsv) > 0,
			.Target_ = std::move (targetVar),
		};
	}
}
