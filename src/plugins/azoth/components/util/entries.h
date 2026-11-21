/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <util/sll/trackingpointer.h>
#include "interfaces/azoth/azothcommon.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucentry.h"

namespace LC::Azoth
{
	class IAccount;

	class EntriesManager;

	void AuthorizeEntry (ICLEntry*);
	void DenyAuthForEntry (ICLEntry*);

	QStringList GetMucParticipants (const EntriesManager&, const GlobalPersistentId& mucEntryId);

	struct NotAnInvitation {};

	struct InviteRefusal
	{
		QString Message_;
	};

	struct InvitePlan
	{
		Util::TrackingPointer<IMUCEntry> Muc_;
		std::variant<Util::TrackingPointer<ICLEntry>, QString> Invitee_;
		QString InviteeLabel_;
	};

	using InviteCheck_t = std::variant<NotAnInvitation, InviteRefusal, InvitePlan>;

	InviteCheck_t CheckInvitation (ICLEntry& entry, ICLEntry& target);
}
