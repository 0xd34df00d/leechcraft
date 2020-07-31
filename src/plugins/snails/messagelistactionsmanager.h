/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include "messagelistactioninfo.h"

namespace LC
{
namespace Snails
{
	class MessageListActionsProvider;
	class Account;

	class MessageListActionsManager : public QObject
	{
		Account * const Acc_;
		QList<std::shared_ptr<MessageListActionsProvider>> Providers_;
	public:
		MessageListActionsManager (Account*, QObject* = nullptr);

		QList<MessageListActionInfo> GetMessageActions (const MessageInfo&) const;
	};
}
}
