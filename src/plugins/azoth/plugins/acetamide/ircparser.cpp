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

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	using namespace boost::spirit::classic;

	IrcParser::IrcParser (IrcServerHandler *sh)
	: QObject (sh)
	, ISH_ (sh)
	, ServerOptions_ (sh->GetServerOptions ())
	{
		LongAnswerCommands_ << "mode"
				<< "names"
				<< "motd"
				<< "stats"
				<< "links"
				<< "info"
				<< "who"
				<< "whois"
				<< "whowas"
				<< "users"
				<< "trace";
	}

	bool IrcParser::CmdHasLongAnswer (const QString& cmd)
	{
		return LongAnswerCommands_.contains (cmd.toLower ());
	}

	void IrcParser::AuthCommand ()
	{
		QString pass = ServerOptions_.ServerPassword_;
		if (!pass.isEmpty ())
			ISH_->SendCommand ("PASS " + pass);

		UserCommand ();
		NickCommand (QStringList () << ISH_->GetNickName ());
	}

	void IrcParser::UserCommand ()
	{
		QString userCmd ("USER " +
				ISH_->GetAccount ()->GetUserName () + " 8 * :" +
				ISH_->GetAccount ()->GetRealName ());
		ISH_->SendCommand (userCmd);
	}

	void IrcParser::NickCommand (const QStringList& nick)
	{
		const auto& name = nick.value (0);
		QString nickCmd ("NICK " + name);
		ISH_->SendCommand (nickCmd);
	}

	void IrcParser::JoinCommand (QStringList cmd)
	{
		auto& str = cmd [0];
		if (!str.isEmpty () &&
				!str.startsWith ('#') &&
				!str.startsWith ('&') &&
				!str.startsWith ('+') &&
				!str.startsWith ('!'))
			str.prepend ('#');

		QString joinCmd ("JOIN " + cmd.join (" "));
		ISH_->SendCommand (joinCmd);
	}

	void IrcParser::PrivMsgCommand (const QStringList& cmd)
	{
		const QString target = cmd.first ();
		const QStringList msg = cmd.mid (1);
		QString privmsgCmd ("PRIVMSG " + target + " :" +
				msg.join (" "));
		ISH_->SendCommand (privmsgCmd);
	}

	void IrcParser::PartCommand (const QStringList& cmd)
	{
		QString partCmd ("PART " + cmd.first () + " :" +
				cmd.mid (1).join (" "));
		ISH_->SendCommand (partCmd);
	}

	void IrcParser::PongCommand (const QStringList& msg)
	{
		QString pongCmd ("PONG :" + msg.join (" "));
		ISH_->SendCommand (pongCmd);
	}

	void IrcParser::RawCommand (const QStringList& cmd)
	{
		QString rawCmd = cmd.join (" ");
		ISH_->SendCommand (rawCmd);
	}

	void IrcParser::CTCPRequest (const QStringList& cmd)
	{
		QString ctcpCmd;
		if (cmd.count () > 2)
			ctcpCmd = "PRIVMSG " + cmd.first () + " :\001" +
					cmd.at (1) + " " + cmd.mid (2).join (" ") +
					"\001";
		else
			ctcpCmd = "PRIVMSG " + cmd.first () + " :\001" +
					cmd.at (1) + "\001";
		ISH_->SendCommand (ctcpCmd);
	}

	void IrcParser::CTCPReply (const QStringList& cmd)
	{
		QString ctcpCmd ("NOTICE " + cmd.first () + " :" +
				cmd.last ());
		ISH_->SendCommand (ctcpCmd);
	}

	void IrcParser::TopicCommand (const QStringList& cmd)
	{
		QString topicCmd;
		if (cmd.count () == 1)
			topicCmd = QString ("TOPIC " + cmd.first ());
		else
			topicCmd = QString ("TOPIC " + cmd.first () + " :" + cmd.mid (1).join (" "));
		ISH_->SendCommand (topicCmd);
	}

	void IrcParser::NamesCommand (const QStringList& cmd)
	{
		auto target = cmd.value (0);

		QString namesCmd ("NAMES " + target);
		ISH_->SendCommand (namesCmd);
	}

	void IrcParser::InviteCommand (const QStringList& cmd)
	{
		QString inviteCmd ("INVITE " + cmd.join (" "));
		ISH_->SendCommand (inviteCmd);
	}

	void IrcParser::KickCommand (const QStringList& cmd)
	{
		QString kickCmd ("KICK " + cmd.join (" "));
		ISH_->SendCommand (kickCmd);
	}

	void IrcParser::OperCommand (const QStringList& cmd)
	{
		QString operCmd ("OPER " + cmd.at (0) + " :" +
				QStringList (cmd.mid (1)).join (" "));
		ISH_->SendCommand (operCmd);
	}

	void IrcParser::SQuitCommand (const QStringList& cmd)
	{
		QString squitCmd ("SQUIT " + cmd.first () + " :" +
				QStringList (cmd.mid (1)).join (" "));
		ISH_->SendCommand (squitCmd);
	}

	void IrcParser::MOTDCommand (const QStringList& cmd)
	{
		QString motdCmd;
		if (cmd.isEmpty ())
			motdCmd = QString ("MOTD");
		else
			motdCmd = QString ("MOTD " + cmd.first ());

		ISH_->SendCommand (motdCmd);
	}

	void IrcParser::LusersCommand (const QStringList& cmd)
	{
		QString lusersCmd ("LUSERS " + cmd.join (" "));
		ISH_->SendCommand (lusersCmd);
	}

	void IrcParser::VersionCommand (const QStringList& cmd)
	{
		QString versionCmd ("VERSION " + cmd.join (" "));
		ISH_->SendCommand (versionCmd);
	}

	void IrcParser::StatsCommand (const QStringList& cmd)
	{
		QString statsCmd ("STATS " + cmd.join (" "));
		ISH_->SendCommand (statsCmd);
	}

	void IrcParser::LinksCommand (const QStringList& cmd)
	{
		QString linksCmd ("LINKS " + cmd.join (" "));
		ISH_->SendCommand (linksCmd);
	}

	void IrcParser::TimeCommand (const QStringList& cmd)
	{
		QString timeCmd ("TIME " + cmd.join (" "));
		ISH_->SendCommand (timeCmd);
	}

	void IrcParser::ConnectCommand (const QStringList& cmd)
	{
		QString connectCmd ("CONNECT " + cmd.join (" "));
		ISH_->SendCommand (connectCmd);
	}

	void IrcParser::TraceCommand (const QStringList& cmd)
	{
		QString traceCmd ("TRACE " + cmd.join (" "));
		ISH_->SendCommand (traceCmd);
	}

	void IrcParser::AdminCommand (const QStringList& cmd)
	{
		QString adminCmd ("ADMIN " + cmd.join (" "));
		ISH_->SendCommand (adminCmd);
	}

	void IrcParser::InfoCommand (const QStringList& cmd)
	{
		QString infoCmd ("INFO " + cmd.join (" "));
		ISH_->SendCommand (infoCmd);
	}

	void IrcParser::WhoCommand (const QStringList& cmd)
	{
		QString whoCmd ("WHO " + cmd.join (" "));
		ISH_->SendCommand (whoCmd);
	}

	void IrcParser::WhoisCommand (const QStringList& cmd)
	{
		QString whoisCmd ("WHOIS " + cmd.join (" "));
		ISH_->SendCommand (whoisCmd);
	}

	void IrcParser::WhowasCommand (const QStringList& cmd)
	{
		QString whowasCmd ("WHOWAS " + cmd.join (" "));
		ISH_->SendCommand (whowasCmd);
	}

	void IrcParser::KillCommand (const QStringList& cmd)
	{
		QString killCmd ("KILL " + cmd.first () + " :" +
				cmd.mid (1).join (" "));
		ISH_->SendCommand (killCmd);
	}

	void IrcParser::PingCommand (const QStringList& cmd)
	{
		QString pingCmd ("PING " + cmd.join (" "));
		ISH_->SendCommand (pingCmd);
	}

	void IrcParser::AwayCommand (const QStringList& cmd)
	{
		QString awayCmd;
		if (!cmd.isEmpty ())
			awayCmd = QString ("AWAY :" + cmd.join (" "));
		else
			awayCmd = QString ("AWAY");
		ISH_->SendCommand (awayCmd);
	}

	void IrcParser::RehashCommand (const QStringList&)
	{
		QString rehashCmd ("REHASH");
		ISH_->SendCommand (rehashCmd);
	}

	void IrcParser::DieCommand (const QStringList&)
	{
		QString dieCmd ("DIE");
		ISH_->SendCommand (dieCmd);
	}

	void IrcParser::RestartCommand (const QStringList&)
	{
		QString dieCmd ("RESTART");
		ISH_->SendCommand (dieCmd);
	}

	void IrcParser::SummonCommand (const QStringList& cmd)
	{
		QString summonCmd ("SUMMON " + cmd.first () +
				QStringList (cmd.mid (1)).join (" "));
		ISH_->SendCommand (summonCmd);
	}

	void IrcParser::UsersCommand (const QStringList& cmd)
	{
		QString usersCmd ("USERS " + cmd.first ());
		ISH_->SendCommand (usersCmd);
	}

	void IrcParser::UserhostCommand (const QStringList& cmd)
	{
		QString userhostCmd ("USERHOST " + cmd.join (" "));
		ISH_->SendCommand (userhostCmd);
	}

	void IrcParser::WallopsCommand (const QStringList& cmd)
	{
		QString wallopsCmd ("WALLOPS :" + cmd.join (" "));
		ISH_->SendCommand (wallopsCmd);
	}

	void IrcParser::IsonCommand (const QStringList& cmd)
	{
		QString isonCmd ("ISON " + cmd.join (" "));
		ISH_->SendCommand (isonCmd);
	}

	void IrcParser::QuitCommand (const QStringList& cmd)
	{
		QString quitCmd ("QUIT :" + cmd.join (" "));
		ISH_->SendCommand (quitCmd);
	}

	void IrcParser::ChanModeCommand (const QStringList& cmd)
	{
		QString modeCmd ("MODE " + cmd.join (" "));
		ISH_->SendCommand (modeCmd);
	}

	void IrcParser::ChannelsListCommand (const QStringList&)
	{
		QString chListCmd ("LIST ");
		ISH_->SendCommand (chListCmd);
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
			qWarning () << "input string is not a valide IRC command"
					<< msg;
			return false;
		}
		else
		{
			IrcMessageOptions_.Nick_ = QString::fromUtf8 (nickStr.c_str ());
			IrcMessageOptions_.Command_ = QString::fromUtf8 (commandStr.c_str ()).toLower ();
			IrcMessageOptions_.Message_ = QString::fromUtf8 (msgStr.c_str ());
			IrcMessageOptions_.UserName_ = QString::fromUtf8 (userStr.c_str ());
			IrcMessageOptions_.Host_ = QString::fromUtf8 (hostStr.c_str ());
			IrcMessageOptions_.Parameters_ = opts;
		}

		return true;
	}

	IrcMessageOptions IrcParser::GetIrcMessageOptions () const
	{
		return IrcMessageOptions_;
	}
}
}
}
