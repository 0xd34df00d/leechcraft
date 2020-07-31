/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QAbstractSocket>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	class Balancer : public QObject
	{
		Q_OBJECT
	public:
		Balancer (QObject* = 0);

		void GetServer ();
	private slots:
		void handleRead ();
		void handleSocketError (QAbstractSocket::SocketError);
	signals:
		void gotServer (const QString&, int);
		void error ();
	};
}
}
}
}
