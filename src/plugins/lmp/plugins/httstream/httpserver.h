/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QReadWriteLock>

class QTcpServer;
class QTcpSocket;

namespace LC
{
namespace LMP
{
namespace HttStream
{
	class HttpServer : public QObject
	{
		Q_OBJECT

		QTcpServer * const Server_;

		mutable QReadWriteLock MapLock_;
		QMap<QTcpSocket*, int> Socket2FD_;
	public:
		HttpServer (QObject* = nullptr);

		void SetAddress (const QString&, int);
	private:
		void HandleSocket (QTcpSocket*);
		void HandleNewConnection ();
		void HandleDisconnected (QTcpSocket*);
	signals:
		void gotClient (int);
		void clientDisconnected (int);
	};
}
}
}
