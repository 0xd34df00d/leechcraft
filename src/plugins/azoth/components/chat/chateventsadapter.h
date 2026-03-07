/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::Azoth
{
	enum ChatPartState : std::uint8_t;

	struct ChatEvent;

	class ICLEntry;

	class ChatEventsAdapter : public QObject
	{
		Q_OBJECT

		ICLEntry& Entry_;
	public:
		explicit ChatEventsAdapter (ICLEntry&);
	private:
		void HandleChatState (ChatPartState, const QString&);
	signals:
		void gotEvent (const ChatEvent&);
	};
}
