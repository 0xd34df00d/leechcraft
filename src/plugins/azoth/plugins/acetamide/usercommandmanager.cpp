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
		constexpr uint64_t CalcHash (std::string_view name)
		{
			uint64_t res = 0;
			for (auto ch : name)
				res = (res << 8) + ch;
			return res;
		}

		template<typename T>
		using CmdImpl_t = void (T::*) (const QStringList&);

		template<typename T>
		using KVPair = Util::KVPair<std::string_view, CmdImpl_t<T>>;

		using KVPairISH = KVPair<IrcServerHandler>;
		using KVPairP = KVPair<IrcParser>;

		constexpr auto ServerHash = Util::MakeHash<std::string_view, CmdImpl_t<IrcServerHandler>, CalcHash> (
				KVPairISH { "privmsg", &IrcServerHandler::SendMessage },
				KVPairISH { "msg", &IrcServerHandler::SendMessage },
				KVPairISH { "say", &IrcServerHandler::SayCommand },
				KVPairISH { "list", &IrcServerHandler::showChannels }
			);
		constexpr auto ParserHash = Util::MakeHash<std::string_view, CmdImpl_t<IrcParser>, CalcHash> (
				KVPairP { "join", &IrcParser::JoinCommand },
				KVPairP { "part", &IrcParser::PartCommand },
				KVPairP { "quit", &IrcParser::QuitCommand },
				KVPairP { "nick", &IrcParser::NickCommand },
				KVPairP { "ping", &IrcParser::PingCommand },
				KVPairP { "pong", &IrcParser::PongCommand },
				KVPairP { "topic", &IrcParser::TopicCommand },
				KVPairP { "kick", &IrcParser::KickCommand },
				KVPairP { "invite", &IrcParser::InviteCommand },
				KVPairP { "ctcp", &IrcParser::CTCPRequest },
				KVPairP { "names", &IrcParser::NamesCommand },
				KVPairP { "away", &IrcParser::AwayCommand },
				KVPairP { "userhost", &IrcParser::UserhostCommand },
				KVPairP { "ison", &IrcParser::IsonCommand },
				KVPairP { "whois", &IrcParser::WhoisCommand },
				KVPairP { "whowas", &IrcParser::WhowasCommand },
				KVPairP { "who", &IrcParser::WhoCommand },
				KVPairP { "summon", &IrcParser::SummonCommand },
				KVPairP { "version", &IrcParser::VersionCommand },
				KVPairP { "links", &IrcParser::LinksCommand },
				KVPairP { "info", &IrcParser::InfoCommand },
				KVPairP { "motd", &IrcParser::MOTDCommand },
				KVPairP { "time", &IrcParser::TimeCommand },
				KVPairP { "oper", &IrcParser::OperCommand },
				KVPairP { "rehash", &IrcParser::RehashCommand },
				KVPairP { "lusers", &IrcParser::LusersCommand },
				KVPairP { "users", &IrcParser::UsersCommand },
				KVPairP { "wallops", &IrcParser::WallopsCommand },
				KVPairP { "quote", &IrcParser::RawCommand },
				KVPairP { "me", &IrcParser::CTCPRequest },
				KVPairP { "squit", &IrcParser::SQuitCommand },
				KVPairP { "stats", &IrcParser::StatsCommand },
				KVPairP { "connect", &IrcParser::ConnectCommand },
				KVPairP { "trace", &IrcParser::TraceCommand },
				KVPairP { "admin", &IrcParser::AdminCommand },
				KVPairP { "kill", &IrcParser::KillCommand },
				KVPairP { "die", &IrcParser::DieCommand },
				KVPairP { "restart", &IrcParser::RestartCommand },
				KVPairP { "mode", &IrcParser::ChanModeCommand }
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
