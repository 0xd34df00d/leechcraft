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
#include <QDataStream>
#include <QDir>
#include <QSettings>
#include <QTextCodec>
#include <QCoreApplication>
#include <QUrl>
#include <QSqlQueryModel>
#include <QElapsedTimer>
#include <util/structuresops.h>
#include <util/sys/paths.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <interfaces/core/itagsmanager.h>
#include "historyentry.h"

namespace LC
{
namespace HistoryHolder
{
	HistoryDB::HistoryDB (ITagsManager *tm, const ILoadProgressReporter_ptr& reporter, QObject *parent)
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

		Migrate (reporter);
	}

	std::shared_ptr<QAbstractItemModel> HistoryDB::CreateModel () const
	{
		auto model = std::make_shared<QSqlQueryModel> ();

		model->setQuery (SelectHistory_);

		/* The following roles should also be handled by the model:
		 *
		 * RoleTags
		 * RoleControls
		 * RoleHash
		 * RoleMime
		 */

		return model;
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
			for (const auto& table : QStringList { "history", "entities", "tags", "tags_mapping" })
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

		InsertEntity_ = QSqlQuery { DB_ };
		InsertEntity_.prepare (loadQuery ("insert_entity"));

		SelectHistory_ = QSqlQuery { DB_ };
		SelectHistory_.prepare (loadQuery ("select_history"));
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
					stren = QTextCodec::codecForName ("UTF-8")->toUnicode (entity);
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

		QByteArray SerializeEntity (const Entity& e)
		{
			QByteArray result;

			QDataStream ostr { &result, QIODevice::ReadWrite };
			ostr << e;

			return result;
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

		auto tags = entity.Additional_ [" Tags"].toStringList ();
		if (tags.isEmpty ())
			tags.push_back ({});
		AssociateTags (historyId, AddTags (tags));

		InsertEntity_.bindValue (":entryId", historyId);
		InsertEntity_.bindValue (":entity", SerializeEntity (entity));

		lock.Good ();
	}

	QList<int> HistoryDB::AddTags (const QStringList& tags)
	{
		QList<int> result;

		for (const auto& tag : tags)
		{
			if (Tags_.contains (tag))
			{
				result << Tags_.value (tag);
				continue;
			}

			InsertTags_.bindValue (":lcid", tag);
			InsertTags_.bindValue (":text", TM_->GetTag (tag));
			Util::DBLock::Execute (InsertTags_);

			const auto id = Util::GetLastId (InsertTags_);
			Tags_ [tag] = id;
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

	void HistoryDB::Migrate (const ILoadProgressReporter_ptr& reporter)
	{
		QSettings settings
		{
			QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_HistoryHolder"
		};
		int size = settings.beginReadArray ("History");
		if (!size)
		{
			settings.endArray ();
			return;
		}

		const auto& process = reporter->InitiateProcess (tr ("Migrating downloads history..."), 0, size);

		QElapsedTimer timer;
		timer.start ();

		{
			Util::DBLock lock { DB_ };
			lock.Init ();

			for (int i = 0; i < size; ++i)
			{
				settings.setArrayIndex (size - i - 1);

				const auto& var = settings.value ("Item");
				if (var.isValid ())
				{
					const auto& entity = var.value<HistoryEntry> ();
					Add (entity.Entity_, entity.DateTime_);
				}

				process->ReportValue (i);
			}

			lock.Good ();
		}

		settings.endArray ();

		qDebug () << Q_FUNC_INFO
				<< "done in"
				<< timer.elapsed ()
				<< "ms for"
				<< size
				<< "entries";

		settings.remove ("History");

		qDebug () << Q_FUNC_INFO
				<< "removed history from QSettings";
	}
}
}
