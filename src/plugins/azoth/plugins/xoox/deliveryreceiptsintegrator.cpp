/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deliveryreceiptsintegrator.h"
#include <QPointer>
#include <QXmppMessageReceiptManager.h>
#include "glooxmessage.h"

namespace LC::Azoth::Xoox
{
	DeliveryReceiptsIntegrator::DeliveryReceiptsIntegrator (QXmppMessageReceiptManager& mgr)
	{
		connect (&mgr,
				&QXmppMessageReceiptManager::messageDelivered,
				this,
				[this] (const QString&, const QString& msgId)
				{
					if (const auto msg = UndeliveredMessages_.take (msgId))
						msg->SetDelivered (true);
				});
	}

	void DeliveryReceiptsIntegrator::ProcessMessage (GlooxMessage& msgObj)
	{
		if (msgObj.GetMessageType () == IMessage::Type::ChatMessage)
		{
			msgObj.SetReceiptRequested (true);
			UndeliveredMessages_ [msgObj.GetNativeMessage ().id ()] = &msgObj;
		}
	}
}
