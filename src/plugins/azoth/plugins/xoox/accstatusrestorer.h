/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include "glooxaccount.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	typedef std::weak_ptr<ClientConnection> ClientConnection_wptr;

	class AccStatusRestorer : public QObject
	{
		Q_OBJECT

		const GlooxAccountState State_;
		ClientConnection_wptr Client_;
	public:
		AccStatusRestorer (const GlooxAccountState&, ClientConnection_wptr);
	private slots:
		void handleDisconnected ();
	};
}
}
}
