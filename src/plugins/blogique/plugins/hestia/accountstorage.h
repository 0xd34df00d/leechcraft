/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <interfaces/blogique/iaccount.h>

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	class LocalBlogAccount;

	class AccountStorage : public QObject
	{
		LocalBlogAccount *Account_;

		QSqlDatabase AccountDB_;

		QSqlQuery AddEntry_;
		QSqlQuery RemoveEntry_;
		QSqlQuery UpdateEntry_;

		QSqlQuery GetEntries_;
		QSqlQuery GetLastEntries_;
		QSqlQuery GetShortEntries_;
		QSqlQuery GetFullEntry_;
		QSqlQuery GetEntriesByDate_;
		QSqlQuery GetEntriesCountByDate_;
		QSqlQuery GetFilteredEntries_;

		QSqlQuery AddEntryTag_;
		QSqlQuery RemoveEntryTags_;
		QSqlQuery GetEntryTags_;
		QSqlQuery GetTags_;
	public:
		enum class Mode
		{
			ShortMode,
			FullMode
		};

		explicit AccountStorage (LocalBlogAccount *acc, QObject *parent = 0);

		void Init (const QString& dbPath);
		bool CheckDatabase (const QString& dbPath);

		qint64 SaveNewEntry (const Entry& e);
		qint64 UpdateEntry (const Entry& e, qint64 entryId);
		void RemoveEntry(qint64 entryId);
		QList<Entry> GetEntries (Mode mode);
		QList<Entry> GetLastEntries (Mode mode, int count);
		QList<Entry> GetEntriesByDate (const QDate& date);
		QList<Entry> GetEntriesWithFilter (const Filter& filter);
		QMap<QDate, int> GetEntriesCountByDate ();
		Entry GetFullEntry (qint64 entryId);
		QHash<QString, int> GetAllTags ();
	private:
		void CreateTables ();
		void PrepareQueries ();
	};
}
}
}
