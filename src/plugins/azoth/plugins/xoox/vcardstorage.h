/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <optional>
#include <QObject>
#include <QCache>
#include <QXmppVCardIq.h>
#include <util/threads/workerthreadbase.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class VCardStorageOnDisk;

	class VCardStorage : public QObject
	{
		VCardStorageOnDisk * const DB_;
		const std::shared_ptr<Util::WorkerThread<VCardStorageOnDisk>> Writer_;

		QMap<QString, QString> PendingVCards_;
		QMap<QString, QByteArray> PendingHashes_;

		mutable QCache<QString, QXmppVCardIq> VCardCache_;
	public:
		VCardStorage (QObject* = nullptr);

		void SetVCard (const QString& jid, const QString& vcard);
		void SetVCard (const QString& jid, const QXmppVCardIq& vcard);

		std::optional<QXmppVCardIq> GetVCard (const QString& jid) const;

		void SetVCardPhotoHash (const QString& jid, const QByteArray& hash);
		std::optional<QByteArray> GetVCardPhotoHash (const QString& jid) const;
	private:
		std::optional<QString> GetVCardString (const QString& jid) const;
	};
}
}
}
