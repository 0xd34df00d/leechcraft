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
#include <boost/spirit/include/classic_utility.hpp>
#include <QUrl>
#include <QtDebug>

namespace LC::Azoth::Acetamide
{
	template<auto Val>
	auto Assign (auto& ref)
	{
		static constexpr auto val = Val;
		return boost::spirit::classic::assign_a (ref, val);
	}

	std::optional<DecodedUrl> DecodeUrl (const QUrl& url)
	{
		std::string host_;
		int port_ = 0;
		bool serverPass = false;

		std::string channel_;
		bool channelPass = false;

		std::string hostmask_;
		std::string user_server_;
		std::string user_;
		std::string nick_;

		enum class TargetType
		{
			None,
			Channel,
			NickOnly,
			NickInfo,
			UserInfo,
		} targetType = TargetType::None;

		using namespace boost::spirit::classic;

		range<> ascii { char { 0x01 }, char { 0x7F } };
		rule<> special = chset_p ("[]\\`^{|}-");
		rule<> let_dig_hyp = alnum_p | ch_p ('-');

		rule<> ldh_str;
		ldh_str = *(let_dig_hyp >> !ldh_str);

		rule<> label = alpha_p >> !(!ldh_str >> alnum_p);
		rule<> subdomain = label >> +(label >> !ch_p ('.'));
		rule<> host = subdomain [assign_a (host_)];
		rule<> nonwhite = ascii - chset_p (" ,\r\n");

		rule<> servername = subdomain [assign_a (user_server_)];
		rule<> user = (+(nonwhite - '@')) [assign_a (user_)];
		rule<> nick = (alpha_p >> *(alnum_p | special)) [assign_a (nick_)];
		rule<> userinfo = user >> ch_p ('@') >> servername;
		rule<> hostmask = lexeme_d [+nonwhite] [assign_a (hostmask_)];
		rule<> nickinfo = nick >> ch_p ('!') >> user >> ch_p ('@') >> hostmask;

		rule<> nicktypes = (nickinfo [Assign<TargetType::NickInfo> (targetType)])
				| (userinfo [Assign<TargetType::UserInfo> (targetType)])
				| (nick [Assign<TargetType::NickOnly> (targetType)]);

		rule<> nicktrgt = nicktypes >> str_p (",isnick");

		rule<> channelstr = lexeme_d [!chset_p ("#&+") >> +nonwhite] [assign_a (channel_)];

		rule<> channeltrgt = (channelstr >> !(str_p (",needkey") [Assign<true> (channelPass)])) [Assign<TargetType::Channel> (targetType)];

		rule<> target = nicktrgt | channeltrgt;

		rule<> port = int_p[assign_a (port_)];
		rule<> uri = str_p ("irc:") >>
				!(str_p ("//") >>
						!(host >> !(ch_p (':') >> port))
						>> !ch_p ('/')
						>> !target >> !(str_p (",needpass") [Assign<true> (serverPass)]));

		if (!parse (url.toString ().toUtf8 ().constData (), uri).full)
			return {};

		ServerOptions so
		{
			.ServerName_ = QString::fromUtf8 (host_.c_str ()),
			.ServerPort_ = port_,
		};

		Target targetVar;
		switch (targetType)
		{
		case TargetType::None:
			targetVar = NoTarget {};
			break;
		case TargetType::Channel:
			targetVar = ChannelTarget
			{
				.Opts_ = ChannelOptions
				{
					.ServerName_ = so.ServerName_,
					.ChannelName_ = QString::fromUtf8 (channel_.c_str ()),
				},
				.HasPassword_ = channelPass,
			};
			break;
		case TargetType::NickOnly:
			targetVar = NickOnly { QString::fromStdString (nick_) };
			break;
		case TargetType::NickInfo:
			targetVar = NickInfo
			{
				.Nick_ = QString::fromStdString (nick_),
				.User_ = QString::fromStdString (user_),
				.HostMask_ = QString::fromStdString (hostmask_),
			};
			break;
		case TargetType::UserInfo:
			targetVar = UserInfo
			{
				.User_ = QString::fromStdString (user_),
				.ServerName_ = QString::fromStdString (user_server_),
			};
			break;
		}

		return DecodedUrl
		{
			.Server_ = so,
			.HasServerPassword_ = serverPass,
			.Target_ = std::move (targetVar),
		};
	}
}
