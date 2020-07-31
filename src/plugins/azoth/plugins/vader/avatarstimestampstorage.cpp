/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarstimestampstorage.h"
#include <QDir>
#include <QSqlError>
#include <util/sys/paths.h>
#include <util/db/oral/oral.h>
#include <util/db/util.h>

namespace LC
{
namespace Azoth
{
namespace Vader
{
	struct AvatarsTimestampStorage::AvatarTimestamp
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> Email_;
		QDateTime TS_;

		static QString ClassName ()
		{
			return "AvatarTimestamps";
		}
	};
}
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Azoth::Vader::AvatarsTimestampStorage::AvatarTimestamp,
		Email_,
		TS_)

namespace LC
{
namespace Azoth
{
namespace Vader
{
	AvatarsTimestampStorage::AvatarsTimestampStorage ()
	: DB_ { "QSQLITE", "org.LeechCraft.Azoth.Vader.AvatarsTimestampStorage" }
	{
		if (!DB_->isOpen ())
		{
			const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth/vader");
			DB_->setDatabaseName (cacheDir.filePath ("avatar_timestamps.db"));
			if (!DB_->open ())
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open the database";
				Util::DBLock::DumpError (DB_->lastError ());
				throw std::runtime_error { "Cannot create database" };
			}

			Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
			Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");
		}

		Adapted_ = Util::oral::AdaptPtr<AvatarTimestamp> (DB_);
	}

	std::optional<QDateTime> AvatarsTimestampStorage::GetTimestamp (const QString& email)
	{
		namespace sph = Util::oral::sph;

		return Adapted_->SelectOne (sph::fields<&AvatarTimestamp::TS_>, sph::f<&AvatarTimestamp::Email_> == email);
	}

	void AvatarsTimestampStorage::SetTimestamp (const QString& full, const QDateTime& dt)
	{
		Adapted_->Insert ({ full, dt }, Util::oral::InsertAction::Replace::PKey<AvatarTimestamp>);
	}
}
}
}
