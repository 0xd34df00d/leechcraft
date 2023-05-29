/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "photourlstorage.h"
#include <QUrl>
#include <QDir>
#include <QSqlError>
#include <util/db/oral/oral.h>
#include <util/db/dblock.h>
#include <util/sll/functor.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	struct PhotoUrlStorage::Record
	{
		Util::oral::PKey<qulonglong, Util::oral::NoAutogen> UserNum_;
		QByteArray BigPhotoUrl_;

		constexpr static auto ClassName = "PhotoUrls"_ct;
	};
}
}
}

ORAL_ADAPT_STRUCT (LC::Azoth::Murm::PhotoUrlStorage::Record,
		UserNum_,
		BigPhotoUrl_)

namespace LC
{
namespace Azoth
{
namespace Murm
{
	PhotoUrlStorage::PhotoUrlStorage (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LeechCraft.Azoth.Murm.PhotoUrls")) }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth/murm");
		DB_.setDatabaseName (cacheDir.filePath ("photourls.db"));
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

	std::optional<QUrl> PhotoUrlStorage::GetUserUrl (qulonglong userId)
	{
		using namespace Util::oral::sph;
		using namespace Util;

		return [] (const QByteArray& ba) { return QUrl::fromEncoded (ba); } *
				AdaptedRecord_->SelectOne (fields<&Record::BigPhotoUrl_>, f<&Record::UserNum_> == userId);
	}

	void PhotoUrlStorage::SetUserUrl (qulonglong userId, const QUrl& url)
	{
		AdaptedRecord_->Insert ({ userId, url.toEncoded () }, Util::oral::InsertAction::Replace::PKey);
	}
}
}
}
