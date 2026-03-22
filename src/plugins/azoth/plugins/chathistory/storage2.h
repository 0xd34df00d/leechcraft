/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>
#include <util/threads/coro/workerthread.h>
#include <interfaces/azoth/ihistoryplugin.h>

namespace LC::Azoth::ChatHistory
{
	struct AccountInfo
	{
		qint64 Id_;
		QByteArray AccountId_;
		QString AccountName_;
	};

	struct Entry
	{
		qint64 Id_;

		QString HumanReadableId_;
		History::SomeEntryDescr EntryInfo_;
	};

	class Storage2 final : public QObject
	{
	public:
		struct AccountRecord;
		struct EntryRecord;
		struct MucContextRecord;
		struct MessageRecord;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<AccountRecord> Accounts_;
		Util::oral::ObjectInfo_ptr<EntryRecord> Entries_;
		Util::oral::ObjectInfo_ptr<MucContextRecord> MucContexts_;
		Util::oral::ObjectInfo_ptr<MessageRecord> Messages_;
	public:
		explicit Storage2 (QObject* = nullptr);
		~Storage2 () override;

		std::optional<QDateTime> GetLastTimestamp (const QByteArray& accountId) const;

		QList<AccountInfo> GetAccounts () const;
		QList<Entry> GetEntries (qint64 accountId) const;

		struct HistoryMessage
		{
			qint64 Id_;

			QString DisplayName_;
			std::optional<QString> Variant_;
			IMessage::Direction Direction_;

			QDateTime TS_;
			QString Body_;
			std::optional<QString> RichBody_;
		};

		struct Pagination
		{
			qint64 CursorMessageId_ = std::numeric_limits<qint64>::max ();
			uint16_t Before_ = 0;
			uint16_t After_ = 0;
			bool IncludeCursor_ = Before_ && After_;
		};
		QList<HistoryMessage> GetMessages (const Entry& entry, const Pagination& pagination) const;
		QList<HistoryMessage> GetMessagesDated (const Entry& entry, const QDate& date) const;
		QList<HistoryMessage> GetLastMessages (const QByteArray& accountId, const QString& entryHumanReadableId, size_t count) const;

		QList<int> GetDaysWithHistory (const Entry& entry, int year, int month) const;

		enum class SearchDirection : std::uint8_t
		{
			Backward,
			Forward,
		};
		std::optional<qint64> Search (const Entry& entry,
				const QString& text, Qt::CaseSensitivity cs,
				SearchDirection dir, qint64 from) const;

		void AddMessage (const History::SomeEntryWithMessages& messages);

		void ClearHistory (const Entry& entry);
	};

	class StorageThread : public Util::Coro::WorkerThread<Storage2, StorageThread>
	{
	public:
		using WorkerThread::WorkerThread;
	};
}
