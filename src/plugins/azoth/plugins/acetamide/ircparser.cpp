/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "ircparser.h"
#include <boost/bind.hpp>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <QTextCodec>
#include "config.h"
#include "socketmanager.h"
#include "ircserver.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	using namespace boost::spirit::classic;

	IrcParser::IrcParser (IrcServer *server)
	: IrcServer_ (server)
	{
		Init ();
	}

	void IrcParser::AuthCommand (const ServerOptions& server)
	{
		if (!server.ServerPassword_.isEmpty ())
		{
			QString passCmd = QString ("PASS " + server.ServerPassword_ + "\r\n");
			Core::Instance ().GetSocketManager ()->
					SendCommand (passCmd, server.ServerName_, server.ServerPort_);
		}

		UserCommand (server);
		NickCommand (server);
	}

	void IrcParser::UserCommand (const ServerOptions& server)
	{
		QString userCmd = QString ("USER " +
				server.ServerNicknames_.at (0) +
				" 0 * :" +
				server.ServerRealName_ + "\r\n");

		Core::Instance ().GetSocketManager ()->
				SendCommand (userCmd, server.ServerName_, server.ServerPort_);
	}

	void IrcParser::NickCommand (const ServerOptions& server)
	{
		QString nickCmd = QString ("NICK " + server.ServerNicknames_.at (0) + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (nickCmd, server.ServerName_, server.ServerPort_);
	}

	void IrcParser::JoinChannel (const ChannelOptions& channel)
	{
		QString joinCmd = QString ("JOIN " + channel.ChannelName_ + "\r\n");
		Core::Instance ().GetSocketManager ()->SendCommand (joinCmd,
				IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	void IrcParser::PublicMessageCommand (const QString& text, const ChannelOptions& channel)
	{
		QTextCodec *codec = QTextCodec::codecForName (IrcServer_->GetEncoding ().toUtf8 ());
		QString mess =  codec->fromUnicode (text);
		QString msg = QString ("PRIVMSG " + channel.ChannelName_ + " :" + mess + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (msg, IrcServer_->GetHost (), IrcServer_->GetPort ());

		IrcServer_->readMessage (IrcServer_->GetNickName (),
				QList<std::string> () << channel.ChannelName_.toUtf8 ().constData (), mess);
	}

	void IrcParser::PrivateMessageCommand (const QString& message, const QString& target)
	{
		QTextCodec *codec = QTextCodec::codecForName (IrcServer_->GetEncoding ().toUtf8 ());
		QString mess =  codec->fromUnicode (message);
		QString msg = QString ("PRIVMSG " + target + " :" + mess + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (msg, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	void IrcParser::HandleServerReply (const QString& result)
	{
		ParseMessage (result);
	}

	void IrcParser::LeaveChannelCommand (const QString& channel)
	{
		// TODO leave message
		QString leaveCmd = QString ("PART " + channel + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (leaveCmd, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	void IrcParser::QuitConnectionCommand (const QString& msg)
	{
		QString msgCmd = msg;
		if (!msg.isEmpty ())
			msgCmd = ":" + msg;
		QString quitCmd = QString ("QUIT " + msgCmd + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (quitCmd, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	void IrcParser::Init ()
	{
		Command2Signal_ ["001"] = boost::bind (&IrcParser::gotAuthSuccess, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotAuthSuccess (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (authFinished (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["005"] = boost::bind (&IrcParser::gotServerSupport, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotServerSupport (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setServerSupport (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["332"] = boost::bind (&IrcParser::gotTopic, this, _1, _2, _3);
		Command2Signal_ ["topic"] = boost::bind (&IrcParser::gotTopic, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotTopic (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setTopic (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["ping"] = boost::bind (&IrcParser::gotPing, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotPing (const QString&, const QList<std::string>&, const QString&)),
				this,
				SLOT (pongCommand (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["353"] = boost::bind (&IrcParser::gotCLEntries, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotCLEntries (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setCLEntries (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["privmsg"] = boost::bind (&IrcParser::gotMessage, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotMessage (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (readMessage (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["join"] = boost::bind (&IrcParser::gotNewParticipant, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotNewParticipant (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setNewParticipant (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["part"] = boost::bind (&IrcParser::gotUserLeave, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotUserLeave (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setUserLeave (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["quit"] = boost::bind (&IrcParser::gotUserQuit, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotUserQuit (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setUserQuit (const QString&, const QList<std::string>&, const QString&)));
	}

	void IrcParser::ParseMessage (const QString& message)
	{
		std::string nickStr = std::string ();
		std::string commandStr = std::string ();
		std::string msgStr = std::string ();
		QList<std::string> opts;

		range<> ascii (char (0x01), char (0x7F));
		rule<> special = lexeme_d [ch_p ('[') | ']' | '\\' | '`' | '_' | '^' | '{' | '|' | '}'];
		rule<> shortname = *(alnum_p >> *(alnum_p || ch_p ('-')) >> *alnum_p);
		rule<> hostname = shortname >> *(ch_p ('.') >> shortname);
		rule<> nickname = (alpha_p | special) >> * (alnum_p | special | ch_p ('-'));
		rule<> user =  +(ascii - '\r' - '\n' - ' ' - '@' - '\0');
		rule<> host = lexeme_d [+(anychar_p - ' ')] ;
		rule<> nick = lexeme_d [nickname [assign_a (nickStr)] >> !(!(ch_p ('!')
				>> user) >> ch_p ('@') >> host)];
		rule<> nospcrlfcl = (anychar_p - '\0' - '\r' - '\n' - ' ' - ':');
		rule<> lastParam = lexeme_d [ch_p (' ') >> !ch_p (':') >> (*(ch_p (':') | ch_p (' ') | nospcrlfcl)) [assign_a (msgStr)]];
		rule<> firsParam = lexeme_d [ch_p (' ') >> (nospcrlfcl >> *(ch_p (':') | nospcrlfcl))[push_back_a (opts)]];
		rule<> params =  *firsParam >> !lastParam;
		rule<> command = longest_d [(+alpha_p) | (repeat_p (3) [digit_p])][assign_a (commandStr)];
		rule<> prefix = longest_d [hostname | nick];
		rule<> reply = (lexeme_d [!(ch_p (':') >> prefix >> ch_p (' '))] >> command >> !params >> eol_p);

		bool res = parse (message.toUtf8 ().constData (), reply).full;

		if (!res)
		{
			qWarning () << "input string is not a valide IRC command"
					<< message;
			return;
		}
		else
		{
			QString cmd = QString::fromUtf8 (commandStr.c_str ()).toLower ();
			QString msg = QString::fromUtf8 (msgStr.c_str ());
			QString nick = QString::fromUtf8 (nickStr.c_str ());
			bool result = false;
			if (cmd == "privmsg")
				result = IsCTCPMessage (msg, nick);
			if (!result)
				if (Command2Signal_.contains (cmd))
					Command2Signal_ [cmd] (nick, opts, msg);
		}
	}

	bool IrcParser::IsCTCPMessage (const QString& msg, const QString& nick)
	{
		if (msg.startsWith ('\001') && msg.endsWith ('\001'))
		{
			QString msg_1 = msg.mid (1, msg.length () - 1);
			QRegExp rxp("([a-zA-Z]+)(\\s.*)?");
			if (rxp.indexIn (msg_1) > -1)
				CTCPAnswer (rxp.cap (1), rxp.cap (2), nick);
			return true;
		}

		return false;
	}

	void IrcParser::CTCPAnswer (const QString& command, const QString& attributs, const QString& nick)
	{
		QString cmd;
		if (command.toLower () == "version")
		{
			QString version = QString ("%1 %2").arg ("LeechCraft Azoth", LEECHCRAFT_VERSION);
			cmd = QString ("%1 %2%3").arg ("\001VERSION", version, QChar ('\001'));
		}
		else if (command.toLower () == "ping")
			cmd = QString ("%1 %2%3").arg ("\001PING", QDateTime::currentDateTime ().toTime_t (), QChar ('\001'));
		else if (command.toLower () == "time")
			cmd = QString ("%1 %2%3").arg ("\001TIME", QDateTime::currentDateTime ().toString ("ddd MMM dd hh:mm:ss yyyy"), QChar ('\001'));

		QString ctcpCommand = QString ("NOTICE " + nick + " :" + cmd + "\r\n");
		Core::Instance ().GetSocketManager ()
				->SendCommand (ctcpCommand, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	void IrcParser::pongCommand (const QString&, const QList<std::string>&, const QString& msg)
	{
		QString pongCmd = QString ("PONG :" + msg + "\r\n");
		Core::Instance ().GetSocketManager ()
				->SendCommand (pongCmd, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

};
};
};
