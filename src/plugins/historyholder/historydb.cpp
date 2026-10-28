/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historydb.h"
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QUrl>
#include <QSqlQueryModel>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/sll/prelude.h>
#include <interfaces/structures.h>
#include <interfaces/core/itagsmanager.h>

namespace LC
{
namespace HistoryHolder
{
	HistoryDB::HistoryDB (ITagsManager *tm, QObject *parent)
	: QObject { parent }
	, TM_ { tm }
	{
		DB_.setDatabaseName (Util::CreateIfNotExists ("historyholder").filePath ("history.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA foreign_keys = ON;");
		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		InitTables ();
		InitQueries ();

		LoadTags ();
	}

	namespace
	{
		class HistoryModel final : public QSqlQueryModel
		{
			const ITagsManager& TM_;
		public:
			enum Column
			{
				Title,
				Timestamp,
				Tags,
			};

			HistoryModel (const QSqlDatabase& db, const ITagsManager& tm)
			: TM_ { tm }
			{
				setQuery (QSqlQuery { Util::LoadQuery ("historyholder", "select_history"), db });
				if (lastError ().isValid ())
					Util::DBLock::DumpError (lastError ());
			}

			QVariant data (const QModelIndex& index, int role) const override
			{
				if (role == +CustomDataRoles::Tags)
					return QVariant::fromValue (GetTagIds (index.row ()));

				if (role == Qt::DisplayRole && index.column () == Tags)
					return TM_.JoinIDs (GetTagIds (index.row ()));

				return QSqlQueryModel::data (index, role);
			}

			QVariant headerData (int section, Qt::Orientation orientation, int role) const override
			{
				if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
					return QSqlQueryModel::headerData (section, orientation, role);

				switch (section)
				{
				case Title:
					return HistoryDB::tr ("Title");
				case Timestamp:
					return HistoryDB::tr ("Date");
				case Tags:
					return HistoryDB::tr ("Tags");
				}
				return {};
			}
		private:
			QList<TagId> GetTagIds (int row) const
			{
				// the separator is the one select_history.sql passes to GROUP_CONCAT()
				const auto& joined = QSqlQueryModel::data (index (row, Tags)).toString ();
				return Util::Map (joined.split (';', Qt::SkipEmptyParts), [] (const QString& id) { return TagId { id }; });
			}
		};
	}

	std::shared_ptr<QAbstractItemModel> HistoryDB::CreateModel () const
	{
		return std::make_shared<HistoryModel> (DB_, *TM_);
	}

	void HistoryDB::Add (const Entity& entity)
	{
		if (entity.Parameters_ & LC::DoNotSaveInHistory ||
				entity.Parameters_ & LC::Internal ||
				!(entity.Parameters_ & LC::IsDownloaded))
			return;

		Add (entity, QDateTime::currentDateTime ());
	}

	void HistoryDB::InitTables ()
	{
		if (DB_.tables ().contains ("History"))
			return;

		try
		{
			for (const auto& table : QStringList { "history", "tags", "tags_mapping" })
				Util::RunQuery (DB_, "historyholder", "create_" + table);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to initialize queries:"
					<< e.what ();
			throw;
		}
	}

	void HistoryDB::InitQueries ()
	{
		auto loadQuery = std::bind (&Util::LoadQuery, "historyholder", std::placeholders::_1);

		InsertHistory_ = QSqlQuery { DB_ };
		InsertHistory_.prepare (loadQuery ("insert_history"));

		InsertTags_ = QSqlQuery { DB_ };
		InsertTags_.prepare (loadQuery ("insert_tags"));

		InsertTagsMapping_ = QSqlQuery { DB_ };
		InsertTagsMapping_.prepare (loadQuery ("insert_tags_mapping"));
	}

	void HistoryDB::LoadTags ()
	{
		QSqlQuery query { DB_ };
		query.prepare (Util::LoadQuery ("historyholder", "select_tags"));
		Util::DBLock::Execute (query);

		while (query.next ())
		{
			const auto id = query.value (0).toInt ();
			const auto& lcId = query.value (1).toString ();
			Tags_ [lcId] = id;
		}
	}

	namespace
	{
		QString GetTitle (const Entity& e)
		{
			QString stren;
			if (e.Additional_.contains ("UserVisibleName") &&
					e.Additional_ ["UserVisibleName"].canConvert<QString> ())
				stren = e.Additional_ ["UserVisibleName"].toString ();
			else if (e.Entity_.canConvert<QUrl> ())
				stren = e.Entity_.toUrl ().toString ();
			else if (e.Entity_.canConvert<QByteArray> ())
			{
				const auto& entity = e.Entity_.toByteArray ();
				if (entity.size () < 250)
					stren = QString::fromUtf8 (entity);
			}
			else
				stren = HistoryDB::tr ("Binary data");

			if (!e.Location_.isEmpty ())
			{
				stren += " (";
				stren += e.Location_;
				stren += ")";
			}

			return stren;
		}
	}

	void HistoryDB::Add (const Entity& entity, const QDateTime& ts)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();

		InsertHistory_.bindValue (":title", GetTitle (entity));
		InsertHistory_.bindValue (":ts", ts);
		Util::DBLock::Execute (InsertHistory_);

		const auto& historyId = Util::GetLastId (InsertHistory_);

		AssociateTags (historyId, AddTags (entity.Additional_ [" Tags"].value<QList<TagId>> ()));

		lock.Good ();
	}

	QList<int> HistoryDB::AddTags (const QList<TagId>& tags)
	{
		QList<int> result;

		for (const auto& tag : tags)
		{
			if (Tags_.contains (tag.Id_))
			{
				result << Tags_.value (tag.Id_);
				continue;
			}

			const auto& text = TM_->GetTag (tag);
			if (!text)
			{
				qWarning () << "unknown tag" << tag.Id_;
				continue;
			}

			InsertTags_.bindValue (":lcid", tag.Id_);
			InsertTags_.bindValue (":text", *text);
			Util::DBLock::Execute (InsertTags_);

			const auto id = Util::GetLastId (InsertTags_);
			Tags_ [tag.Id_] = id;
			result << id;
		}

		return result;
	}

	void HistoryDB::AssociateTags (int historyId, const QList<int>& tags)
	{
		for (const auto tag : tags)
		{
			InsertTagsMapping_.bindValue (":tagId", tag);
			InsertTagsMapping_.bindValue (":entryId", historyId);
			Util::DBLock::Execute (InsertTagsMapping_);
		}
	}
}
}
