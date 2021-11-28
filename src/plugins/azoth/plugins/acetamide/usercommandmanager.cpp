/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "usercommandmanager.h"
#include <QHash>
#include <util/sll/qtutil.h>
#include <util/sll/statichash.h>
#include "ircserverhandler.h"
#include "ircparser.h"
#include "ircaccount.h"

namespace LC::Azoth::Acetamide
{
	namespace
	{
		template<typename T>
		using CmdImpl_t = void (T::*) (const QStringList&);

		using Util::KVPair;
		using namespace std::string_view_literals;

		constexpr auto ServerHash = Util::MakeStringHash<CmdImpl_t<IrcServerHandler>> (
				KVPair { "privmsg"sv, &IrcServerHandler::SendMessage },
				KVPair { "msg"sv, &IrcServerHandler::SendMessage },
				KVPair { "say"sv, &IrcServerHandler::SayCommand },
				KVPair { "list"sv, &IrcServerHandler::showChannels }
			);
		constexpr auto ParserHash = Util::MakeStringHash<CmdImpl_t<IrcParser>> (
				KVPair { "join"sv, &IrcParser::JoinCommand },
				KVPair { "part"sv, &IrcParser::PartCommand },
				KVPair { "quit"sv, &IrcParser::QuitCommand },
				KVPair { "nick"sv, &IrcParser::NickCommand },
				KVPair { "ping"sv, &IrcParser::PingCommand },
				KVPair { "pong"sv, &IrcParser::PongCommand },
				KVPair { "topic"sv, &IrcParser::TopicCommand },
				KVPair { "kick"sv, &IrcParser::KickCommand },
				KVPair { "invite"sv, &IrcParser::InviteCommand },
				KVPair { "ctcp"sv, &IrcParser::CTCPRequest },
				KVPair { "names"sv, &IrcParser::NamesCommand },
				KVPair { "away"sv, &IrcParser::AwayCommand },
				KVPair { "userhost"sv, &IrcParser::UserhostCommand },
				KVPair { "ison"sv, &IrcParser::IsonCommand },
				KVPair { "whois"sv, &IrcParser::WhoisCommand },
				KVPair { "whowas"sv, &IrcParser::WhowasCommand },
				KVPair { "who"sv, &IrcParser::WhoCommand },
				KVPair { "summon"sv, &IrcParser::SummonCommand },
				KVPair { "version"sv, &IrcParser::VersionCommand },
				KVPair { "links"sv, &IrcParser::LinksCommand },
				KVPair { "info"sv, &IrcParser::InfoCommand },
				KVPair { "motd"sv, &IrcParser::MOTDCommand },
				KVPair { "time"sv, &IrcParser::TimeCommand },
				KVPair { "oper"sv, &IrcParser::OperCommand },
				KVPair { "rehash"sv, &IrcParser::RehashCommand },
				KVPair { "lusers"sv, &IrcParser::LusersCommand },
				KVPair { "users"sv, &IrcParser::UsersCommand },
				KVPair { "wallops"sv, &IrcParser::WallopsCommand },
				KVPair { "quote"sv, &IrcParser::RawCommand },
				KVPair { "me"sv, &IrcParser::CTCPRequest },
				KVPair { "squit"sv, &IrcParser::SQuitCommand },
				KVPair { "stats"sv, &IrcParser::StatsCommand },
				KVPair { "connect"sv, &IrcParser::ConnectCommand },
				KVPair { "trace"sv, &IrcParser::TraceCommand },
				KVPair { "admin"sv, &IrcParser::AdminCommand },
				KVPair { "kill"sv, &IrcParser::KillCommand },
				KVPair { "die"sv, &IrcParser::DieCommand },
				KVPair { "restart"sv, &IrcParser::RestartCommand },
				KVPair { "mode"sv, &IrcParser::ChanModeCommand }
			);
	}

	UserCommandManager::UserCommandManager (IrcServerHandler *ish,
			IrcParser *parser)
	: ISH_ { ish }
	, Parser_ { parser }
	{
	}

	QString UserCommandManager::VerifyMessage (const QString& msg, const QString& channelName) const
	{
		const int pos = msg.indexOf (' ');
		const auto cmd = (msg.startsWith ('/') ? msg.mid (1, pos) : msg.left (pos))
				.toUtf8 ()
				.trimmed ()
				.toLower ();

		auto serverCommand = ServerHash (Util::AsStringView (cmd));
		auto parserCommand = ParserHash (Util::AsStringView (cmd));
		if (!serverCommand && !parserCommand)
			return {};

		QString message;
		QStringList messageList;
		if (pos != -1)
		{
			message = msg.mid (pos).trimmed ();
			messageList = message.split (' ');
		}

		if (cmd == "me")
		{
			messageList.insert (0, channelName);
			messageList.insert (1, "ACTION");
		}
		else if (cmd == "part" && message.isEmpty ())
			messageList << channelName; //TODO message for part
		else if (cmd == "join" && !message.isEmpty ())
		{
			QStringList channelList = messageList.value (0).split (',');

			for (int i = 0; i < channelList.count (); ++i)
			{
				const QString& channel = channelList.at (i);
				if (!channel.startsWith ('#') &&
						!channel.startsWith ('+') &&
						!channel.startsWith ('&') &&
						!channel.startsWith ('!'))
					channelList [i].prepend ('#');
			}

			QString passwords;
			if (messageList.count () == 2)
				passwords = messageList.last ();
			messageList.clear ();
			messageList << channelList.join (",")
					<< passwords;
		}
		else if (cmd == "away")
		{
			ISH_->SetAway (messageList.join (" "));
			return cmd;
		}
// 		else if (cmd == "kick" && !message.isEmpty ())
// 		{
// 			if (ISH_->IsParticipantExists (messageList.first ()))
// 				messageList.insert (0, channelName);
// 		}
		else if (cmd == "say")
			messageList.insert (0, channelName);

		if (parserCommand)
			(Parser_->*parserCommand) (messageList);
		else if (serverCommand)
			(ISH_->*serverCommand) (messageList);
		return cmd;
	}
}
