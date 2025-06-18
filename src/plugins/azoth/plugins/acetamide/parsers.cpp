/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "parsers.h"
#include <QRegularExpression>
#include <QUrl>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>

namespace LC::Azoth::Acetamide
{
	namespace
	{
		QByteArray With (QByteArray str, std::initializer_list<QPair<QByteArray, QByteArray>> subs)
		{
			for (const auto& [key, value] : subs)
				str.replace (key, value);
			return str;
		}

		QString Fixup (const QByteArray& pat)
		{
			return QString { pat }.remove ('\n').remove ('\t').remove (' ');
		}

		// TODO C++20 replace with constexpr std::strings when they become more widely available
		const auto host = With (R"( {subdomain}(\.{subdomain})* )", { { "{subdomain}", R"( [a-zA-Z][-\w]+ )" } });

		constexpr auto nick = R"( (\w | [\[\]\`^{|}-])+ )";

		constexpr auto nonwhite = "[^ ,\r\n@]";

		auto ParseMessagePrefix (QStringView str)
		{
			struct Result
			{
				QString Nick_;
				QString Username_;
				QString Host_;
			};

			const auto pat = With (R"(
						(?<host>{host})
						| ((?<nick>{nick})(!(?<user>{nonwhite}+))?(@(?<servername>@{host}))?)
					)",
					{
						{ "{host}", host },
						{ "{nick}", nick },
						{ "{nonwhite}", nonwhite },
					});

			static const QRegularExpression rx
			{
				Fixup (pat),
				QRegularExpression::DontCaptureOption
			};

			const auto match = rx.matchView (str);

			const auto host = match.captured (u"host");
			return Result
			{
				.Nick_ = match.captured (u"nick"),
				.Username_ = match.captured (u"user"),
				.Host_ = host.isEmpty () ? match.captured (u"servername") : host,
			};
		}

		bool IsValidCommand (QStringView cmd)
		{
			if (cmd.size () == 3 && std::all_of (cmd.begin (), cmd.end (), [] (QChar ch) { return ch.isDigit (); }))
				return true;

			return std::all_of (cmd.begin (), cmd.end (), [] (QChar ch) { return ch.isLetter (); });
		}
	}

	std::optional<IrcMessageOptions> ParseMessage (QStringView msg)
	{
		msg = msg.trimmed ();
		const auto lastParamIdx = msg.indexOf (" :"_ql);
		const auto preMessage = msg.left (lastParamIdx);

		QStringView prefix;
		QStringView commandsStr;
		if (preMessage.startsWith (':'))
		{
			const auto& [left, right] = Util::BreakAt (preMessage, ' ');
			prefix = left;
			commandsStr = right;
		}
		else
			commandsStr = preMessage;

		const auto [command, params] = Util::BreakAt (commandsStr, ' ');

		if (!IsValidCommand (command))
			return {};

		auto split = preMessage.split (' ', Qt::SkipEmptyParts);
		if (split.isEmpty ())
			return {};

		const auto parsedPrefix = ParseMessagePrefix (prefix);

		return IrcMessageOptions
		{
			.Nick_ = !parsedPrefix.Nick_.isEmpty () ? parsedPrefix.Nick_ : parsedPrefix.Host_.section ('.', 0, 0),
			.UserName_ = parsedPrefix.Username_,
			.Host_ = parsedPrefix.Host_,
			.Command_ = command.toString ().toLower (),
			.Message_ = lastParamIdx > 0 ? msg.mid (lastParamIdx + 2).toString () : QString {},
			.Parameters_ = Util::MapAs<QList> (params.split (' ', Qt::SkipEmptyParts), &QStringView::toString),
		};
	}

	namespace
	{
		QRegularExpression MkUrlRx ()
		{
			const auto authority = With (R"( (?<host>{host})(:(?<port>\d+))? )", { {  "{host}", host } });

			const auto userinfo = With (R"( (?<userinfo_user>{nonwhite}+)@(?<userinfo_servername>{host}) )",
					{
						{ "{nonwhite}", nonwhite },
						{ "{host}", host },
					});
			const auto nickinfo = With (R"( (?<nickinfo_nick>{nick})!(?<nickinfo_user>({nonwhite}+))@(?<nickinfo_hostmask>{nonwhite}+) )",
					{
						{ "{nick}", nick },
						{ "{nonwhite}", nonwhite },
					});
			const auto nickonly = With (R"( (?<nickonly_nick>{nick}) )", { { "{nick}", nick } });

			const auto target = With (R"(
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

			const auto pat = With (R"(
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

			return QRegularExpression { Fixup (pat), QRegularExpression::DontCaptureOption };
		}
	}

	std::optional<DecodedUrl> DecodeUrl (const QUrl& url)
	{
		static const auto rx = MkUrlRx ();
		const auto& matchRes = rx.match (url.toString ());
		if (!matchRes.hasMatch ())
			return {};

		ServerOptions so
		{
			.ServerName_ = matchRes.captured (u"host"),
			.ServerPort_ = matchRes.capturedView (u"port").toInt (),
		};

		Target targetVar;
		if (const auto& channel = matchRes.captured (u"channel");
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
		else if (matchRes.capturedLength (u"nickonly") > 0)
			targetVar = NickOnly { matchRes.captured (u"nickonly_nick") };
		else if (matchRes.capturedLength (u"nickinfo") > 0)
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
			.HasServerPassword_ = matchRes.capturedLength (u"needpass") > 0,
			.Target_ = std::move (targetVar),
		};
	}
}
