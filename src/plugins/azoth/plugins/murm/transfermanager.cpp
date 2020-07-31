/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "transfermanager.h"
#include "pendingupload.h"
#include "vkaccount.h"
#include "vkentry.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	TransferManager::TransferManager (VkAccount *acc)
	: QObject { acc }
	, Acc_ { acc }
	{
	}

	bool TransferManager::IsAvailable () const
	{
		return true;
	}

	QObject* TransferManager::SendFile (const QString& id,
			const QString&, const QString& name, const QString& comment)
	{
		const auto entry = Acc_->FindEntryById (id);
		if (!entry)
			return nullptr;

		return new PendingUpload { entry, name, comment, Acc_ };
	}
}
}
}
