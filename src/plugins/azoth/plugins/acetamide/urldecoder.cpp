/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urldecoder.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <QUrl>
#include <QtDebug>

namespace LC::Azoth::Acetamide
{
	std::optional<DecodedUrl> DecodeUrl (const QUrl& url)
	{
		std::string host_;
		int port_ = 0;
		std::string channel_;
		std::string nick_;
		bool isNick = false;
		bool serverPass = false;
		bool channelPass = false;

		using namespace boost::spirit::classic;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'^' | '{' | '|' | '}' | '-'];
		rule<> hostmask = lexeme_d [+(ascii - ' ' - '\0' - ',' - '\r' - '\n')];
		rule<> let_dig_hyp = alnum_p | ch_p ('-');

		rule<> ldh_str;
		ldh_str = *(let_dig_hyp >> !ldh_str);

		rule<> label = alpha_p >> !(!ldh_str >> alnum_p);
		rule<> subdomain = label >> +(label >> !ch_p ('.'));
		rule<> host = subdomain  [assign_a (host_)];
		rule<> servername = host;
		rule<> user = +(ascii - ' ' - '\0' - '\r' - '\n');
		rule<> nick = alpha_p >> *(alnum_p | special);
		rule<> userinfo = user >> ch_p ('@') >> servername;
		rule<> nickinfo = nick >> ch_p ('!')
				>> user >> ch_p ('@')
				>> hostmask;

		rule<> nicktypes = (nick | nickinfo | userinfo)[assign_a (nick_)];

		rule<> nicktrgt = nicktypes >> ch_p (',') >> str_p ("isnick")[assign_a (isNick, true)];

		rule<> channelstr = lexeme_d [!(ch_p ('#') | ch_p ('&') | ch_p ('+')) >>
				+(ascii - ' ' - '\0' - ',' - '\r' - '\n')][assign_a (channel_)];

		rule<> keystr = lexeme_d [channelstr >> ch_p (',') >> str_p ("needkey")[assign_a (channelPass, true)]];

		rule<> channeltrgt = longest_d [channelstr | keystr];

		rule<> target = longest_d [nicktrgt | channeltrgt];

		rule<> port = int_p[assign_a (port_)];
		rule<> uri = str_p ("irc:") >>
				!(str_p ("//") >>
						!(host >> !(ch_p (':') >> port))
						>> !ch_p ('/')
						>> !target >> !(ch_p (',') >> str_p ("needpass")[assign_a (serverPass, true)]));

		bool res = parse (url.toString ().toUtf8 ().constData (), uri).full;
		if (!res)
		{
			qWarning () << "input string is not a valid IRC URI"
					<< url.toString ().toUtf8 ().constData ();
			return {};
		}

		if (isNick)
			return {};

		ServerOptions so;
		so.SSL_ = false;
		if (!host_.empty ())
			so.ServerName_ = QString::fromUtf8 (host_.c_str ());

		so.ServerPort_ = port_;

		ChannelOptions cho;
		if (!channel_.empty ())
			cho.ChannelName_ = QString::fromUtf8 (channel_.c_str ());
		cho.ServerName_ = so.ServerName_;

		return DecodedUrl
		{
			.Server_ = so,
			.Channel_ = cho,
			.HasServerPassword_ = serverPass,
			.HasChannelPassword_ = channelPass,
		};
	}
}
