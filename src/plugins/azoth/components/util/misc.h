/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/azoth/message.h>

template<typename>
class QFuture;

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

	bool OfferURLs (TransferJobManager& transfers, ICLEntry *entry, QList<QUrl> urls, QWidget *parent = nullptr);

	[[nodiscard]] QFuture<Entity> BuildNotification (AvatarsManager*, Entity, ICLEntry*, const QString& id = {}, ICLEntry* = nullptr);

	void InitiateAccountAddition (QWidget *parent = nullptr);
	void RemoveAccount (IAccount*);
}
