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

#include "ircclient.h"
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
	IrcClient::IrcClient (QObject *parent)
	: QObject (parent)
	, incr (0)
	{
		SocketManager_.reset (new SocketManager (this));
		
		connect (this,
			SIGNAL (readyToReadAnswer (const QString&, const QString&)),
			this,
			SLOT (handleServerReply (const QString&, const QString&)));
		
		Init ();
	}
	
	void IrcClient::JoinChannel (const ServerOptions& server, const ChannelOptions& channel, ClientConnection *cc)
	{
		QString key = server.ServerName_ + ":" + 
				QString::number (server.ServerPort_);
		
		if (!SocketManager_->IsConnected (key))
			AuthCommand (server, channel);
		
		NickCommand (server, channel);
		
		QString joinCmd = QString ("JOIN " + 
				channel.ChannelName_ + "\r\n");

		InitClientConnection (cc);

		SocketManager_->SendCommand (server, channel, joinCmd);
	}
	
	void IrcClient::NickCommand(const ServerOptions& server, const ChannelOptions& channel)
	{
		QString nick;
		if (channel.ChannelNickname_.isEmpty ())
			nick = server.ServerNicknames_.at (0);
		else
			nick = server.ServerNicknames_.last ();
		
		QString nickCmd = QString ("NICK " + nick + "\r\n");
		
		SocketManager_->SendCommand (server, channel, nickCmd);
	} 

	void IrcClient::PrivMessageCommand(const QString& text, const ServerOptions& server, const ChannelOptions& channel)
	{
		QTextCodec *codec = QTextCodec::codecForName (server.ServerEncoding_.toUtf8 ());
		QString mess =  codec->fromUnicode (text);
		QString msg = QString ("PRIVMSG " + channel.ChannelName_ + " :" + mess + "\r\n");
		SocketManager_->SendCommand (server, channel, msg);
		QString  id = QString ("%1@%2")
					.arg (channel.ChannelName_, channel.ServerName_);
		emit messageReceived (mess, id, server.ServerNicknames_.at (0));
	}


	void IrcClient::Init ()
	{
		//TODO тщательно составить регулярки для всех видов сообщений!!!!
		// PING :calvino.freenode.net
		Name2RegExp_ ["ping"] = QRegExp ("^PING\\s:\\w([\\w,\\.]+\\w)\r\n$");
		// :calvino.freenode.net 353 leechraft17 = #qt :[[@|+]<nick> [[@|+]<nick> [...]]]
		Name2RegExp_ ["contact_list"] = QRegExp ("^:\\w[\\w,\\.]+\\w\\s353\\s(\\w+)\\s[=,\\*,@]\\s([#,\\+,&,\\!]\\w+)\\s:(.+)\\r\\n$");
		// :calvino.freenode.net 332 leechraft17 #qt :Latest Qt: 4.7.1 | docs: qt.nokia.com/doc/ | #qt-creator | Bugs: bugreports.qt.nokia.com | Off-topic:..
		Name2RegExp_ ["topic"] = QRegExp ("^:\\w[\\w,\\.]+\\w\\s332\\s(\\w+)\\s([#,\\+,&,\\!]\\w+)\\s:(.+)\\r\\n$");
		// :pillar!fd@xob.kapsi.fi PRIVMSG #qt :hey is there some API to open
		Name2RegExp_ ["message"] = QRegExp ("^:(.+)\\!(.+)@(\\w[\\w,\\.]+\\w)\\sPRIVMSG\\s([#,\\+,&,\\!]\\w+)\\s:(.+)\\r\\n$");
	}
	
	void IrcClient::InitClientConnection (ClientConnection *cc)
	{
		connect (this,
				SIGNAL (gotCLEntries (const QString&, const QString&)),
				cc,
				SLOT (setChannelUseres (const QString&, const QString&)));
		
		connect (this,
				SIGNAL (gotTopic (const QString&, const QString&)),
				cc,
				SLOT (setSubject (const QString&, const QString&)));
		
		connect (this,
				SIGNAL (messageReceived (const QString&, const QString&, const QString&)),
				cc,
				SLOT (handleMessageReceived (const QString&, const QString&, const QString&)));
	}

	
	void IrcClient::AuthCommand (const ServerOptions& server, const ChannelOptions& channel)
	{
		QString nick;
		if (channel.ChannelNickname_.isEmpty ())
			nick = server.ServerNicknames_.at (0);
		else
			nick = server.ServerNicknames_.last ();
		
		if (!server.ServerPassword_.isEmpty ())
		{
			QString passCmd = QString ("PASS " + server.ServerPassword_ + "\r\n");
		
			SocketManager_->SendCommand (server, channel, passCmd);
		}
		
		QString userCmd = QString ("USER " + 
				nick + 
				" 0 * :" + 
				server.ServerRealName_ + "\r\n");
		
		SocketManager_->SendCommand (server, channel, userCmd);
	}
	
	void IrcClient::handleServerReply (const QString& result, const QString& serverKey)
	{
		QString serv = GetPingServer (result);
		qDebug () << result;
		if (!serv.isEmpty ())
			PongCommand (serv, serverKey);
		else if (Name2RegExp_ ["contact_list"].indexIn (result) > -1)
		{
			QString  id = QString ("%1@%2")
					.arg (Name2RegExp_ ["contact_list"].cap (2), serverKey);
			emit gotCLEntries (Name2RegExp_ ["contact_list"].cap (3), id);
		}
		else if (Name2RegExp_ ["topic"].indexIn (result) > -1)
		{
			QString  id = QString ("%1@%2")
					.arg (Name2RegExp_ ["topic"].cap (2), serverKey);
			emit gotTopic (Name2RegExp_ ["topic"].cap (3), id);
		}
		else if (Name2RegExp_ ["message"].indexIn (result) > -1)
		{
			QString  id = QString ("%1@%2")
					.arg (Name2RegExp_ ["message"].cap (4), serverKey);
			emit messageReceived (Name2RegExp_ ["message"].cap (5), id, 
					Name2RegExp_ ["message"].cap (1));
		}
	}

	QString IrcClient::GetPingServer (const QString& msg) const
	{
		if (Name2RegExp_ ["ping"].indexIn (msg) > -1)
			return Name2RegExp_ ["ping"].cap (1);
		
		return QString ();
	}

	void IrcClient::PongCommand (const QString& server, const QString& serverKey)
	{
		QString pongCmd = QString ("PONG :" + server + "\\r\\n");
		SocketManager_->SendCommand (pongCmd, 
				serverKey.split (':').at (0), serverKey.split (':').at (1).toInt ());
	}
};
};
};