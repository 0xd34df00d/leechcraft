/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "usercommandmanager.h"
#include <boost/bind.hpp>
#include "ircserverhandler.h"
#include "ircparser.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	UserCommandManager::UserCommandManager (IrcServerHandler *ish)
	: QObject (ish)
	, ISH_ (ish)
	, Parser_ (ish->GetParser ())
	{
		Init ();
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
			QStringList channelList = messageList.first ().split (',');

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

	void UserCommandManager::Init ()
	{
		Command2Action_ ["join"] = boost::bind (&IrcParser::JoinCommand,
				Parser_, _1);
		Command2Action_ ["part"] = boost::bind (&IrcParser::PartCommand,
				Parser_, _1);
		Command2Action_ ["quit"] = boost::bind (&IrcParser::QuitCommand,
				Parser_, _1);
		Command2Action_ ["privmsg"] = boost::bind (&IrcServerHandler::SendMessage,
				ISH_, _1);
		Command2Action_ ["msg"] = boost::bind (&IrcServerHandler::SendMessage,
				ISH_, _1);
		Command2Action_ ["nick"] = boost::bind (&IrcParser::NickCommand,
				Parser_, _1);
		Command2Action_ ["ping"] = boost::bind (&IrcParser::PingCommand,
				Parser_, _1);
		Command2Action_ ["pong"] = boost::bind (&IrcParser::PongCommand,
				Parser_, _1);
		Command2Action_ ["topic"] = boost::bind (&IrcParser::TopicCommand,
				Parser_, _1);
		Command2Action_ ["kick"] = boost::bind (&IrcParser::KickCommand,
				Parser_, _1);
		Command2Action_ ["invite"] = boost::bind (&IrcParser::InviteCommand,
				Parser_, _1);
		Command2Action_ ["ctcp"] = boost::bind (&IrcParser::CTCPRequest,
				Parser_, _1);
		Command2Action_ ["names"] = boost::bind (&IrcParser::NamesCommand,
				Parser_, _1);
		Command2Action_ ["away"] = boost::bind (&IrcParser::AwayCommand,
				Parser_, _1);
		Command2Action_ ["userhost"] = boost::bind (&IrcParser::UserhostCommand,
				Parser_, _1);
		Command2Action_ ["ison"] = boost::bind (&IrcParser::IsonCommand,
				Parser_, _1);
		Command2Action_ ["whois"] = boost::bind (&IrcParser::WhoisCommand,
				Parser_, _1);
		Command2Action_ ["whowas"] = boost::bind (&IrcParser::WhowasCommand,
				Parser_, _1);
		Command2Action_ ["who"] = boost::bind (&IrcParser::WhoCommand,
				Parser_, _1);
		Command2Action_ ["summon"] = boost::bind (&IrcParser::SummonCommand,
				Parser_, _1);
		Command2Action_ ["version"] = boost::bind (&IrcParser::VersionCommand,
				Parser_, _1);
		Command2Action_ ["links"] = boost::bind (&IrcParser::LinksCommand,
				Parser_, _1);
		Command2Action_ ["info"] = boost::bind (&IrcParser::InfoCommand,
				Parser_, _1);
		Command2Action_ ["motd"] = boost::bind (&IrcParser::MOTDCommand,
				Parser_, _1);
		Command2Action_ ["time"] = boost::bind (&IrcParser::TimeCommand,
				Parser_, _1);
		Command2Action_ ["oper"] = boost::bind (&IrcParser::OperCommand,
				Parser_, _1);
		Command2Action_ ["rehash"] = boost::bind (&IrcParser::RehashCommand,
				Parser_, _1);
		Command2Action_ ["lusers"] = boost::bind (&IrcParser::LusersCommand,
				Parser_, _1);
		Command2Action_ ["users"] = boost::bind (&IrcParser::UsersCommand,
				Parser_, _1);
		Command2Action_ ["wallops"] = boost::bind (&IrcParser::WallopsCommand,
				Parser_, _1);
		Command2Action_ ["quote"] = boost::bind (&IrcParser::RawCommand,
				Parser_, _1);
		Command2Action_ ["me"] = boost::bind (&IrcParser::CTCPRequest,
				Parser_, _1);
		Command2Action_ ["squit"] = boost::bind (&IrcParser::SQuitCommand,
				Parser_, _1);
		Command2Action_ ["stats"] = boost::bind (&IrcParser::StatsCommand,
				Parser_, _1);
		Command2Action_ ["connect"] = boost::bind (&IrcParser::ConnectCommand,
				Parser_, _1);
		Command2Action_ ["trace"] = boost::bind (&IrcParser::TraceCommand,
				Parser_, _1);
		Command2Action_ ["admin"] = boost::bind (&IrcParser::AdminCommand,
				Parser_, _1);
		Command2Action_ ["kill"] = boost::bind (&IrcParser::KillCommand,
				Parser_, _1);
		Command2Action_ ["die"] = boost::bind (&IrcParser::DieCommand,
				Parser_, _1);
		Command2Action_ ["restart"] = boost::bind (&IrcParser::RestartCommand,
				Parser_, _1);
		Command2Action_ ["mode"] = boost::bind (&IrcParser::ChanModeCommand,
				Parser_, _1);
		Command2Action_ ["say"] = boost::bind (&IrcServerHandler::SayCommand,
				ISH_, _1);
	}
}
}
}
