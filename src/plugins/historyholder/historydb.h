/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>
#include <interfaces/core/iloadprogressreporter.h>

class QDateTime;
class QAbstractItemModel;

class ITagsManager;

namespace LC
{
struct Entity;

namespace HistoryHolder
{
	class HistoryDB : public QObject
	{
		ITagsManager * const TM_;

		QSqlDatabase DB_ = QSqlDatabase::addDatabase ("QSQLITE", "org.LeechCraft.HistoryHolder");

		QSqlQuery InsertHistory_;
		QSqlQuery InsertTags_;
		QSqlQuery InsertTagsMapping_;
		QSqlQuery InsertEntity_;

		QSqlQuery SelectHistory_;

		QMap<QString, int> Tags_;
	public:
		HistoryDB (ITagsManager*, const ILoadProgressReporter_ptr&, QObject* = nullptr);

		std::shared_ptr<QAbstractItemModel> CreateModel () const;

		void Add (const Entity&);
	private:
		void InitTables ();
		void InitQueries ();
		void LoadTags ();

		void Add (const Entity&, const QDateTime&);
		QList<int> AddTags (const QStringList&);
		void AssociateTags (int, const QList<int>&);

		void Migrate (const ILoadProgressReporter_ptr&);
	};
}
}
