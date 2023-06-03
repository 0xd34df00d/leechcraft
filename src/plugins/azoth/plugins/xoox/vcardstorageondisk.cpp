/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcardstorageondisk.h"
#include <QDir>
#include <QSqlError>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	struct VCardStorageOnDisk::VCardRecord
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> JID_;
		QString VCardIq_;

		constexpr static auto ClassName = "VCards"_ct;
	};

	struct VCardStorageOnDisk::PhotoHashRecord
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> JID_;
		QByteArray Hash_;

		constexpr static auto ClassName = "PhotoHashes"_ct;
	};
}
}
}

ORAL_ADAPT_STRUCT (LC::Azoth::Xoox::VCardStorageOnDisk::VCardRecord,
		JID_,
		VCardIq_)

ORAL_ADAPT_STRUCT (LC::Azoth::Xoox::VCardStorageOnDisk::PhotoHashRecord,
		JID_,
		Hash_)

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	namespace sph = Util::oral::sph;

	VCardStorageOnDisk::VCardStorageOnDisk (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
				Util::GenConnectionName ("org.LeechCraft.Azoth.Xoox.VCards")) }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth/xoox");
		DB_.setDatabaseName (cacheDir.filePath ("vcards.db"));
		if (!DB_.open ())
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		AdaptedVCards_ = Util::oral::AdaptPtr<VCardRecord> (DB_);
		AdaptedPhotoHashes_ = Util::oral::AdaptPtr<PhotoHashRecord> (DB_);
	}

	VCardStorageOnDisk::~VCardStorageOnDisk () = default;

	void VCardStorageOnDisk::SetVCard (const QString& jid, const QString& vcard)
	{
		AdaptedVCards_->Insert ({ jid, vcard }, Util::oral::InsertAction::Replace::PKey);
	}

	std::optional<QString> VCardStorageOnDisk::GetVCard (const QString& jid) const
	{
		return AdaptedVCards_->SelectOne (sph::fields<&VCardRecord::VCardIq_>, sph::f<&VCardRecord::JID_> == jid);
	}

	void VCardStorageOnDisk::SetVCardPhotoHash (const QString& jid, const QByteArray& hash)
	{
		AdaptedPhotoHashes_->Insert ({ jid, hash }, Util::oral::InsertAction::Replace::PKey);
	}

	std::optional<QByteArray> VCardStorageOnDisk::GetVCardPhotoHash (const QString& jid) const
	{
		return AdaptedPhotoHashes_->SelectOne (sph::fields<&PhotoHashRecord::Hash_>, sph::f<&PhotoHashRecord::JID_> == jid);
	}
}
}
}
