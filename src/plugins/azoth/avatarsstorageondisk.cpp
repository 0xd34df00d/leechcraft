/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarsstorageondisk.h"
#include <QDir>
#include <QSqlError>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Azoth
{
	struct AvatarsStorageOnDisk::Record
	{
		Util::oral::PKey<int> ID_;

		QByteArray EntryID_;
		IHaveAvatars::Size Size_;
		QByteArray ImageData_;

		static QByteArray ClassName ()
		{
			return "Record";
		}

		using Constraints = Util::oral::Constraints<
				Util::oral::UniqueSubset<1, 2>
			>;
	};
}
}

BOOST_FUSION_ADAPT_STRUCT (LC::Azoth::AvatarsStorageOnDisk::Record,
		ID_,
		EntryID_,
		Size_,
		ImageData_)

namespace LC
{
namespace Util
{
namespace oral
{
	template<typename ImplFactory>
	struct Type2Name<ImplFactory, Azoth::IHaveAvatars::Size>
	{
		auto operator() () const
		{
			return Type2Name<ImplFactory, int> {} ();
		}
	};

	template<>
	struct ToVariant<Azoth::IHaveAvatars::Size>
	{
		QVariant operator() (Azoth::IHaveAvatars::Size size) const
		{
			return static_cast<int> (size);
		}
	};

	template<>
	struct FromVariant<Azoth::IHaveAvatars::Size>
	{
		Azoth::IHaveAvatars::Size operator() (const QVariant& var) const
		{
			return static_cast<Azoth::IHaveAvatars::Size> (var.toInt ());
		}
	};
}
}

namespace Azoth
{
	namespace sph = Util::oral::sph;

	AvatarsStorageOnDisk::AvatarsStorageOnDisk (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE",
				Util::GenConnectionName ("org.LeechCraft.Azoth.Avatars")) }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth");
		DB_.setDatabaseName (cacheDir.filePath ("avatars.db"));
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

	void AvatarsStorageOnDisk::SetAvatar (const QString& entryId,
			IHaveAvatars::Size size, const QByteArray& imageData) const
	{
		AdaptedRecord_->Insert ({ {}, entryId.toUtf8 (), size, imageData },
				Util::oral::InsertAction::Replace::Fields<&Record::EntryID_, &Record::Size_>);
	}

	std::optional<QByteArray> AvatarsStorageOnDisk::GetAvatar (const QString& entryId, IHaveAvatars::Size size) const
	{
		return AdaptedRecord_->SelectOne (sph::fields<&Record::ImageData_>,
				sph::f<&Record::EntryID_> == entryId.toUtf8 () && sph::f<&Record::Size_> == size);
	}

	void AvatarsStorageOnDisk::DeleteAvatars (const QString& entryId) const
	{
		AdaptedRecord_->DeleteBy (sph::f<&Record::EntryID_> == entryId.toUtf8 ());
	}
}
}
