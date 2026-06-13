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

namespace LC::Azoth::Xoox
{
	struct VCardStorage::VCardRecord
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> JID_;
		QString VCardIq_;

		constexpr static auto ClassName = "VCards"_ct;
	};

	struct VCardStorage::PhotoHashRecord
	{
		Util::oral::PKey<QString, Util::oral::NoAutogen> JID_;
		QByteArray Hash_;

		constexpr static auto ClassName = "PhotoHashes"_ct;
	};
}

ORAL_ADAPT_STRUCT (LC::Azoth::Xoox::VCardStorage::VCardRecord,
		JID_,
		VCardIq_)

ORAL_ADAPT_STRUCT (LC::Azoth::Xoox::VCardStorage::PhotoHashRecord,
		JID_,
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

	void VCardStorage::SetVCard (const QString& jid, const QString& vcard)
	{
		AdaptedVCards_->Insert ({ jid, vcard }, Util::oral::InsertAction::Replace::Whole);
	}

	void VCardStorage::SetVCard (const QString& jid, const QXmppVCardIq& vcard)
	{
		QString serialized;
		QXmlStreamWriter writer { &serialized };
		vcard.toXml (&writer);

		SetVCard (jid, serialized);
	}

	std::optional<QXmppVCardIq> VCardStorage::GetVCard (const QString& jid) const
	{
		if (const auto vcard = VCardCache_.object (jid))
			return *vcard;

		const auto res = GetVCardString (jid);
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

		VCardCache_.insert (jid, new QXmppVCardIq { vcard }, res->size ());

		return vcard;
	}

	void VCardStorage::SetVCardPhotoHash (const QString& jid, const QByteArray& hash)
	{
		AdaptedPhotoHashes_->Insert ({ jid, hash }, Util::oral::InsertAction::Replace::Whole);
	}

	std::optional<QByteArray> VCardStorage::GetVCardPhotoHash (const QString& jid) const
	{
		return AdaptedPhotoHashes_->SelectOne (sph::fields<&PhotoHashRecord::Hash_>, sph::f<&PhotoHashRecord::JID_> == jid);
	}

	std::optional<QString> VCardStorage::GetVCardString (const QString& jid) const
	{
		return AdaptedVCards_->SelectOne (sph::fields<&VCardRecord::VCardIq_>, sph::f<&VCardRecord::JID_> == jid);
	}
}
