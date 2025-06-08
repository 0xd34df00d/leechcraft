/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "storage.h"
#include <util/threads/coro/taskfwd.h>
#include <util/threads/workerthreadbasefwd.h>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	class Storage;
	using StorageThread = Util::WorkerThread<Storage>;

	class LoggingStateKeeper;

	class StorageManager : public QObject
	{
		const std::shared_ptr<StorageThread> StorageThread_;
		LoggingStateKeeper * const LoggingStateKeeper_;
	public:
		StorageManager (LoggingStateKeeper*);

		void Process (QObject*);
		void AddLogItems (const QString&, const QString&, const QString&, const QList<LogItem>&, bool);

		QFuture<IHistoryPlugin::MaxTimestampResult_t> GetMaxTimestamp (const QString&);

		QFuture<QStringList> GetOurAccounts ();

		QFuture<UsersForAccountResult_t> GetUsersForAccount (const QString&);

		QFuture<ChatLogsResult_t> GetChatLogs (const QString& accountId, const QString& entryId,
				int backpages, int amount);

		QFuture<SearchResult_t> Search (const QString& accountId, const QString& entryId,
				const QString& text, int shift, bool cs);
		QFuture<SearchResult_t> Search (const QString& accountId, const QString& entryId, const QDateTime& dt);

		QFuture<DaysResult_t> GetDaysForSheet (const QString& accountId, const QString& entryId, int year, int month);
		void ClearHistory (const QString& accountId, const QString& entryId);

		void RegenUsersCache ();
	private:
		Util::ContextTask<void> CheckStorage ();

		void StartStorage ();
		void HandleStorageError (const Storage::InitializationError_t&);
	};
}
}
}
