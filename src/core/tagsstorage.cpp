/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsstorage.h"
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/sll/qtutil.h>

namespace LC
{
	struct TagsStorage::Record
	{
		Util::oral::NotNull<QByteArray> Id_;
		Util::oral::NotNull<QString> Name_;

		constexpr static auto ClassName = "TagNames"_ct;

		using Constraints = Util::oral::Constraints<
				Util::oral::UniqueSubset<&Record::Id_, &Record::Name_>
			>;
	};
}

ORAL_ADAPT_STRUCT (LC::TagsStorage::Record,
		Id_,
		Name_)

namespace LC
{
	namespace sph = Util::oral::sph;

	TagsStorage::TagsStorage (const QString& dbPath)
	: DB_ { QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LeechCraft.Core.TagsStorage")) }
	{
		DB_.setDatabaseName (dbPath);

		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		Record_ = Util::oral::AdaptPtr<Record> (DB_);
		MigrateLegacyTags ();
	}

	TagsStorage::~TagsStorage () = default;

	void TagsStorage::AddTag (const Id& id, const QString& name)
	{
		Record_->Insert ({ id.toByteArray (), name }, Util::oral::InsertAction::Ignore);
	}

	void TagsStorage::ReplaceName (const QString& old, const QStringList& newNames)
	{
		Util::DBLock lock { DB_ };
		lock.Init ();

		for (const auto& rec : Record_->Select (sph::f<&Record::Name_> == old))
			for (const auto& name : newNames)
				Record_->Insert ({ rec.Id_, name }, Util::oral::InsertAction::Ignore);

		if (!newNames.contains (old))
			Record_->DeleteBy (sph::f<&Record::Name_> == old);

		lock.Good ();
	}

	QList<std::pair<TagsStorage::Id, QString>> TagsStorage::GetAllTags () const
	{
		QList<std::pair<Id, QString>> result;
		for (const auto& rec : Record_->Select ())
		{
			const QUuid id { QString::fromLatin1 (rec.Id_) };
			if (id.isNull ())
			{
				qWarning () << "skipping tag with an unparseable id" << *rec.Id_ << *rec.Name_;
				continue;
			}
			result.emplaceBack (id, *rec.Name_);
		}
		return result;
	}

	// TODO drop this Nov 2027
	void TagsStorage::MigrateLegacyTags ()
	{
		if (!DB_.tables ().contains ("Tags"_qs))
			return;

		Util::DBLock lock { DB_ };
		lock.Init ();
		Util::RunTextQuery (DB_, "INSERT OR IGNORE INTO TagNames (Id, Name) SELECT Id, Name FROM Tags;"_qs);
		Util::RunTextQuery (DB_, "DROP TABLE Tags;"_qs);
		lock.Good ();
	}
}
