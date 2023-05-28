/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sslerrorschoicestorage.h"
#include <stdexcept>
#include <QDir>
#include <QSqlError>
#include <util/sys/paths.h>
#include <util/db/util.h>
#include <util/db/dblock.h>
#include <util/db/oral/oral.h>
#include <util/db/oral/migrate.h>

namespace LC
{
namespace Azoth
{
	struct SslErrorsChoiceStorage::Record
	{
		QByteArray AccountID_;
		QSslError::SslError Error_;
		SslErrorsChoiceStorage::Action Action_;

		using Constraints = Util::oral::Constraints<Util::oral::PrimaryKey<0, 1>>;

		constexpr static auto ClassName ()
		{
			return "SslErrors"_ct;
		}
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Azoth::SslErrorsChoiceStorage::Record,
		AccountID_,
		Error_,
		Action_)

namespace LC
{
namespace Azoth
{
	SslErrorsChoiceStorage::SslErrorsChoiceStorage ()
	: DB_ { QSqlDatabase::addDatabase ("QSQLITE",
			Util::GenConnectionName ("org.LeechCraft.Azoth.SslErrors")) }
	{
		const auto& dbDir = Util::GetUserDir (Util::UserDir::LC, "azoth");
		DB_.setDatabaseName (dbDir.filePath ("sslerrors.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		Util::oral::Migrate<Record> (DB_);

		AdaptedRecord_ = Util::oral::AdaptPtr<Record> (DB_);
	}

	namespace sph = Util::oral::sph;

	auto SslErrorsChoiceStorage::GetAction (const QByteArray& id,
			QSslError::SslError err) const -> std::optional<Action>
	{
		return AdaptedRecord_->SelectOne (sph::fields<&Record::Action_>,
				sph::f<&Record::AccountID_> == id && sph::f<&Record::Error_> == err);
	}

	void SslErrorsChoiceStorage::SetAction (const QByteArray& id,
			QSslError::SslError err, Action act)
	{
		AdaptedRecord_->Insert ({ id, err, act },
				Util::oral::InsertAction::Replace::Fields<&Record::AccountID_, &Record::Error_>);
	}
}
}
