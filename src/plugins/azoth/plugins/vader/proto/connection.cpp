/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "connection.h"
#include <QSslSocket>
#include <QTimer>
#include "conversions.h"
#include "packet.h"
#include "exceptions.h"
#include "message.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	Connection::Connection (QObject *parent)
	: QObject (parent)
	, Socket_ (new QSslSocket (this))
	, PingTimer_ (new QTimer (this))
	, Host_ ("94.100.187.24")
	, Port_ (443)
	, IsConnected_ (false)
	{
		connect (Socket_,
				SIGNAL (sslErrors (const QList<QSslError>&)),
				Socket_,
				SLOT (ignoreSslErrors ()));
		connect (Socket_,
				SIGNAL (connected ()),
				this,
				SLOT (greet ()));
		connect (Socket_,
				SIGNAL (readyRead ()),
				this,
				SLOT (tryRead ()));
		connect (Socket_,
				SIGNAL (error (QAbstractSocket::SocketError)),
				this,
				SLOT (handleSocketError (QAbstractSocket::SocketError)));

		connect (PingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handlePing ()));

		PacketActors_ [Packets::HelloAck] = [this] (HalfPacket hp) { HandleHello (hp); Login (); };
		PacketActors_ [Packets::LoginAck] = [this] (HalfPacket hp) { CorrectAuth (hp); };
		PacketActors_ [Packets::LoginRej] = [this] (HalfPacket hp) { IncorrectAuth (hp); };
		PacketActors_ [Packets::ConnParams] = [this] (HalfPacket hp) { ConnParams (hp); };

		PacketActors_ [Packets::UserInfo] = [this] (HalfPacket hp) { UserInfo (hp); };
		PacketActors_ [Packets::UserStatus] = [this] (HalfPacket hp) { UserStatus (hp); };
		PacketActors_ [Packets::ContactList2] = [this] (HalfPacket hp) { ContactList (hp); };

		PacketActors_ [Packets::MsgAck] = [this] (HalfPacket hp) { IncomingMsg (hp); };
		PacketActors_ [Packets::MsgStatus] = [this] (HalfPacket hp) { MsgStatus (hp); };
		PacketActors_ [Packets::OfflineMsgAck] = [this] (HalfPacket hp) { OfflineMsg (hp); };

		PacketActors_ [Packets::AuthorizeAck] = [this] (HalfPacket hp) { AuthAck (hp); };
		PacketActors_ [Packets::ContactAck] = [this] (HalfPacket hp) { ContactAdded (hp); };
	}

	void Connection::SetTarget (const QString& host, int port)
	{
		Host_ = host;
		Port_ = port;
	}

	void Connection::SetCredentials (const QString& login, const QString& pass)
	{
		Login_ = login;
		Pass_ = pass;
	}

	void Connection::tryRead ()
	{
		PE_ += Read ();

		auto defaultActor = [] (HalfPacket hp)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown packet type"
					<< hp.Header_.MsgType_;
		};

		while (PE_.MayGetPacket ())
		{
			const auto& hp = PE_.GetPacket ();

			try
			{
				PacketActors_.value (hp.Header_.MsgType_, defaultActor) (hp);
			}
			catch (const std::exception& e)
			{
				qDebug () << Q_FUNC_INFO
						<< "error parsing packet:"
						<< e.what ();
			}

			if (Socket_->bytesAvailable ())
				PE_ += Read ();
		}
	}

	bool Connection::IsConnected () const
	{
		return IsConnected_;
	}

	void Connection::Connect ()
	{
		if (Socket_->isOpen ())
			Socket_->disconnectFromHost ();

		Socket_->connectToHost (Host_, Port_);
	}

	void Connection::SetState (const EntryStatus& status)
	{
		if (IsConnected_ && status.State_ == SOffline)
		{
			Disconnect ();
		}
		else if (!IsConnected_ && status.State_ != SOffline)
		{
			Connect ();
			PendingStatus_ = status;
		}
		else if (status.State_ != SOffline)
		{
			const quint32 state = PendingStatus_.State_ == SOnline ?
				UserState::Online :
				UserState::Away;
			Write (PF_.SetStatus (state, status.StatusString_).Packet_);
		}
	}

	quint32 Connection::SendMessage (const QString& to, const QString& message)
	{
		auto hp = PF_.Message (0, to, message);
		Write (hp.Packet_);
		return hp.Seq_;
	}

	void Connection::Authorize (const QString& email)
	{
		Write (PF_.Authorize (email).Packet_);
	}

	quint32 Connection::AddContact (quint32 group, const QString& email, const QString& name)
	{
		const auto& p = PF_.AddContact (0, group, email, name);
		Write (p.Packet_);
		return p.Seq_;
	}

	void Connection::ModifyContact (quint32 contactId,
			quint32 groupId, const QString& email, const QString& name)
	{
		const auto& p = PF_.ModifyContact (contactId,
				ContactOpFlag::None, groupId, email, name);
		Write (p.Packet_);
	}

	void Connection::RemoveContact (quint32 id, const QString& email, const QString& name)
	{
		Write (PF_.RemoveContact (id, email, name).Packet_);
	}

	void Connection::RequestAuth (const QString& email, const QString& msg)
	{
		Write (PF_.Message (MsgFlag::Authorize | MsgFlag::NoRecv, email, msg).Packet_);
	}

	quint32 Connection::AddGroup (const QString& group, int groupNum)
	{
		const auto& p = PF_.AddGroup (group, groupNum);
		Write (p.Packet_);
		return p.Seq_;
	}

	void Connection::HandleHello (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO;
		quint32 timeout;
		FromMRIM (hp.Data_, timeout);

		PingTimer_->start (timeout * 1000);
	}

	void Connection::Login ()
	{
		const quint32 state = PendingStatus_.State_ == SOnline ?
				UserState::Online :
				UserState::Away;
		Write (PF_.Login (Login_, Pass_, state,
					PendingStatus_.StatusString_, "LeechCraft Azoth Vader").Packet_);
	}

	void Connection::CorrectAuth (HalfPacket)
	{
		qDebug () << Q_FUNC_INFO;
		emit statusChanged (PendingStatus_);
	}

	void Connection::IncorrectAuth (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO;
		Str1251 string;
		FromMRIM (hp.Data_, string);
		qDebug () << string;

		Disconnect ();

		emit authenticationError (string);
	}

	void Connection::ConnParams (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO;
		quint32 timeout;
		FromMRIM (hp.Data_, timeout);

		PingTimer_->stop ();
		PingTimer_->start (timeout * 1000);
	}

	void Connection::UserInfo (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO << hp.Data_.size ();

		QMap<QString, QString> info;
		while (!hp.Data_.isEmpty ())
		{
			try
			{
				Str1251 key;
				Str16 value;
				FromMRIM (hp.Data_, key, value);
				info [key] = value;
			}
			catch (const TooShortBA&)
			{
				break;
			}
		}
	}

	void Connection::UserStatus (HalfPacket hp)
	{
		quint32 statusId = 0;
		Str1251 uri;
		Str16 title, desc;
		Str1251 email;
		quint32 features = 0;
		Str1251 ua;

		FromMRIM (hp.Data_, statusId, uri, title, desc, email, features, ua);

		qDebug () << Q_FUNC_INFO << statusId << email << uri << title << desc << ua;

		emit userStatusChanged ({-1, 0, statusId, email,
				QString (), title, desc, features, ua});
	}

	void Connection::ContactList (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO << hp.Data_.size ();
		quint32 result = 0;
		FromMRIM (hp.Data_, result);

		switch (result)
		{
		case CLResponse::IntErr:
			qWarning () << Q_FUNC_INFO
					<< "internal server error";
			return;
		case CLResponse::Error:
			qWarning () << Q_FUNC_INFO
					<< "error";
			return;
		case CLResponse::OK:
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown response code"
					<< result;
			return;
		}

		quint32 groupsNum = 0;
		QByteArray gMask, cMask;
		FromMRIM (hp.Data_, groupsNum, gMask, cMask);

		qDebug () << groupsNum << "groups; masks:" << gMask << cMask;

		auto skip = [&hp] (const QByteArray& mask)
		{
			for (int i = 0; i < mask.size (); ++i)
				switch (mask [i])
				{
				case 'u':
				{
					quint32 dummy;
					FromMRIM (hp.Data_, dummy);
					break;
				}
				case 's':
				{
					QByteArray ba;
					FromMRIM (hp.Data_, ba);
					break;
				}
				}
		};

		gMask = gMask.mid (2);
		QStringList groups;
		for (quint32 i = 0; i < groupsNum; ++i)
		{
			quint32 flags = 0;
			Str16 name;
			FromMRIM (hp.Data_, flags, name);
			groups << name;

			qDebug () << "got group" << name << flags;
			try
			{
				skip (gMask);
			}
			catch (const TooShortBA&)
			{
				qDebug () << "got premature end in additional groups part, but that's OK";
			}
		}
		emit gotGroups (groups);

		cMask = cMask.mid (12);
		QList<ContactInfo> contacts;
		quint32 contactId = 20;
		while (!hp.Data_.isEmpty ())
		{
			try
			{
				quint32 flags = 0, group = 0;
				Str1251 email;
				Str16 alias;
				quint32 serverFlags = 0, status = 0;
				Str1251 phones, statusURI;
				Str16 statusTitle, statusDesc;
				quint32 comSupport = 0;
				Str1251 ua;

				FromMRIM (hp.Data_, flags, group, email, alias, serverFlags,
						status, phones, statusURI, statusTitle, statusDesc, comSupport, ua);

				qDebug () << "got buddy" << flags << group << email << alias
						<< serverFlags << status << phones << statusURI
						<< statusTitle << statusDesc << comSupport << ua;

				contacts << ContactInfo { contactId++, group, status, email, alias, statusTitle, statusDesc, comSupport, ua };

				try
				{
					skip (cMask);
				}
				catch (const TooShortBA&)
				{
					qDebug () << "got premature end in additional CL part, but that's OK";
				}
			}
			catch (const TooShortBA&)
			{
				break;
			}
		}
		emit gotContacts (contacts);
	}

	void Connection::IncomingMsg (HalfPacket hp)
	{
		quint32 msgId = 0, flags = 0;
		Str1251 from;
		FromMRIM (hp.Data_, msgId, flags, from);

		QByteArray textBA;
		FromMRIM (hp.Data_, textBA);
		const QString& text = (flags & MsgFlag::CP1251) ?
				FromMRIM1251 (textBA) :
				FromMRIM16 (textBA);

		qDebug () << Q_FUNC_INFO << from << text << (flags & MsgFlag::NoRecv);

		if (!(flags & MsgFlag::NoRecv))
			Write (PF_.MessageAck (from, msgId).Packet_);

		if (flags & MsgFlag::Authorize)
			emit gotAuthRequest (from, text);
		else if (flags & MsgFlag::Notify)
		{
		}
		else if (flags & MsgFlag::Alarm)
			emit gotAttentionRequest (from, text);
		else if (flags & MsgFlag::Multichat)
		{
		}
		else
			emit gotMessage ({msgId, flags, from, text});
	}

	void Connection::MsgStatus (HalfPacket hp)
	{
		quint32 seq = hp.Header_.Seq_;
		quint32 status = 0;
		FromMRIM (hp.Data_, status);

		if (status == MessageStatus::Delivered)
			emit messageDelivered (seq);
	}

	void Connection::OfflineMsg (HalfPacket hp)
	{
		UIDL id;
		Str1251 message;
		FromMRIM (hp.Data_, id, message);
		// TODO
	}

	void Connection::AuthAck (HalfPacket hp)
	{
		Str1251 from;
		FromMRIM (hp.Data_, from);

		emit gotAuthAck (from);
	}

	void Connection::ContactAdded (HalfPacket hp)
	{
		quint32 status = 0, contactId = 0;
		FromMRIM (hp.Data_, status, contactId);

		qDebug () << Q_FUNC_INFO << hp.Header_.Seq_ << status << contactId;

		if (status == Proto::ContactAck::Success)
			emit contactAdded (hp.Header_.Seq_, contactId);
		else
			emit contactAdditionError (hp.Header_.Seq_, status);
	}

	void Connection::Disconnect ()
	{
		PingTimer_->stop ();
		Socket_->disconnectFromHost ();

		PE_.Clear ();

		IsConnected_ = false;
	}

	QByteArray Connection::Read ()
	{
		QByteArray res = Socket_->readAll ();
		qDebug () << "MRIM READ" << res.toBase64 ();
		return res;
	}

	void Connection::Write (const QByteArray& ba)
	{
		qDebug () << "MRIM WRITE" << ba.toBase64 ();
		Socket_->write (ba);
		Socket_->flush ();
	}

	void Connection::greet ()
	{
		IsConnected_ = true;
		Write (PF_.Hello ().Packet_);
	}

	void Connection::handlePing ()
	{
		Write (PF_.Ping ().Packet_);
	}

	void Connection::handleSocketError (QAbstractSocket::SocketError err)
	{
		qWarning () << Q_FUNC_INFO << err << Socket_->errorString ();
		qWarning () << "was connected with" << Host_ << Port_ << "as" << Login_;
		IsConnected_ = false;
	}
}
}
}
}
