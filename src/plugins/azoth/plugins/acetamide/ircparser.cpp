/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircparser.h"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include "ircaccount.h"
#include "ircserverhandler.h"

namespace LC::Azoth::Acetamide
{
	using namespace boost::spirit::classic;

	IrcParser::IrcParser (IrcServerHandler *sh)
	: QObject { sh }
	, ISH_ { sh }
	, ServerOptions_ { sh->GetServerOptions () }
	{
	}

	void IrcParser::AuthCommand ()
	{
		const auto& pass = ServerOptions_.ServerPassword_;
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

	void IrcParser::RawCommand (const QStringList& cmd)
	{
		ISH_->SendCommand (cmd.join (' '));
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

	bool IrcParser::ParseMessage (const QString& msg)
	{
		IrcMessageOptions_.Command_.clear ();
		IrcMessageOptions_.Nick_.clear ();
		IrcMessageOptions_.Message_.clear ();
		IrcMessageOptions_.Parameters_.clear ();
		IrcMessageOptions_.UserName_.clear ();
		IrcMessageOptions_.Host_.clear ();

		std::string nickStr;
		std::string userStr;
		std::string hostStr;
		std::string commandStr;
		std::string msgStr;
		QList<std::string> opts;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' |
				'_' | '^' | '{' | '|' | '}'];
		rule<> shortname = *(alnum_p
				>> *(alnum_p || ch_p ('-'))
				>> *alnum_p);
		rule<> hostname = (shortname
				>> *(ch_p ('.')
				>> shortname)) [assign_a (hostStr)];
		rule<> nickname = (alpha_p | special)
				>> *(alnum_p | special | ch_p ('-'));
		rule<> user =  +(ascii - '\r' - '\n' - ' ' - '@' - '\0');
		rule<> host = lexeme_d [+(anychar_p - ' ')] ;
		rule<> nick = lexeme_d [nickname [assign_a (nickStr)]
				>> !(!(ch_p ('!')
				>> user [assign_a (userStr)])
				>> ch_p ('@')
				>> host [assign_a (hostStr)])];
		rule<> nospcrlfcl = (anychar_p - '\0' - '\r' - '\n' -
				' ' - ':');
		rule<> lastParam = lexeme_d [ch_p (' ')
				>> !ch_p (':')
				>> (*(ch_p (':') | ch_p (' ') | nospcrlfcl))
					[assign_a (msgStr)]];
		rule<> firsParam = lexeme_d [ch_p (' ')
				>> (nospcrlfcl
				>> *(ch_p (':') | nospcrlfcl))
					[push_back_a (opts)]];
		rule<> params =  *firsParam
				>> !lastParam;
		rule<> command = longest_d [(+alpha_p) |
				(repeat_p (3) [digit_p])] [assign_a (commandStr)];
		rule<> prefix = longest_d [hostname | nick];
		rule<> reply = (lexeme_d [!(ch_p (':')
				>> prefix >> ch_p (' '))]
				>> command
				>> !params
				>> eol_p);

		bool res = parse (msg.toUtf8 ().constData (), reply).full;

		if (!res)
		{
			qWarning () << "input string is not a valid IRC command"
					<< msg;
			return false;
		}

		IrcMessageOptions_.Nick_ = QString::fromStdString (nickStr);
		IrcMessageOptions_.Command_ = QString::fromStdString (commandStr).toLower ();
		IrcMessageOptions_.Message_ = QString::fromStdString (msgStr);
		IrcMessageOptions_.UserName_ = QString::fromStdString (userStr);
		IrcMessageOptions_.Host_ = QString::fromStdString (hostStr);
		IrcMessageOptions_.Parameters_ = opts;

		return true;
	}

	IrcMessageOptions IrcParser::GetIrcMessageOptions () const
	{
		return IrcMessageOptions_;
	}
}
