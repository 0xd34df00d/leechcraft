/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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
	enum class Mode
	{
		ShortMode,
		FullMode
	};

	class StorageManager : public QObject
	{
		Q_OBJECT
		QSqlDatabase BlogiqueDB_;

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

	public:
		explicit StorageManager (QObject *parent = 0);

		void AddAccount (const QByteArray& accounId);

		qint64 SaveNewDraft (const Entry& e);
		void RemoveDraft (qint64 draftId);
		QList<Entry> GetDrafts (Mode mode);
		QMap<QDate, int> GetDraftsCountByDate ();
		Entry GetFullDraft (qint64 draftId);
	private:
		void CreateTables ();
		void PrepareQueries ();

	public slots:
		void updateDraft (const Entry& e, qint64 draftId);
	};
}
}

