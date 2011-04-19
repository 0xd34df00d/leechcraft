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

#include "ircserverhandler.h"
#include <boost/bind.hpp>
#include <plugininterface/util.h>
#include <plugininterface/notificationactionhandler.h>
#include "ircaccount.h"
#include "ircmessage.h"
#include "ircparser.h"
#include "ircserverclentry.h"
#include "channelhandler.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	IrcServerHandler::IrcServerHandler (const ServerOptions& server,
			IrcAccount *account )
	: Account_ (account)
	, ServerOptions_ (server)
	, ServerCLEntry_ (new IrcServerCLEntry (this, account))
	, ServerID_ (server.ServerName_ + ":" +
			QString::number (server.ServerPort_))
	, ServerConnectionState_ (NotConnected)
	, NickName_ (account->GetOurNick ())
	{
		IrcParser_ = new IrcParser (this);
		InitErrorsReplys ();
	}

	IrcServerCLEntry* IrcServerHandler::GetCLEntry () const
	{
		return ServerCLEntry_;
	}

	IrcAccount* IrcServerHandler::GetAccount () const
	{
		return Account_;
	}

	QString IrcServerHandler::GetNickName () const
	{
		return NickName_;
	}

	QString IrcServerHandler::GetServerID_ () const
	{
		return ServerID_;
	}

	ServerOptions IrcServerHandler::GetServerOptions () const
	{
		return ServerOptions_;
	}

	ConnectionState IrcServerHandler::GetConnectionState () const
	{
		return ServerConnectionState_;
	}

	bool IrcServerHandler::IsChannelExists (const QString& channelID)
	{
		return ChannelHandlers_.contains (channelID);
	}

	ChannelHandler*
			IrcServerHandler::GetChannelHandler (const QString& id)
	{
		return ChannelHandlers_.contains (id) ?
				ChannelHandlers_ [id] :
				0;
	}

	QList<ChannelHandler*> IrcServerHandler::GetChannelHandlers () const
	{
		return ChannelHandlers_.values ();
	}

	bool IrcServerHandler::ConnectToServer ()
	{
		if (ServerConnectionState_ == NotConnected)
		{
			TcpSocket_ptr.reset (new QTcpSocket (this));
			ServerConnectionState_ = InProcess;

			TcpSocket_ptr->connectToHost (ServerOptions_.ServerName_,
					ServerOptions_.ServerPort_);

			if (!TcpSocket_ptr->waitForConnected(30000))
			{
				ServerConnectionState_ = NotConnected;
				qDebug () << Q_FUNC_INFO
						<< "cannot to connect to host"
						<< ServerID_;
				return false;
			}

			ServerConnectionState_ = Connected;
			ServerCLEntry_->
					SetStatus (EntryStatus (SOnline, QString ()));
			InitSocket ();
			IrcParser_->AuthCommand ();
		}
		return true;
	}

	bool IrcServerHandler::JoinChannel (const ChannelOptions& channel)
	{
		QString id = channel.ChannelName_ + "@" + channel.ServerName_;
		if (!ChannelHandlers_.contains (id))
		{
			ChannelHandler *ch = new ChannelHandler (this, channel);
			ChannelHandlers_ [id] = ch;
			IrcParser_->JoinCommand (channel.ChannelName_);
		}
		return true;
	}

	void IrcServerHandler::SendCommand (const QString& cmd)
	{
		qDebug () << TcpSocket_ptr.get () << cmd;
		if (!TcpSocket_ptr->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}

		if (TcpSocket_ptr->write (cmd.toAscii ()) == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< TcpSocket_ptr->error ()
					<< TcpSocket_ptr->errorString ();
			return;
		}
	}

	void IrcServerHandler::InboxMessage2Server ()
	{
		IrcMessage *msg = new IrcMessage (
				IMessage::MTServiceMessage,
				IMessage::DIn,
				ServerID_,
				QString (),
				Account_->GetClientConnection ().get ());

		msg->SetBody (IrcParser_-> GetIrcMessageOptions ().Message_);
		msg->SetDateTime (QDateTime::currentDateTime ());

		ServerCLEntry_->HandleMessage (msg);
	}

	void IrcServerHandler::InitErrorsReplys ()
	{
		Error2Action_ ["401"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["402"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["403"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["404"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["405"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["406"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["407"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["408"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["409"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["411"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["412"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["413"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["414"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["415"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["421"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["422"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["424"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["431"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["432"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["433"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["436"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["437"] =
				boost::bind (&IrcServerHandler::NickCmdError, this);
		Error2Action_ ["441"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["442"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["443"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["444"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["445"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["446"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["451"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["461"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["462"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["463"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["464"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["465"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["466"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["467"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["471"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["472"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["473"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["474"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["475"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["476"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["477"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["478"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["481"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["482"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["483"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["484"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["485"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["491"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["501"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
		Error2Action_ ["502"] =
				boost::bind (&IrcServerHandler::NoSuchNickError, this);
	}

	void IrcServerHandler::NoSuchNickError ()
	{

	}

	void IrcServerHandler::NickCmdError ()
	{
		int index = Account_->GetNickNames ().indexOf (NickName_);
		if (index < Account_->GetNickNames ().count ())
		{
			NickName_ = Account_->GetNickNames ().at (++index);
			IrcParser_->NickCommand ();
		}
	}

	void IrcServerHandler::InitSocket ()
	{
		connect (TcpSocket_ptr.get (),
				SIGNAL (readyRead ()),
				this,
				SLOT (readReply ()));
	}

	bool IrcServerHandler::IsErrorReply (const QString& cmd)
	{
		return Error2Action_.contains (cmd);
	}

	void IrcServerHandler::readReply ()
	{
		while (TcpSocket_ptr->canReadLine ())
		{
			QString str = TcpSocket_ptr->readLine ();

			if (!IrcParser_->ParseMessage (str))
				return;

			QString cmd = IrcParser_->GetIrcMessageOptions ().Command_;
			if (IsErrorReply (cmd))
			{
				QString msg = IrcParser_->GetIrcMessageOptions ()
						.Message_ + QString::fromUtf8 (IrcParser_->
						GetIrcMessageOptions ().Parameters_.last ()
							.c_str ());
				Entity e = Util::MakeNotification ("Azoth",
						msg,
						PInfo_);
				Core::Instance ().SendEntity (e);
				Error2Action_ [cmd] ();
			}
			if (!ActiveChannels_.count ())
				InboxMessage2Server ();
		}
	}


};
};
};