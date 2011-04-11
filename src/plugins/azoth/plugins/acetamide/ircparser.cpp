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
#include <boost/bind/bind.hpp>
#include <QTextCodec>
#include "socketmanager.h"
#include "ircserver.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
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
		Command2Signal_ ["001"] = ::boost::bind (&IrcParser::gotAuthSuccess, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotAuthSuccess (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (authFinished (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["005"] = ::boost::bind (&IrcParser::gotServerSupport, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotServerSupport (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setServerSupport (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["332"] = ::boost::bind (&IrcParser::gotTopic, this, _1, _2, _3);
		Command2Signal_ ["topic"] = ::boost::bind (&IrcParser::gotTopic, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotTopic (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setTopic (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["ping"] = ::boost::bind (&IrcParser::gotPing, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotPing (const QString&, const QList<std::string>&, const QString&)),
				this,
				SLOT (pongCommand (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["353"] = ::boost::bind (&IrcParser::gotCLEntries, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotCLEntries (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setCLEntries (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["privmsg"] = ::boost::bind (&IrcParser::gotMessage, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotMessage (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (readMessage (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["join"] = ::boost::bind (&IrcParser::gotNewParticipant, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotNewParticipant (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setNewParticipant (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["part"] = ::boost::bind (&IrcParser::gotUserLeave, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotUserLeave (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setUserLeave (const QString&, const QList<std::string>&, const QString&)));

		Command2Signal_ ["quit"] = ::boost::bind (&IrcParser::gotUserQuit, this, _1, _2, _3);
		connect (this,
				SIGNAL (gotUserQuit (const QString&, const QList<std::string>&, const QString&)),
				IrcServer_,
				SLOT (setUserQuit (const QString&, const QList<std::string>&, const QString&)));
	}

	void IrcParser::ParseMessage (const QString& message)
	{
		QString mess = message.trimmed ();
		std::string str = mess.toStdString ();

		bool res = IrcParserGrammar_.ParseMessage (str);

		if (res)
		{
			IrcMessageStruct ircMessStruct = IrcParserGrammar_.GetMessageStruct ();
			QString cmd = QString::fromUtf8 (ircMessStruct.Command_.c_str ()).toLower ();
			if (Command2Signal_.contains (cmd))
				Command2Signal_ [cmd] (QString::fromUtf8 (ircMessStruct.Nickname_.c_str ()),
						ircMessStruct.Parameters_, QString::fromUtf8 (ircMessStruct.Message_.c_str ()));
		}
		else
		{
			qWarning () << "input string is not a valide IRC command:"
					<< message;
			return;
		}
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
