/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "usercommandmanager.h"
#include <util/sll/typegetter.h>
#include "ircserverhandler.h"
#include "ircparser.h"
#include "ircaccount.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	UserCommandManager::UserCommandManager (IrcServerHandler *ish, 
			IrcParser *parser)
	: QObject (ish)
	, ISH_ (ish)
	{
		auto bind = [=] (auto funPtr)
		{
			using Obj_t = Util::MemberTypeStruct_t<decltype (funPtr)>;
			if constexpr (std::is_same_v<Obj_t, IrcParser>)
				return [funPtr, parser] (const QStringList& list) { (parser->*funPtr) (list); };
			if constexpr (std::is_same_v<Obj_t, IrcServerHandler>)
				return [funPtr, ish] (const QStringList& list) { (ish->*funPtr) (list); };
		};

		Command2Action_ ["join"] = bind (&IrcParser::JoinCommand);
		Command2Action_ ["part"] = bind (&IrcParser::PartCommand);
		Command2Action_ ["quit"] = bind (&IrcParser::QuitCommand);
		Command2Action_ ["privmsg"] = bind (&IrcServerHandler::SendMessage);
		Command2Action_ ["msg"] = bind (&IrcServerHandler::SendMessage);
		Command2Action_ ["nick"] = bind (&IrcParser::NickCommand);
		Command2Action_ ["ping"] = bind (&IrcParser::PingCommand);
		Command2Action_ ["pong"] = bind (&IrcParser::PongCommand);
		Command2Action_ ["topic"] = bind (&IrcParser::TopicCommand);
		Command2Action_ ["kick"] = bind (&IrcParser::KickCommand);
		Command2Action_ ["invite"] = bind (&IrcParser::InviteCommand);
		Command2Action_ ["ctcp"] = bind (&IrcParser::CTCPRequest);
		Command2Action_ ["names"] = bind (&IrcParser::NamesCommand);
		Command2Action_ ["away"] = bind (&IrcParser::AwayCommand);
		Command2Action_ ["userhost"] = bind (&IrcParser::UserhostCommand);
		Command2Action_ ["ison"] = bind (&IrcParser::IsonCommand);
		Command2Action_ ["whois"] = bind (&IrcParser::WhoisCommand);
		Command2Action_ ["whowas"] = bind (&IrcParser::WhowasCommand);
		Command2Action_ ["who"] = bind (&IrcParser::WhoCommand);
		Command2Action_ ["summon"] = bind (&IrcParser::SummonCommand);
		Command2Action_ ["version"] = bind (&IrcParser::VersionCommand);
		Command2Action_ ["links"] = bind (&IrcParser::LinksCommand);
		Command2Action_ ["info"] = bind (&IrcParser::InfoCommand);
		Command2Action_ ["motd"] = bind (&IrcParser::MOTDCommand);
		Command2Action_ ["time"] = bind (&IrcParser::TimeCommand);
		Command2Action_ ["oper"] = bind (&IrcParser::OperCommand);
		Command2Action_ ["rehash"] = bind (&IrcParser::RehashCommand);
		Command2Action_ ["lusers"] = bind (&IrcParser::LusersCommand);
		Command2Action_ ["users"] = bind (&IrcParser::UsersCommand);
		Command2Action_ ["wallops"] = bind (&IrcParser::WallopsCommand);
		Command2Action_ ["quote"] = bind (&IrcParser::RawCommand);
		Command2Action_ ["me"] = bind (&IrcParser::CTCPRequest);
		Command2Action_ ["squit"] = bind (&IrcParser::SQuitCommand);
		Command2Action_ ["stats"] = bind (&IrcParser::StatsCommand);
		Command2Action_ ["connect"] = bind (&IrcParser::ConnectCommand);
		Command2Action_ ["trace"] = bind (&IrcParser::TraceCommand);
		Command2Action_ ["admin"] = bind (&IrcParser::AdminCommand);
		Command2Action_ ["kill"] = bind (&IrcParser::KillCommand);
		Command2Action_ ["die"] = bind (&IrcParser::DieCommand);
		Command2Action_ ["restart"] = bind (&IrcParser::RestartCommand);
		Command2Action_ ["mode"] = bind (&IrcParser::ChanModeCommand);
		Command2Action_ ["say"] = bind (&IrcServerHandler::SayCommand);
		Command2Action_ ["list"] = bind (&IrcServerHandler::showChannels);
	}

	QString UserCommandManager::VerifyMessage (const QString& msg,
			const QString& channelName)
	{
		const int pos = msg.indexOf (' ');
		QString cmd;
		if (msg.startsWith ('/'))
			cmd = msg.mid (1, pos).trimmed ().toLower ();
		else
			cmd = msg.left (pos).trimmed ().toLower ();

		if (!Command2Action_.contains (cmd))
			return QString ();

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

		Command2Action_ [cmd] (messageList);
		return cmd;
	}
}
}
}
