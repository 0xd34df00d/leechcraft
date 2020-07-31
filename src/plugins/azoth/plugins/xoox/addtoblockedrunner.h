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
#include <QStringList>
#include "xeps/privacylistsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	using ClientConnection_ptr = std::shared_ptr<ClientConnection>;

	class AddToBlockedRunner : public QObject
	{
		Q_OBJECT

		const QStringList Ids_;
		const ClientConnection_ptr Conn_;
	public:
		AddToBlockedRunner (const QStringList& ids, const ClientConnection_ptr&, QObject* = nullptr);
	private:
		void HandleGotLists (const QStringList&, const QString&, const QString&);
		void FetchList (const QString&, bool);
		void AddToList (const QString&, PrivacyList, bool);
	};
}
}
}
