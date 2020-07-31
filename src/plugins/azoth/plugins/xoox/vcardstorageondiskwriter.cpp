/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcardstorageondiskwriter.h"
#include "vcardstorageondisk.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	QFuture<void> VCardStorageOnDiskWriter::SetVCard (const QString& jid, const QString& vcard)
	{
		return ScheduleImpl ([=] { Storage_->SetVCard (jid, vcard); });
	}

	QFuture<void> VCardStorageOnDiskWriter::SetVCardPhotoHash (const QString& jid,
			const QByteArray& hash)
	{
		return ScheduleImpl ([=] { Storage_->SetVCardPhotoHash (jid, hash); });
	}

	void VCardStorageOnDiskWriter::Initialize ()
	{
		Storage_.reset (new VCardStorageOnDisk);
	}

	void VCardStorageOnDiskWriter::Cleanup ()
	{
		Storage_.reset ();
	}
}
}
}
