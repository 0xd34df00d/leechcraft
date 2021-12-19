/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircparser.h"
#include "ircaccount.h"
#include "ircserverhandler.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	IrcParser::IrcParser (IrcServerHandler *sh)
	: QObject { sh }
	, ISH_ { sh }
	{
	}

	void IrcParser::AuthCommand ()
	{
		const auto& pass = ISH_->GetServerOptions ().ServerPassword_;
		if (!pass.isEmpty ())
			ISH_->SendCommand ("PASS " + pass);

		UserCommand ();
		NickCommand ({ ISH_->GetNickName () });
	}

	void IrcParser::UserCommand ()
	{
		ISH_->SendCommand ("USER " +
				ISH_->GetAccount ()->GetUserName () + " 8 * :" +
				ISH_->GetAccount ()->GetRealName ());
	}

	void IrcParser::QuoteCommand (const QStringList& quote)
	{
		ISH_->SendCommand (quote.join (' '));
	}

	void IrcParser::NickCommand (const QStringList& nick)
	{
		ISH_->SendCommand ("NICK " + nick.first ());
	}

	void IrcParser::JoinCommand (const QStringList& rawCmd)
	{
		auto cmd = rawCmd;

		auto& str = cmd [0];
		if (!str.isEmpty () &&
				!str.startsWith ('#') &&
				!str.startsWith ('&') &&
				!str.startsWith ('+') &&
				!str.startsWith ('!'))
			str.prepend ('#');

		ISH_->SendCommand ("JOIN " + cmd.join (' '));
	}

	void IrcParser::PrivMsgCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("PRIVMSG " + cmd.first () + " :" + cmd.mid (1).join (' '));
	}

	void IrcParser::PartCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("PART " + cmd.first () + " :" + cmd.mid (1).join (' '));
	}

	void IrcParser::PongCommand (const QStringList& msg)
	{
		ISH_->SendCommand ("PONG :" + msg.join (' '));
	}

	void IrcParser::CTCPRequest (const QStringList& cmd)
	{
		ISH_->SendCommand ("PRIVMSG " + cmd.first () + " :\001" + cmd.mid (1).join (' ') + "\001");
	}

	void IrcParser::CTCPReply (const QStringList& cmd)
	{
		ISH_->SendCommand ("NOTICE " + cmd.first () + " :" + cmd.last ());
	}

	void IrcParser::TopicCommand (const QStringList& cmd)
	{
		auto topicCmd = "TOPIC " + cmd.first ();
		if (cmd.count () > 1)
			topicCmd += " :" + cmd.mid (1).join (' ');
		ISH_->SendCommand (topicCmd);
	}

	void IrcParser::NamesCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("NAMES " + cmd.value (0));
	}

	void IrcParser::InviteCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("INVITE " + cmd.join (' '));
	}

	void IrcParser::KickCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("KICK " + cmd.join (' '));
	}

	void IrcParser::OperCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("OPER " + cmd.at (0) + " :" + cmd.mid (1).join (' '));
	}

	void IrcParser::SQuitCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("SQUIT " + cmd.first () + " :" + cmd.mid (1).join (' '));
	}

	void IrcParser::MOTDCommand (const QStringList& cmd)
	{
		ISH_->SendCommand (cmd.isEmpty () ? QStringLiteral ("MOTD") : "MOTD " + cmd.first ());
	}

	void IrcParser::LusersCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("LUSERS " + cmd.join (' '));
	}

	void IrcParser::VersionCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("VERSION " + cmd.join (' '));
	}

	void IrcParser::StatsCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("STATS " + cmd.join (' '));
	}

	void IrcParser::LinksCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("LINKS " + cmd.join (' '));
	}

	void IrcParser::TimeCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("TIME " + cmd.join (' '));
	}

	void IrcParser::ConnectCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("CONNECT " + cmd.join (' '));
	}

	void IrcParser::TraceCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("TRACE " + cmd.join (' '));
	}

	void IrcParser::AdminCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("ADMIN " + cmd.join (' '));
	}

	void IrcParser::InfoCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("INFO " + cmd.join (' '));
	}

	void IrcParser::WhoCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("WHO " + cmd.join (' '));
	}

	void IrcParser::WhoisCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("WHOIS " + cmd.join (' '));
	}

	void IrcParser::WhowasCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("WHOWAS " + cmd.join (' '));
	}

	void IrcParser::KillCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("KILL " + cmd.first () + " :" + cmd.mid (1).join (' '));
	}

	void IrcParser::PingCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("PING " + cmd.join (' '));
	}

	void IrcParser::AwayCommand (const QStringList& cmd)
	{
		ISH_->SendCommand (cmd.isEmpty () ? QStringLiteral ("AWAY") : "AWAY :" + cmd.join (' '));
	}

	void IrcParser::RehashCommand (const QStringList&)
	{
		ISH_->SendCommand (QStringLiteral ("REHASH"));
	}

	void IrcParser::DieCommand (const QStringList&)
	{
		ISH_->SendCommand (QStringLiteral ("DIE"));
	}

	void IrcParser::RestartCommand (const QStringList&)
	{
		ISH_->SendCommand (QStringLiteral ("RESTART"));
	}

	void IrcParser::SummonCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("SUMMON " + cmd.first () + cmd.mid (1).join (' '));
	}

	void IrcParser::UsersCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("USERS " + cmd.first ());
	}

	void IrcParser::UserhostCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("USERHOST " + cmd.join (' '));
	}

	void IrcParser::WallopsCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("WALLOPS :" + cmd.join (' '));
	}

	void IrcParser::IsonCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("ISON " + cmd.join (' '));
	}

	void IrcParser::QuitCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("QUIT :" + cmd.join (' '));
	}

	void IrcParser::ChanModeCommand (const QStringList& cmd)
	{
		ISH_->SendCommand ("MODE " + cmd.join (' '));
	}

	void IrcParser::ChannelsListCommand (const QStringList&)
	{
		ISH_->SendCommand (QStringLiteral ("LIST "));
	}
}
