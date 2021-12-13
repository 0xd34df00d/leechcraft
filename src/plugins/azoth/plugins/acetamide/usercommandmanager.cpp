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

		constexpr auto ServerHash = Util::MakeStringHash<CmdImpl_t<IrcServerHandler>> (
					KVPair { "privmsg", &IrcServerHandler::SendMessage },
					KVPair { "msg", &IrcServerHandler::SendMessage },
					KVPair { "say", &IrcServerHandler::SayCommand },
					KVPair { "list", &IrcServerHandler::showChannels }
				);

		constexpr auto ParserHash = Util::MakeStringHash<CmdImpl_t<IrcParser>> (
					KVPair { "join", &IrcParser::JoinCommand },
					KVPair { "part", &IrcParser::PartCommand },
					KVPair { "quit", &IrcParser::QuitCommand },
					KVPair { "nick", &IrcParser::NickCommand },
					KVPair { "ping", &IrcParser::PingCommand },
					KVPair { "pong", &IrcParser::PongCommand },
					KVPair { "topic", &IrcParser::TopicCommand },
					KVPair { "kick", &IrcParser::KickCommand },
					KVPair { "invite", &IrcParser::InviteCommand },
					KVPair { "ctcp", &IrcParser::CTCPRequest },
					KVPair { "names", &IrcParser::NamesCommand },
					KVPair { "away", &IrcParser::AwayCommand },
					KVPair { "userhost", &IrcParser::UserhostCommand },
					KVPair { "ison", &IrcParser::IsonCommand },
					KVPair { "whois", &IrcParser::WhoisCommand },
					KVPair { "whowas", &IrcParser::WhowasCommand },
					KVPair { "who", &IrcParser::WhoCommand },
					KVPair { "summon", &IrcParser::SummonCommand },
					KVPair { "version", &IrcParser::VersionCommand },
					KVPair { "links", &IrcParser::LinksCommand },
					KVPair { "info", &IrcParser::InfoCommand },
					KVPair { "motd", &IrcParser::MOTDCommand },
					KVPair { "time", &IrcParser::TimeCommand },
					KVPair { "oper", &IrcParser::OperCommand },
					KVPair { "rehash", &IrcParser::RehashCommand },
					KVPair { "lusers", &IrcParser::LusersCommand },
					KVPair { "users", &IrcParser::UsersCommand },
					KVPair { "wallops", &IrcParser::WallopsCommand },
					KVPair { "quote", &IrcParser::QuoteCommand },
					KVPair { "me", &IrcParser::CTCPRequest },
					KVPair { "squit", &IrcParser::SQuitCommand },
					KVPair { "stats", &IrcParser::StatsCommand },
					KVPair { "connect", &IrcParser::ConnectCommand },
					KVPair { "trace", &IrcParser::TraceCommand },
					KVPair { "admin", &IrcParser::AdminCommand },
					KVPair { "kill", &IrcParser::KillCommand },
					KVPair { "die", &IrcParser::DieCommand },
					KVPair { "restart", &IrcParser::RestartCommand },
					KVPair { "mode", &IrcParser::ChanModeCommand }
				);
	}

	UserCommandManager::UserCommandManager (IrcServerHandler *ish,
			IrcParser *parser)
	: ISH_ { ish }
	, Parser_ { parser }
	{
	}

	QByteArray UserCommandManager::VerifyMessage (const QString& msg, const QString& channelName) const
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
