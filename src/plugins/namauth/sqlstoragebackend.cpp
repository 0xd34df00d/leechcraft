/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sqlstoragebackend.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>

ORAL_ADAPT_STRUCT (LC::NamAuth::SQLStorageBackend::AuthRecord,
		RealmName_,
		Context_,
		Login_,
		Password_)

namespace LC::NamAuth
{
	SQLStorageBackend::SQLStorageBackend ()
	: DB_ (std::make_shared<QSqlDatabase> (QSqlDatabase::addDatabase ("QSQLITE"_qs,
			Util::GenConnectionName ("NamAuth.Connection"_qs))))
	{
		DB_->setDatabaseName (GetDBPath ());
		if (!DB_->open ())
		{
			Util::DBLock::DumpError (DB_->lastError ());
			return;
		}

		AdaptedRecord_ = Util::oral::AdaptPtr<AuthRecord> (*DB_);
	}

	SQLStorageBackend::~SQLStorageBackend () = default;

	QString SQLStorageBackend::GetDBPath ()
	{
		return Util::CreateIfNotExists ("core"_qs).filePath ("core.db"_qs);
	}

	namespace sph = Util::oral::sph;

	std::optional<SQLStorageBackend::AuthRecord> SQLStorageBackend::GetAuth (const QString& realm, const QString& context)
	{
		return AdaptedRecord_->SelectOne (sph::f<&AuthRecord::RealmName_> == realm && sph::f<&AuthRecord::Context_> == context);
	}

	void SQLStorageBackend::SetAuth (const AuthRecord& record)
	{
		AdaptedRecord_->Insert (record,
				Util::oral::InsertAction::Replace::Fields<&AuthRecord::RealmName_, &AuthRecord::Context_>);
	}
}
