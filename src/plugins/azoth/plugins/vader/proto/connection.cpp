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
#include "packet.h"
#include "exceptions.h"

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
	, Port_ (0)
	, Socket_ (new QSslSocket (this))
	, PingTimer_ (new QTimer (this))
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

		PacketActors_ [Packets::UserInfo] = [this] (HalfPacket hp) { UserInfo (hp); };
		PacketActors_ [Packets::ContactList2] = [this] (HalfPacket hp) { ContactList (hp); };
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

			PacketActors_.value (hp.Header_.MsgType_, defaultActor) (hp);

			if (Socket_->bytesAvailable ())
				PE_ += Read ();
		}
	}

	void Connection::Connect ()
	{
		if (Socket_->isOpen ())
			Socket_->disconnectFromHost ();

		Socket_->connectToHost (Host_, Port_);
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
		Write (PF_.Login (Login_, Pass_, UserState::Online, "LeechCraft Azoth Vader").Packet_);
	}

	void Connection::CorrectAuth (HalfPacket)
	{
		qDebug () << Q_FUNC_INFO;
	}

	void Connection::IncorrectAuth (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO;
		QByteArray string;
		FromMRIM (hp.Data_, string);
		qDebug () << string;

		Disconnect ();
	}

	void Connection::UserInfo (HalfPacket hp)
	{
		qDebug () << Q_FUNC_INFO << hp.Data_.size ();

		QMap<QString, QString> info;
		while (!hp.Data_.isEmpty ())
		{
			try
			{
				QByteArray keyBA, valueBA;
				FromMRIM (hp.Data_, keyBA, valueBA);

				const QString& key = FromMRIM1251 (keyBA);
				const QString& val = FromMRIM16 (valueBA);
				info [key] = val;
			}
			catch (const TooShortBA&)
			{
				break;
			}
		}
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
		gMask = gMask.mid (2);
		cMask = cMask.mid (6);

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

		for (quint32 i = 0; i < groupsNum; ++i)
		{
			quint32 flags = 0;
			QByteArray nameBA;
			FromMRIM (hp.Data_, flags, nameBA);
			const QString& name = FromMRIM16 (nameBA);

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

		while (!hp.Data_.isEmpty ())
		{
			try
			{
				quint32 flags = 0, group = 0, serverFlags = 0, status = 0;
				QByteArray emailBA, aliasBA;
				FromMRIM (hp.Data_, flags, group, emailBA, aliasBA, serverFlags, status);
				const QString& email = FromMRIM1251 (emailBA);
				const QString& alias = FromMRIM16 (aliasBA);

				qDebug () << "got buddy" << flags << group << email << alias << serverFlags << status;

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
	}

	void Connection::Disconnect ()
	{
		PingTimer_->stop ();
		Socket_->disconnectFromHost ();

		PE_.Clear ();
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
		Write (PF_.Hello ().Packet_);
	}

	void Connection::handlePing ()
	{
		Write (PF_.Ping ().Packet_);
	}

	void Connection::handleSocketError (QAbstractSocket::SocketError err)
	{
		qWarning () << Q_FUNC_INFO << err << Socket_->errorString ();
	}
}
}
}
}
