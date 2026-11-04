/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcardstorage.h"
#include <QDir>
#include <QSqlError>
#include <QXmlStreamWriter>
#include <QtDebug>
#include <util/db/dblock.h>
#include <util/db/util.h>
#include <util/db/oral/oral.h>
#include <util/sys/paths.h>
#include <util/azoth/entryidoral.h>

namespace LC::Azoth::Xoox
{
	struct VCardStorage::VCardRecord
	{
		Util::oral::PKey<EntryConventionalId, Util::oral::NoAutogen> Jid_;
		QString VCardIq_;

		constexpr static auto ClassName = "VCards"_ct;
	};

	struct VCardStorage::PhotoHashRecord
	{
		Util::oral::PKey<EntryConventionalId, Util::oral::NoAutogen> Jid_;
		QByteArray Hash_;

		constexpr static auto ClassName = "PhotoHashes"_ct;
	};
}

ORAL_ADAPT_STRUCT (LC::Azoth::Xoox::VCardStorage::VCardRecord,
		Jid_,
		VCardIq_)

ORAL_ADAPT_STRUCT (LC::Azoth::Xoox::VCardStorage::PhotoHashRecord,
		Jid_,
		Hash_)

namespace LC::Azoth::Xoox
{
	namespace sph = Util::oral::sph;

	VCardStorage::VCardStorage (QObject *parent)
	: QObject { parent }
	, DB_ { QSqlDatabase::addDatabase ("QSQLITE", Util::GenConnectionName ("org.LeechCraft.Azoth.Xoox.VCards")) }
	, VCardCache_ { 1024 * 1024 }
	{
		const auto& cacheDir = Util::GetUserDir (Util::UserDir::Cache, "azoth/xoox");
		DB_.setDatabaseName (cacheDir.filePath ("vcards.db"));
		if (!DB_.open ())
		{
			qWarning () << "cannot open the database";
			Util::DBLock::DumpError (DB_.lastError ());
			throw std::runtime_error { "Cannot create database" };
		}

		Util::RunTextQuery (DB_, "PRAGMA synchronous = NORMAL;");
		Util::RunTextQuery (DB_, "PRAGMA journal_mode = WAL;");

		AdaptedVCards_ = Util::oral::AdaptPtr<VCardRecord> (DB_);
		AdaptedPhotoHashes_ = Util::oral::AdaptPtr<PhotoHashRecord> (DB_);
	}

	VCardStorage::~VCardStorage () = default;

	void VCardStorage::SetVCard (const EntryConventionalId& id, const QString& vcard)
	{
		AdaptedVCards_->Insert ({ id, vcard }, Util::oral::InsertAction::Replace::Whole);
	}

	void VCardStorage::SetVCard (const EntryConventionalId& id, const QXmppVCardIq& vcard)
	{
		QString serialized;
		QXmlStreamWriter writer { &serialized };
		vcard.toXml (&writer);

		SetVCard (id, serialized);
	}

	std::optional<QXmppVCardIq> VCardStorage::GetVCard (const EntryConventionalId& id) const
	{
		if (const auto vcard = VCardCache_.object (id))
			return *vcard;

		const auto res = GetVCardString (id);
		if (!res)
			return {};

		QDomDocument vcardDoc;
		if (!vcardDoc.setContent (*res))
		{
			qWarning () << "unable to parse" << *res;
			return {};
		}

		QXmppVCardIq vcard;
		vcard.parse (vcardDoc.documentElement ());

		VCardCache_.insert (id, new QXmppVCardIq { vcard }, res->size ());

		return vcard;
	}

	void VCardStorage::SetVCardPhotoHash (const EntryConventionalId& id, const QByteArray& hash)
	{
		AdaptedPhotoHashes_->Insert ({ id, hash }, Util::oral::InsertAction::Replace::Whole);
	}

	std::optional<QByteArray> VCardStorage::GetVCardPhotoHash (const EntryConventionalId& id) const
	{
		return AdaptedPhotoHashes_->SelectOne (sph::fields<&PhotoHashRecord::Hash_>, sph::f<&PhotoHashRecord::Jid_> == id);
	}

	std::optional<QString> VCardStorage::GetVCardString (const EntryConventionalId& id) const
	{
		return AdaptedVCards_->SelectOne (sph::fields<&VCardRecord::VCardIq_>, sph::f<&VCardRecord::Jid_> == id);
	}
}
