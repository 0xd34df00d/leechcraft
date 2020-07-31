/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcardstorage.h"
#include <QXmlStreamWriter>
#include <QtDebug>
#include <util/threads/futures.h>
#include "vcardstorageondisk.h"
#include "vcardstorageondiskwriter.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	VCardStorage::VCardStorage (QObject *parent)
	: QObject { parent }
	, DB_ { new VCardStorageOnDisk { this } }
	, Writer_
	{
		new VCardStorageOnDiskWriter,
		[] (VCardStorageOnDiskWriter *writer)
		{
			writer->quit ();
			writer->wait (5000);
			delete writer;
		}
	}
	, VCardCache_ { 1024 * 1024 }
	{
		Writer_->start (QThread::IdlePriority);
	}

	void VCardStorage::SetVCard (const QString& jid, const QString& vcard)
	{
		PendingVCards_ [jid] = vcard;

		Util::Sequence (this, Writer_->SetVCard (jid, vcard)) >>
				[this, jid] { PendingVCards_.remove (jid); };
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
			qWarning () << Q_FUNC_INFO
					<< "unable to parse"
					<< *res;
			return {};
		}

		QXmppVCardIq vcard;
		vcard.parse (vcardDoc.documentElement ());

		VCardCache_.insert (jid, new QXmppVCardIq { vcard }, res->size ());

		return vcard;
	}

	void VCardStorage::SetVCardPhotoHash (const QString& jid, const QByteArray& hash)
	{
		PendingHashes_ [jid] = hash;

		Util::Sequence (this, Writer_->SetVCardPhotoHash (jid, hash)) >>
				[this, jid] { PendingHashes_.remove (jid); };
	}

	std::optional<QByteArray> VCardStorage::GetVCardPhotoHash (const QString& jid) const
	{
		if (PendingHashes_.contains (jid))
			return PendingHashes_.value (jid);

		return DB_->GetVCardPhotoHash (jid);
	}

	std::optional<QString> VCardStorage::GetVCardString (const QString& jid) const
	{
		if (PendingVCards_.contains (jid))
			return PendingVCards_.value (jid);

		return DB_->GetVCard (jid);
	}
}
}
}
