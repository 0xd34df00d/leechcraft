/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ondiskstorage.h"
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>
#include "entrystats.h"

namespace LC
{
namespace Azoth
{
namespace LastSeen
{
	struct OnDiskStorage::Record
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> EntryID_;
		QDateTime Available_;
		QDateTime Online_;
		QDateTime StatusChange_;

		static Record FromEntryStats (const QString& entryId, const EntryStats& stats)
		{
			return
			{
				.EntryID_ = entryId,
				.Available_ = stats.Available_,
				.Online_ = stats.Online_,
				.StatusChange_ = stats.StatusChange_
			};
		}

		EntryStats ToEntryStats () const
		{
			return { .Available_ = Available_, .Online_ = Online_, .StatusChange_ = StatusChange_ };
		}

		constexpr static auto ClassName = "EntryStats"_ct;
	};
}
}
}

using StatsRecord = LC::Azoth::LastSeen::OnDiskStorage::Record;

ORAL_ADAPT_STRUCT (StatsRecord,
		EntryID_,
		Available_,
		Online_,
		StatusChange_)

namespace LC
{
namespace Azoth
{
namespace LastSeen
{
	namespace sph = Util::oral::sph;

	OnDiskStorage::OnDiskStorage (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
				Util::GenConnectionName ("org.LeechCraft.Azoth.LastSeen.EntryStats")) }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth/lastseen");
		DB_.setDatabaseName (cacheDir.filePath ("entrystats.db"));

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		AdaptedRecord_ = Util::oral::AdaptPtr<Record> (DB_);
	}

	OnDiskStorage::~OnDiskStorage () = default;

	std::optional<EntryStats> OnDiskStorage::GetEntryStats (const QString& entryId)
	{
		return AdaptedRecord_->SelectOne (sph::f<&Record::EntryID_> == entryId).transform (&Record::ToEntryStats);
	}

	void OnDiskStorage::SetEntryStats (const QString& entryId, const EntryStats& stats)
	{
		AdaptedRecord_->Insert (Record::FromEntryStats (entryId, stats), Util::oral::InsertAction::Replace::Whole);
	}

	Util::DBLock OnDiskStorage::BeginTransaction ()
	{
		Util::DBLock lock { DB_ };
		lock.Init ();
		return lock;
	}
}
}
}
