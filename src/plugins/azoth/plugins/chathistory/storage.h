/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QSqlQuery>
#include <QHash>
#include <QVariant>
#include <QDateTime>
#include <util/sll/void.h>
#include <interfaces/azoth/ihistoryplugin.h>
#include "storagestructures.h"

class QSqlDatabase;

namespace LC
{
namespace Azoth
{
class IMessage;

namespace ChatHistory
{
	class Storage : public QObject
	{
		Q_OBJECT

		std::shared_ptr<QSqlDatabase> DB_;
		QSqlQuery MaxTimestampSelector_;
		QSqlQuery UserSelector_;
		QSqlQuery AccountSelector_;
		QSqlQuery UserIDSelector_;
		QSqlQuery AccountIDSelector_;
		QSqlQuery UserInserter_;
		QSqlQuery AccountInserter_;
		QSqlQuery MessageDumper_;
		QSqlQuery MessageDumperFuzzy_;
		QSqlQuery UsersForAccountGetter_;
		QSqlQuery RowID2Pos_;
		QSqlQuery Date2Pos_;
		QSqlQuery GetMonthDates_;
		QSqlQuery LogsSearcher_;
		QSqlQuery LogsSearcherWOContact_;
		QSqlQuery LogsSearcherWOContactAccount_;
		QSqlQuery HistoryGetter_;
		QSqlQuery HistoryClearer_;
		QSqlQuery UserClearer_;
		QSqlQuery EntryCacheSetter_;
		QSqlQuery EntryCacheGetter_;
		QSqlQuery EntryCacheClearer_;

		QHash<QString, qint32> Users_;
		QHash<QString, qint32> Accounts_;

		QHash<qint32, QString> EntryCache_;

		struct RawSearchResult
		{
			qint32 EntryID_ = 0;
			qint32 AccountID_ = 0;
			qint64 RowID_ = -1;

			RawSearchResult () = default;
			RawSearchResult (qint32 entryId, qint32 accountId, qint64 rowId);

			bool IsEmpty () const;
		};
	public:
		Storage (QObject* = nullptr);

		struct GeneralError
		{
			QString ErrorText_;
		};

		using InitializationError_t = std::variant<GeneralError>;
		using InitializationResult_t = Util::Either<InitializationError_t, Util::Void>;

		static QString GetDatabasePath ();

		QSqlDatabase GetDB () const;

		InitializationResult_t Initialize ();

		IHistoryPlugin::MaxTimestampResult_t GetMaxTimestamp (const QString&);

		QStringList GetOurAccounts () const;
		UsersForAccountResult_t GetUsersForAccount (const QString&);
		ChatLogsResult_t GetChatLogs (const QString& accountId,
				const QString& entryId, int backpages, int amount);

		void AddMessages (const QString& accountId, const QString& entryId,
				const QString& visibleName, const QList<LogItem>&, bool fuzzy);

		SearchResult_t Search (const QString& accountId, const QString& entryId,
				const QString& text, int shift, bool cs);
		SearchResult_t SearchDate (const QString& accountId,
				const QString& entryId, const QDateTime& dt);

		DaysResult_t GetDaysForSheet (const QString& accountId, const QString& entryId, int year, int month);

		boost::optional<int> GetAllHistoryCount ();

		void RegenUsersCache ();
		void ClearHistory (const QString& accountId, const QString& entryId);
	private:
		void InitializeTables ();
		void UpdateTables ();

		QHash<QString, qint32> GetUsers ();
		qint32 GetUserID (const QString&);
		void AddUser (const QString& id, const QString& accountId);

		void PrepareEntryCache ();

		QHash<QString, qint32> GetAccounts ();
		qint32 GetAccountID (const QString&);
		void AddAccount (const QString& id);
		RawSearchResult SearchImpl (const QString& accountId, const QString& entryId,
				const QString& text, int shift, bool cs);
		RawSearchResult SearchImpl (const QString& accountId, const QString& text, int shift, bool cs);
		RawSearchResult SearchImpl (const QString& text, int shift, bool cs);

		SearchResult_t SearchRowIdImpl (qint32, qint32, qint64);
		SearchResult_t SearchDateImpl (qint32, qint32, const QDateTime&);
	};
}
}
}
