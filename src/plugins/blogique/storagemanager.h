/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "interfaces/blogique/iaccount.h"

namespace LC
{
namespace Blogique
{
	class StorageManager : public QObject
	{
		Q_OBJECT
		QSqlDatabase BlogiqueDB_;

		QSqlQuery AddAccount_;
		QSqlQuery RemoveAccount_;

		QSqlQuery AddDraft_;
		QSqlQuery UpdateDraft_;
		QSqlQuery RemoveDraft_;

		QSqlQuery GetDrafts_;
		QSqlQuery GetShortDrafts_;
		QSqlQuery GetFullDraft_;
		QSqlQuery GetDraftsByDate_;
		QSqlQuery GetDraftsCountByDate_;

		QSqlQuery AddDraftTag_;
		QSqlQuery RemoveDraftTags_;
		QSqlQuery GetDraftTags_;

	public:
		enum class Mode
		{
			ShortMode,
			FullMode
		};

		explicit StorageManager (const QByteArray& id, QObject *parent = 0);

		void AddAccount (const QByteArray& accounId);
		void RemoveAccount (const QByteArray& accounId);

		qint64 SaveNewDraft (const Entry& e);
		qint64 UpdateDraft (const Entry& e, qint64 draftId);
		void RemoveDraft (qint64 draftId);
		QList<Entry> GetDrafts (Mode mode);
		QList<Entry> GetDraftsByDate (const QDate& date);
		QMap<QDate, int> GetDraftsCountByDate ();
		Entry GetFullDraft (qint64 draftId);
	private:
		void CreateTables ();
		void PrepareQueries ();
	};
}
}

