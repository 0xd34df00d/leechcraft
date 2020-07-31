/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QSet>
#include <QXmppStanza.h>
#include <QXmppClient.h>

class QXmppIq;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;

	class ClientConnectionErrorMgr : public QObject
	{
		Q_OBJECT

		ClientConnection *ClientConn_;
		QXmppClient *Client_;

		QSet<QString> WhitelistedErrors_;

		int SocketErrorAccumulator_;

		bool IsDisconnecting_ = false;
	public:
		using ErrorHandler_f = std::function<bool (QXmppIq)>;
	private:
		QHash<QString, ErrorHandler_f> ErrorHandlers_;
	public:
		ClientConnectionErrorMgr (ClientConnection*);

		void Whitelist (const QString&, bool add = true);
		void SetErrorHandler (const QString&, const ErrorHandler_f&);

		void HandleIq (const QXmppIq&);
		void HandleMessage (const QXmppMessage&);

		static QString HandleErrorCondition (QXmppStanza::Error::Condition);
	private:
		void HandleError (const QXmppIq&);
	private slots:
		void handleError (QXmppClient::Error);
		void decrementErrAccumulators ();
	signals:
		void serverAuthFailed ();
	};
}
}
}
