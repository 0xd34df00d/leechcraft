/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ircserversocket.h"
#include <QTcpSocket>
#include <QTextCodec>
#include <QTimer>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include "ircserverhandler.h"
#include "clientconnection.h"

namespace LC::Azoth::Acetamide
{
	constexpr int MaxRetriesCount = 10;

	IrcServerSocket::IrcServerSocket (IrcServerHandler *ish)
	: QObject { ish }
	, ISH_ { ish }
	, RetryTimer_ { new QTimer { this } }
	{
		RetryTimer_->setSingleShot (true);
		RetryTimer_->callOnTimeout ([this]
				{
					qDebug () << "LC::Azoth::Acetamide::IrcServerSocket: retrying...";
					ConnectToHost (Host_, Port_);
				});

		if (ish->GetServerOptions ().SSL_)
		{
			auto socket = std::make_shared<QSslSocket> ();
			Socket_ = socket;
			connect (socket.get (),
					qOverload<const QList<QSslError>&> (&QSslSocket::sslErrors),
					this,
					&IrcServerSocket::handleSslErrors);
		}
		else
			Socket_ = std::make_shared<QTcpSocket> ();

		const auto socket = GetSocketPtr ();

		connect (socket,
				&QTcpSocket::readyRead,
				this,
				[this, socket]
				{
					RetriesCount_ = 0;
					RetryTimer_->stop ();

					while (socket->canReadLine ())
						ISH_->ReadReply (GetCodec ()->toUnicode (socket->readLine ()));
				});

		connect (socket,
				&QTcpSocket::connected,
				this,
				[this, socket]
				{
					qDebug () << "LC::Azoth::Acetamide::IrcServerSocket: connected";
					emit connected ();
					connect (socket,
							&QTcpSocket::disconnected,
							this,
							&IrcServerSocket::disconnected,
							Qt::UniqueConnection);
				});

		connect (socket,
				&QTcpSocket::errorOccurred,
				this,
				[this, socket] (QAbstractSocket::SocketError error)
				{
					disconnect (socket,
							&QTcpSocket::disconnected,
							this,
							&IrcServerSocket::disconnected);

					qWarning () << "LC::Azoth::Acetamide::IrcServerSocket:"
							<< error
							<< "after" << RetriesCount_ << "retries;"
							<< socket->errorString ();
					if (++RetriesCount_ > MaxRetriesCount)
					{
						emit finalSocketError (error, socket->errorString ());
						return;
					}
					emit retriableSocketError (error, socket->errorString ());
					RetryTimer_->start ((2 * RetriesCount_ + 1) * 1000);
				});
	}

	IrcServerSocket::~IrcServerSocket ()
	{
		if (const auto socket = GetSocketPtr ())
		{
			QObject::disconnect (socket, nullptr, nullptr, nullptr);
			socket->abort ();
		}
	}

	void IrcServerSocket::ConnectToHost (const QString& host, int port)
	{
		Host_ = host;
		Port_ = port;

		Util::Visit (Socket_,
				[&] (const Tcp_ptr& ptr) { ptr->connectToHost (host, port); },
				[&] (const Ssl_ptr& ptr) { ptr->connectToHostEncrypted (host, port); });
	}

	void IrcServerSocket::DisconnectFromHost ()
	{
		GetSocketPtr ()->disconnectFromHost ();
	}

	void IrcServerSocket::Send (const QString& message)
	{
		const auto socket = GetSocketPtr ();
		if (!socket->isWritable ())
		{
			qWarning () << Q_FUNC_INFO
					<< socket->error ()
					<< socket->errorString ();
			return;
		}

		if (socket->write (GetCodec ()->fromUnicode (message)) == -1)
			qWarning () << Q_FUNC_INFO
					<< socket->error ()
					<< socket->errorString ();
	}

	void IrcServerSocket::Close ()
	{
		GetSocketPtr ()->close ();
	}

	QTextCodec* IrcServerSocket::GetCodec ()
	{
		const auto encoding = ISH_->GetServerOptions ().ServerEncoding_;
		if (LastCodec_ && LastCodec_->name () == encoding)
			return LastCodec_;

		const auto newCodec = encoding == "System"_ql ?
				QTextCodec::codecForLocale () :
				QTextCodec::codecForName (encoding.toLatin1 ());
		if (newCodec)
		{
			LastCodec_ = newCodec;
			return LastCodec_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unable to create codec for encoding `"
				<< encoding.toUtf8 ()
				<< "`; known codecs:"
				<< QTextCodec::availableCodecs ();

		const auto& notify = Util::MakeNotification ("Azoth Acetamide",
				tr ("Unknown encoding %1.")
					.arg ("<em>" + encoding + "</em>"),
				Priority::Critical);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (notify);

		if (LastCodec_)
			return LastCodec_;

		qWarning () << Q_FUNC_INFO
				<< "no codec is set, will fall back to locale-default codec";

		LastCodec_ = QTextCodec::codecForLocale ();
		return LastCodec_;
	}

	QTcpSocket* IrcServerSocket::GetSocketPtr () const
	{
		return Util::Visit (Socket_,
				[] (const auto& ptr) -> QTcpSocket* { return ptr.get (); });
	}

	namespace
	{
		class SslErrorsReaction final : public ICanHaveSslErrors::ISslErrorsReaction
		{
			std::weak_ptr<QSslSocket> Sock_;
		public:
			explicit SslErrorsReaction (const std::shared_ptr<QSslSocket>& sock)
			: Sock_ { sock }
			{
			}

			void Ignore () override
			{
				if (const auto sock = Sock_.lock ())
					sock->ignoreSslErrors ();
			}

			void Abort () override
			{
			}
		};
	}

	void IrcServerSocket::handleSslErrors (const QList<QSslError>& errors)
	{
		Util::Visit (Socket_,
				[&] (const Ssl_ptr& s) { emit sslErrors (errors, std::make_shared<SslErrorsReaction> (s)); },
				[] (auto) { qWarning () << Q_FUNC_INFO << "expected SSL socket"; });
	}
}
