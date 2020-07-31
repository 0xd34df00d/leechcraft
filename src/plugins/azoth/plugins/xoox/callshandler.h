/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QXmppCallManager;

namespace LC::Azoth::Xoox
{
	class GlooxAccount;
	class ClientConnection;

	class CallsHandler : public QObject
	{
		QXmppCallManager& Mgr_;
		ClientConnection& Conn_;
		GlooxAccount& Acc_;
	public:
		explicit CallsHandler (QXmppCallManager&, ClientConnection&, GlooxAccount&, QObject* = nullptr);

		QObject* Call (const QString& id, const QString& variant);
	};
}
