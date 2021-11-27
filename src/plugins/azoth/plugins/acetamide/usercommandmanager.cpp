/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "usercommandmanager.h"
#include <QHash>
#include "ircserverhandler.h"
#include "ircparser.h"
#include "ircaccount.h"

namespace LC::Azoth::Acetamide
{
	namespace
	{
		const QHash<QByteArray, void (IrcServerHandler::*) (const QStringList&)> ServerCommand2Action
		{
			{ "privmsg", &IrcServerHandler::SendMessage },
			{ "msg", &IrcServerHandler::SendMessage },
			{ "say", &IrcServerHandler::SayCommand },
			{ "list", &IrcServerHandler::showChannels },
		};
		const QHash<QByteArray, void (IrcParser::*) (const QStringList&)> ParserCommand2Action
		{
			{ "join", &IrcParser::JoinCommand },
			{ "part", &IrcParser::PartCommand },
			{ "quit", &IrcParser::QuitCommand },
			{ "nick", &IrcParser::NickCommand },
			{ "ping", &IrcParser::PingCommand },
			{ "pong", &IrcParser::PongCommand },
			{ "topic", &IrcParser::TopicCommand },
			{ "kick", &IrcParser::KickCommand },
			{ "invite", &IrcParser::InviteCommand },
			{ "ctcp", &IrcParser::CTCPRequest },
			{ "names", &IrcParser::NamesCommand },
			{ "away", &IrcParser::AwayCommand },
			{ "userhost", &IrcParser::UserhostCommand },
			{ "ison", &IrcParser::IsonCommand },
			{ "whois", &IrcParser::WhoisCommand },
			{ "whowas", &IrcParser::WhowasCommand },
			{ "who", &IrcParser::WhoCommand },
			{ "summon", &IrcParser::SummonCommand },
			{ "version", &IrcParser::VersionCommand },
			{ "links", &IrcParser::LinksCommand },
			{ "info", &IrcParser::InfoCommand },
			{ "motd", &IrcParser::MOTDCommand },
			{ "time", &IrcParser::TimeCommand },
			{ "oper", &IrcParser::OperCommand },
			{ "rehash", &IrcParser::RehashCommand },
			{ "lusers", &IrcParser::LusersCommand },
			{ "users", &IrcParser::UsersCommand },
			{ "wallops", &IrcParser::WallopsCommand },
			{ "quote", &IrcParser::RawCommand },
			{ "me", &IrcParser::CTCPRequest },
			{ "squit", &IrcParser::SQuitCommand },
			{ "stats", &IrcParser::StatsCommand },
			{ "connect", &IrcParser::ConnectCommand },
			{ "trace", &IrcParser::TraceCommand },
			{ "admin", &IrcParser::AdminCommand },
			{ "kill", &IrcParser::KillCommand },
			{ "die", &IrcParser::DieCommand },
			{ "restart", &IrcParser::RestartCommand },
			{ "mode", &IrcParser::ChanModeCommand },
		};
	}

	UserCommandManager::UserCommandManager (IrcServerHandler *ish, 
			IrcParser *parser)
	: QObject { ish }
	, ISH_ { ish }
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

		auto serverCommand = ServerCommand2Action [cmd];
		auto parserCommand = ParserCommand2Action [cmd];
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
