/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <util/threads/coro/taskfwd.h>
#include "interfaces/azoth/ihaveserverhistory.h"

class QDateTime;

namespace LC
{
namespace Azoth
{
	class IAccount;
	class IHistoryPlugin;

	class HistorySyncer : public QObject
	{
		Q_OBJECT

		QList<IHistoryPlugin*> Storages_;

		QSet<IAccount*> CurrentlyOnline_;
	public:
		HistorySyncer (QObject* = nullptr);

		void AddStorage (IHistoryPlugin*);
		void AddAccount (IAccount*);
	private:
		Util::ContextTask<void> SyncAccount (IAccount*);
		Util::ContextTask<void> RequestAccountFrom (IAccount*, const std::optional<QDateTime>&);

		void AppendItems (const QList<History::SomeEntryWithMessages>&);
	};
}
}
