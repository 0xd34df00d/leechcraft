/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class VCardStorageOnDisk : public QObject
	{
	public:
		struct VCardRecord;
		struct PhotoHashRecord;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<VCardRecord> AdaptedVCards_;
		Util::oral::ObjectInfo_ptr<PhotoHashRecord> AdaptedPhotoHashes_;
	public:
		VCardStorageOnDisk (QObject* = nullptr);

		void SetVCard (const QString& jid, const QString& vcard);
		std::optional<QString> GetVCard (const QString& jid) const;

		void SetVCardPhotoHash (const QString& jid, const QByteArray& hash);
		std::optional<QByteArray> GetVCardPhotoHash (const QString& jid) const;
	};
}
}
}
