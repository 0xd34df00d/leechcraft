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
	, Prefix_ (QString ())
	, Command_ (QString ())
	, Parameters_ (QStringList ())
	, Nick_ (QString ())
	, User_ (QString ())
	, Host_ (QString ())
	, ServerName (QString ())
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
		QStringList params;
		params << channel.ChannelName_ << mess << IrcServer_->GetNickName ();
		IrcServer_->readMessage (params);
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
		if (Command2Signal_.contains (Command_.toLower ()))
		{
			Parameters_.append (Nick_);
			Command2Signal_ [Command_.toLower ()] (Parameters_);
		}
		Parameters_.clear ();
		Nick_.clear ();
		User_.clear ();
		Host_.clear ();
		ServerName.clear ();
	}

	void IrcParser::LeaveChannelCommand (const QString& channel)
	{
		// TODO leave message 
		QString leaveCmd = QString ("PART " + channel + "\r\n");
		Core::Instance ().GetSocketManager ()->
				SendCommand (leaveCmd, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

	QString IrcParser::GetNickName () const
	{
		return Nick_;
	}

	void IrcParser::Init ()
	{
		Command2Signal_ ["001"] = boost::bind (&IrcParser::gotAuthSuccess, this, _1);
		connect (this,
				SIGNAL (gotAuthSuccess (const QStringList&)),
				IrcServer_,
				SLOT (authFinished (const QStringList&)));

		Command2Signal_ ["005"] = boost::bind (&IrcParser::gotServerSupport, this, _1);
		connect (this,
				SIGNAL (gotServerSupport (const QStringList&)),
				IrcServer_,
				SLOT (setServerSupport (const QStringList&)));

		Command2Signal_ ["332"] = boost::bind (&IrcParser::gotTopic, this, _1);
		Command2Signal_ ["topic"] = boost::bind (&IrcParser::gotTopic, this, _1);
		connect (this,
				SIGNAL (gotTopic (const QStringList&)),
				IrcServer_,
				SLOT (setTopic (const QStringList&)));

		Command2Signal_ ["ping"] = boost::bind (&IrcParser::gotPing, this, _1);
		connect (this,
				SIGNAL (gotPing (const QStringList&)),
				this,
				SLOT (pongCommand (const QStringList&)));

		Command2Signal_ ["353"] = boost::bind (&IrcParser::gotCLEntries, this, _1);
		connect (this,
				SIGNAL (gotCLEntries (const QStringList&)),
				IrcServer_,
				SLOT (setCLEntries (const QStringList&)));

		Command2Signal_ ["privmsg"] = boost::bind (&IrcParser::gotMessage, this, _1);
		connect (this,
				SIGNAL (gotMessage (const QStringList&)),
				IrcServer_,
				SLOT (readMessage (const QStringList&)));

		Command2Signal_ ["join"] = boost::bind (&IrcParser::gotNewParticipant, this, _1);
		connect (this,
				SIGNAL (gotNewParticipant (const QStringList&)),
				IrcServer_,
				SLOT (setNewParticipant (const QStringList&)));

		Command2Signal_ ["part"] = boost::bind (&IrcParser::gotUserLeave, this, _1);
		connect (this,
				SIGNAL (gotUserLeave (const QStringList&)),
				IrcServer_,
				SLOT (setUserLeave (const QStringList&)));

		Command2Signal_ ["quit"] = boost::bind (&IrcParser::gotUserQuit, this, _1);
		connect (this,
				SIGNAL (gotUserQuit (const QStringList&)),
				IrcServer_,
				SLOT (setUserQuit (const QStringList&)));
	}

	void IrcParser::ParseMessage (const QString& message)
	{
		int pos = 0;
		QString msg = message.trimmed ();
		int msg_len = msg.length ();
		if (msg.startsWith (':'))
		{
			int delim = msg.indexOf (' ');
			if (delim == -1)
				delim = msg_len;
			Prefix_ = msg.mid (1, delim - 1);
			pos = delim + 1;
			ParsePrefix (Prefix_);
		}
		else
			Prefix_ = QString ("");
		
		int par_start = msg.indexOf (' ', pos);
		if (par_start == -1)
			par_start = msg_len;
		
		Command_ = msg.mid (pos, par_start - pos);
		pos = par_start + 1;
		
		while (pos < msg_len)
		{
			if (msg [pos] == ':') {
				Parameters_.append (msg.mid (pos + 1));
				break;
			}
			else 
			{
				int nextpos = msg.indexOf (' ',pos);
				if (nextpos == -1)
					nextpos = msg_len;
				Parameters_.append (msg.mid (pos,nextpos - pos));
				pos = nextpos + 1;
			}
		}
	}

	void IrcParser::ParsePrefix (const QString& prefix)
	{
		QRegExp rexp ("([a-zA-Z\\[\\]`_\\^\\{\\|\\}][a-zA-Z0-9\\[\\]`_\\^\\{\\|\\}-]+)!(([^\\0\\r\\n\\s@])+)?@(.+)?");

		if (rexp.indexIn (prefix) != -1)
		{
			Nick_ = rexp.cap (1);
			User_ = rexp.cap (2);
			Host_ = rexp.cap (4);
		}
		else
			ServerName = prefix;
	}

	void IrcParser::pongCommand (const QStringList& params)
	{
		QString pongCmd = QString ("PONG :" + params.at (0) + "\r\n");
		Core::Instance ().GetSocketManager ()
				->SendCommand (pongCmd, IrcServer_->GetHost (), IrcServer_->GetPort ());
	}

};
};
};