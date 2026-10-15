/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/sll/qtutil.h>
#include <interfaces/azoth/message.h>

namespace LC
{
	struct Entity;
}

namespace LC::Azoth
{
	class IAccount;
	class ICLEntry;

	class AvatarsManager;
	class TransferJobManager;

	bool SendMessage (ICLEntry& e, OutgoingMessage);

	bool OfferURLs (TransferJobManager& transfers, ICLEntry *entry, const QList<QUrl>& urls, QWidget *parent = nullptr);

	struct NotificationInfo
	{
		AvatarsManager& Avatars_;
		ICLEntry& Conversation_;
		ICLEntry& Actor_ = Conversation_;
		QString EventId_ = "IncomingMessageFrom"_qs;
	};
	[[nodiscard]]
	Entity BuildNotification (Entity prefilled, NotificationInfo info);

	void BuildAndSendNotification (Entity prefilled, NotificationInfo info);

	void InitiateAccountAddition (QWidget *parent = nullptr);
	void RemoveAccount (IAccount*);
}
