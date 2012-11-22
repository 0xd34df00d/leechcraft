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
		QSqlQuery AddDraftPostOptions_;
		QSqlQuery UpdateDraftPostOptions_;
		QSqlQuery AddDraftCustomOptions_;
		QSqlQuery UpdateDraftCustomOptions_;
		QSqlQuery GetDrafts_;
		QSqlQuery GetShortDrafts_;
		QSqlQuery GetFullDraft_;
		QSqlQuery GetDraftPostOptions_;
		QSqlQuery GetDraftCustomOptions_;
		QSqlQuery RemoveDraft_;
		QSqlQuery AddEntry_;
		QSqlQuery AddEntryParam_;
	public:
		LocalStorage (QObject *obj = 0);

		void AddAccount (const QByteArray& accounId);
		qlonglong SaveDraft (const QByteArray& accountID, const Event& e);
		void UpdateDraft (qlonglong id, const Event& e);
		QList<Event> GetDrafts (const QByteArray& accountId);
		Event GetFullDraft (const QByteArray& accountId, qlonglong draftId);
		QList<Event> GetShortDrafts (const QByteArray& accountId);
		void RemoveDraft (qlonglong id);
	private:
		void CreateTables ();
		void PrepareQueries ();
	};
}
}

