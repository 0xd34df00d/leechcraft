/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QCache>
#include <QSqlDatabase>
#include <QXmppVCardIq.h>
#include <util/db/oral/oralfwd.h>

namespace LC::Azoth::Xoox
{
	class VCardStorage : public QObject
	{
	public:
		struct VCardRecord;
		struct PhotoHashRecord;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<VCardRecord> AdaptedVCards_;
		Util::oral::ObjectInfo_ptr<PhotoHashRecord> AdaptedPhotoHashes_;

		mutable QCache<QString, QXmppVCardIq> VCardCache_;
	public:
		explicit VCardStorage (QObject* = nullptr);
		~VCardStorage ();

		void SetVCard (const QString& id, const QString& vcard);
		void SetVCard (const QString& id, const QXmppVCardIq& vcard);

		std::optional<QXmppVCardIq> GetVCard (const QString& id) const;

		void SetVCardPhotoHash (const QString& id, const QByteArray& hash);
		std::optional<QByteArray> GetVCardPhotoHash (const QString& id) const;
	private:
		std::optional<QString> GetVCardString (const QString& id) const;
	};
}
