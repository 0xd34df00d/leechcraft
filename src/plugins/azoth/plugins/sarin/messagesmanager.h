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
#include <QPointer>
#include <QHash>
#include <util/threads/coro/taskfwd.h>
#include "types.h"

namespace LC::Azoth::Sarin
{
	class ToxAccount;
	class ToxRunner;
	class ChatMessage;

	class MessagesManager : public QObject
	{
		ToxAccount& Acc_;
		QHash<uint32_t, QPointer<ChatMessage>> MsgId2Msg_;
	public:
		explicit MessagesManager (ToxAccount&);

		Util::ContextTask<void> SendMessage (Pubkey pkey, QPointer<ChatMessage>);
	private:
		void HandleReadReceipt (quint32);
		Util::ContextTask<void> HandleInMessage (qint32, QString);

		void SetThread (const std::shared_ptr<ToxRunner>&);
	};
}
