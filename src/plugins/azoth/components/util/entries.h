/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/trackingpointer.h>
#include "interfaces/azoth/azothcommon.h"
#include "interfaces/azoth/iclentry.h"

namespace LC::Azoth
{
	class IAccount;
	class IMUCEntry;
	class ISupportPGP;

	class EntriesManager;

	void AuthorizeEntry (ICLEntry*);
	void DenyAuthForEntry (ICLEntry*);

	bool ChoosePGPKey (ISupportPGP*, ICLEntry*);

	QStringList GetMucParticipants (const EntriesManager&, const GlobalPersistentId& mucEntryId);

	void TryInvite (const QList<Util::TrackingPointer<ICLEntry>>& entries,
			const Util::TrackingPointer<ICLEntry>& target,
			QWidget *parent);
}
