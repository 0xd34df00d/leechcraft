/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <variant>
#include <QObject>
#include <QAbstractSocket>
#include <interfaces/azoth/icanhavesslerrors.h>

class QTcpSocket;
class QSslSocket;
class QTimer;

namespace LC::Azoth::Acetamide
{
	class IrcServerHandler;
	class IrcAccount;

	class IrcServerSocket : public QObject
	{
		Q_OBJECT

		IrcServerHandler * const ISH_;

		using Tcp_ptr = std::shared_ptr<QTcpSocket>;
		using Ssl_ptr = std::shared_ptr<QSslSocket>;
		std::variant<Tcp_ptr, Ssl_ptr> Socket_;

		QTextCodec *LastCodec_ = nullptr;

		QString Host_;
		int Port_;
		int RetriesCount_ = 0;
		QTimer * const RetryTimer_;
	public:
		explicit IrcServerSocket (IrcServerHandler*);
		~IrcServerSocket () override;
		
		void ConnectToHost (const QString&, int);
		void DisconnectFromHost ();
		void Send (const QString&);
		void Close ();
	private:
		QTextCodec* GetCodec ();

		QTcpSocket* GetSocketPtr () const;
	private slots:
		void handleSslErrors (const QList<QSslError>& errors);
	signals:
		void connected ();
		void disconnected ();

		void retriableSocketError (QAbstractSocket::SocketError, const QString&);
		void finalSocketError (QAbstractSocket::SocketError, const QString&);
		void sslErrors (const QList<QSslError>&, const ICanHaveSslErrors::ISslErrorsReaction_ptr&);
	};
}
