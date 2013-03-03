/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <interfaces/blogique/iaccount.h>

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	class LocalBlogAccount;

	class AccountStorage : public QObject
	{
		Q_OBJECT

		LocalBlogAccount *Account_;
		bool Ready_;

		QSqlDatabase AccountDB_;

		QSqlQuery AddEntry_;
		QSqlQuery RemoveEntry_;
		QSqlQuery UpdateEntry_;

		QSqlQuery GetEntries_;
		QSqlQuery GetShortEntries_;
		QSqlQuery GetFullEntry_;
		QSqlQuery GetEntriesByDate_;
		QSqlQuery GetEntriesCountByDate_;

		QSqlQuery AddEntryTag_;
		QSqlQuery RemoveEntryTags_;
		QSqlQuery GetEntryTags_;

	public:
		enum class Mode
		{
			ShortMode,
			FullMode
		};

		explicit AccountStorage (LocalBlogAccount *acc, QObject *parent = 0);

		void Init (const QString& dbPath);
		bool IsReady () const;
		bool CheckDatabase (const QString& dbPath);

		qint64 SaveNewEntry (const Entry& e);
		qint64 UpdateEntry (const Entry& e, qint64 entryId);
		void RemoveEntry(qint64 entryId);
		QList<Entry> GetEntries (Mode mode);
		QList<Entry> GetEntriesByDate (const QDate& date);
		QMap<QDate, int> GetEntriesCountByDate ();
		Entry GetFullEntry (qint64 entryId);
	private:
		void CreateTables ();
		void PrepareQueries ();
	};
}
}
}
