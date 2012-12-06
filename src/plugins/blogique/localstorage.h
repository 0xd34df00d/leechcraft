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
#include "interfaces/blogique/iaccount.h"

namespace LeechCraft
{
namespace Blogique
{
	class LocalStorage :  public QObject
	{
		Q_OBJECT

		QSqlDatabase DB_;

		QSqlQuery AddAccount_;

		QSqlQuery AddDraft_;
		QSqlQuery UpdateDraft_;
		QSqlQuery RemoveDraft_;
		QSqlQuery GetDrafts_;
		QSqlQuery GetShortDrafts_;
		QSqlQuery GetFullDraft_;

		QSqlQuery AddDraftPostOptions_;
		QSqlQuery UpdateDraftPostOptions_;
		QSqlQuery GetDraftPostOptions_;

		QSqlQuery AddDraftCustomOptions_;
		QSqlQuery UpdateDraftCustomOptions_;
		QSqlQuery GetDraftCustomOptions_;

		QSqlQuery AddDraftTag_;
		QSqlQuery RemoveDraftTags_;
		QSqlQuery GetDraftTags_;

		QSqlQuery AddEntry_;
		QSqlQuery UpdateEntry_;
		QSqlQuery RemoveEntry_;
		QSqlQuery GetEntry_;
		QSqlQuery GetAllEntries_;
		QSqlQuery GetLastEntries_;
		QSqlQuery GetEntriesByDate_;
		QSqlQuery GetEntriesCountByDate_;

		QSqlQuery AddEntryPostOptions_;
		QSqlQuery UpdateEntryPostOptions_;
		QSqlQuery GetEntryPostOptions_;

		QSqlQuery AddEntryCustomOptions_;
		QSqlQuery UpdateEntryCustomOptions_;
		QSqlQuery GetEntryCustomOptions_;

		QSqlQuery AddEntryTag_;
		QSqlQuery RemoveEntryTags_;
		QSqlQuery GetEntryTags_;

	public:
		LocalStorage (QObject *obj = 0);

		void AddAccount (const QByteArray& accounId);

		qlonglong SaveDraft (const QByteArray& accountID, const Entry& e);
		void UpdateDraft (qlonglong id, const Entry& e);
		void RemoveDraft (const QByteArray& accountId, qlonglong id);
		QList<Entry> GetDrafts (const QByteArray& accountId);
		QList<Entry> GetShortDrafts (const QByteArray& accountId);
		Entry GetFullDraft (const QByteArray& accountId, qlonglong draftId);

		void SaveEntries (const QByteArray& accountID, const QList<Entry>& events);
		void UpdateEntry (const Entry& e);
		void RemoveEntry (const QByteArray& accountId, qlonglong id);
		Entry GetEntry (const QByteArray& accountId, qlonglong entryId);
		QList<Entry> GetAllEntries (const QByteArray& accountId);
		QList<Entry> GetLastEntries (const QByteArray& accountId, int count);
		QList<Entry> GetEntriesByDate (const QByteArray& accountId,
				const QDate& date);
		QMap<QDate, int> GetEntriesCountByDate (const QByteArray& accountId);

		void MoveFromEntriesToDrafts (const QByteArray& id, int itemId);
	private:
		void CreateTables ();
		void PrepareQueries ();
	};

}
}

