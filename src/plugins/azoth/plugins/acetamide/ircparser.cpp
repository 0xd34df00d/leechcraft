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
#include <QTextCodec>
#include "ircaccount.h"
#include "socketmanager.h"
#include "clientconnection.h"

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
	{
		Init ();
	}
	
	void IrcParser::JoinChannel (const ServerOptions& server, const ChannelOptions& channel)
	{
		Channel_ = channel;
		
		QString key = server.ServerName_ + ":" + 
				QString::number (server.ServerPort_);
		
		if (!Core::Instance ().GetSocketManager ()->IsConnected (key))
			AuthCommand (server);
		else
			SuccessfulAuth ();
	}

	void IrcParser::SuccessfulAuth ()
	{
// 		//NickCommand ();
// 		
// 		QString joinCmd = QString ("JOIN " + Channel_.ChannelName_ + "\r\n");
// 		Core::Instance ().GetSocketManager ()->SendCommand (joinCmd, 
// 				Server_.ServerName_, Server_.ServerPort_);
	}

	void IrcParser::NickCommand ()
	{
/*		QString nick;
		if (channel.ChannelNickname_.isEmpty ())
			nick = server.ServerNicknames_.at (0);
		else
			nick = server.ServerNicknames_.last ();
		
		QString nickCmd = QString ("NICK " + nick + "\r\n");
		
		Core::Instance ().GetSocketManager ()->SendCommand (nickCmd, Server_.ServerName_, Server_.ServerPort_*/);
	} 

	void IrcParser::PrivMessageCommand(const QString& text, const ServerOptions& server, const ChannelOptions& channel)
	{
		QTextCodec *codec = QTextCodec::codecForName (server.ServerEncoding_.toUtf8 ());
		QString mess =  codec->fromUnicode (text);
		QString msg = QString ("PRIVMSG " + channel.ChannelName_ + " :" + mess + "\r\n");
		Core::Instance ().GetSocketManager ()->SendCommand (msg, Server_.ServerName_, Server_.ServerPort_);
		QString  id = QString ("%1@%2")
					.arg (channel.ChannelName_, channel.ServerName_);
		emit messageReceived (mess, id, server.ServerNicknames_.at (0));
	}


	void IrcParser::Init ()
	{
// 		connect (this,
// 				SIGNAL (readyToReadAnswer (const QString&, const QString&)),
// 				this,
// 				SLOT (handleServerReply (const QString&, const QString&)));
// 		
// 		connect (this,
// 				SIGNAL (gotAuthSuccess ()),
// 				this,
// 				SLOT (SuccessfulAuth ()));
		

		
		//TODO тщательно составить регулярки для всех видов сообщений!!!!
		// PING :calvino.freenode.net
/*		Name2RegExp_ ["ping"] = QRegExp ("^PING\\s:(.+)\r\n$");
		// :calvino.freenode.net 353 leechraft17 = #qt :[[@|+]<nick> [[@|+]<nick> [...]]]
		Name2RegExp_ ["contact_list"] = QRegExp ("^:\\w[\\w,\\.]+\\w\\s353\\s(\\w+)\\s[=,\\*,@]\\s([#,\\+,&,\\!][^\\0\\n\\r\\a ,:]+)\\s:(.+)\r\n$");
		// :calvino.freenode.net 332 leechraft17 #qt :Latest Qt: 4.7.1 | docs: qt.nokia.com/doc/ | #qt-creator | Bugs: bugreports.qt.nokia.com | Off-topic:..
		Name2RegExp_ ["topic"] = QRegExp ("^:\\w[\\w,\\.]+\\w\\s332\\s(\\w+)\\s([#,\\+,&,\\!][^\\0\\n\\r\\a ,:]+)\\s:(.+)\r\n$");
		// :pillar!fd@xob.kapsi.fi PRIVMSG #qt :hey is there some API to open
		Name2RegExp_ ["message"] = QRegExp ("^:(.+)\\!(.+)@(\\w[\\w,\\.]+\\w)\\sPRIVMSG\\s([#,\\+,&,\\!][^\\0\\n\\r\\a ,:]+)\\s:(.+)\r\n$");*/
	}
	
	void IrcParser::InitClientConnection (ClientConnection *cc)
	{
// 		connect (this,
// 				SIGNAL (gotCLEntries (const QString&, const QString&)),
// 				cc,
// 				SLOT (setChannelUseres (const QString&, const QString&)));
// 		
// 		connect (this,
// 				SIGNAL (gotTopic (const QString&, const QString&)),
// 				cc,
// 				SLOT (setSubject (const QString&, const QString&)));
// 		
// 		connect (this,
// 				SIGNAL (messageReceived (const QString&, const QString&, const QString&)),
// 				cc,
// 				SLOT (handleMessageReceived (const QString&, const QString&, const QString&)));
	}

	
	void IrcParser::AuthCommand (const ServerOptions& server)
	{
// I don't understand how to detect that password is true'
// 		if (!server.ServerPassword_.isEmpty ())
// 		{
// 			QString passCmd = QString ("PASS " + server.ServerPassword_ + "\r\n");
// 			Core::Instance ().GetSocketManager ()->SendCommand (passCmd, server.ServerName_, server.ServerPort_);
// 		}
// 		else
			UserCommand (server);
	}
	
	
	
	
	void IrcParser::handleServerReply (const QString& result, const QString& serverKey)
	{
		ParseMessage (result);
		if (Command_.toLower () == "ping")
			PongCommand (Parameters_.at (0), serverKey);
		else if (Command_.toLower () == "001")
			emit gotAuthSuccess ();
			
		
		
// 		QString serv = GetPingServer (result);
// 		qDebug () << result;
// 		if (!serv.isEmpty ())
// 			PongCommand (serv, serverKey);
// 		else if (Name2RegExp_ ["contact_list"].indexIn (result) > -1)
// 		{
// 			QString  id = QString ("%1@%2")
// 					.arg (Name2RegExp_ ["contact_list"].cap (2), serverKey);
// 			emit gotCLEntries (Name2RegExp_ ["contact_list"].cap (3), id);
// 		}
// 		else if (Name2RegExp_ ["topic"].indexIn (result) > -1)
// 		{
// 			QString  id = QString ("%1@%2")
// 					.arg (Name2RegExp_ ["topic"].cap (2), serverKey);
// 			emit gotTopic (Name2RegExp_ ["topic"].cap (3), id);
// 		}
// 		else if (Name2RegExp_ ["message"].indexIn (result) > -1)
// 		{
// 			QString  id = QString ("%1@%2")
// 					.arg (Name2RegExp_ ["message"].cap (4), serverKey);
// 			emit messageReceived (Name2RegExp_ ["message"].cap (5), id, 
// 					Name2RegExp_ ["message"].cap (1));
// 		}
	}

	void IrcParser::UserCommand (const ServerOptions& server)
	{
		QString userCmd = QString ("USER " + 
				server.ServerNicknames_.at (0) + 
				" 0 * :" + 
				server.ServerRealName_ + "\r\n");

		Core::Instance ().GetSocketManager ()->
				SendCommand (userCmd, server, IrcServer_->GetIrcAccount ());
	}



	QString IrcParser::GetPingServer (const QString& msg) const
	{
// 		if (Name2RegExp_ ["ping"].indexIn (msg) > -1)
// 			return Name2RegExp_ ["ping"].cap (1);
// 		return QString ();
	}

	void IrcParser::PongCommand (const QString& server, const QString& serverKey)
	{
		QString pongCmd = QString ("PONG :" + server + "\r\n");
		Core::Instance ().GetSocketManager ()->SendCommand (pongCmd, serverKey, 8001);
	}
	
	void IrcParser::ParseMessage (const QString& msg)
	{
		int pos = 0;
		int msg_len = msg.length ();
		if (msg.startsWith (':'))
		{
			int delim = msg.indexOf (' ');
			if (delim == -1)
				delim = msg_len;
			Prefix_ = msg.mid (1, delim - 1);
			pos = delim + 1;
		}
		else
			Prefix_ = QString ("");
		
		int par_start = msg.indexOf (' ', pos);
		if (par_start == -1)
			par_start = msg_len - pos;
		
		Command_ = msg.mid (pos, par_start);
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
					nextpos = msg_len - pos;
				Parameters_.append (msg.mid (pos,nextpos));
				pos = nextpos + 1;
			}
		}
	}
};
};
};