/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsstorage.h"
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/sll/prelude.h>
#include <util/sys/paths.h>

namespace LC
{
	struct TagsStorage::Record
	{
		Util::oral::Unique<Util::oral::NotNull<QByteArray>> Id_;
		Util::oral::NotNull<QString> Name_;

		constexpr static auto ClassName ()
		{
			return "Tags"_ct;
		}
	};
}

BOOST_FUSION_ADAPT_STRUCT (LC::TagsStorage::Record,
		Id_,
		Name_)

namespace LC
{
	namespace sph = Util::oral::sph;

	TagsStorage::TagsStorage (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LeechCraft.Core.TagsStorage")) }
	{
		const auto& coreDir = Util::GetUserDir (Util::UserDir::LC, "core");
		DB_.setDatabaseName (coreDir.filePath ("core.db"));

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
	}

	void TagsStorage::AddTag (const Id& id, const QString& name)
	{
		Record_->Insert ({ id.toByteArray (), name });
	}

	void TagsStorage::DeleteTag (const Id& id)
	{
		Record_->DeleteBy (sph::f<&Record::Id_> == id.toByteArray ());
	}

	void TagsStorage::SetTagName (const Id& id, const QString& newName)
	{
		Record_->Update (sph::f<&Record::Name_> = newName,
				sph::f<&Record::Id_> == id.toByteArray ());
	}

	QList<QPair<TagsStorage::Id, QString>> TagsStorage::GetAllTags () const
	{
		return Util::Map (Record_->Select (),
				[] (const Record& rec) { return QPair { QUuid { QString::fromLatin1 (**rec.Id_) }, *rec.Name_ }; });
	}
}
