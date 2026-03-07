/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chateventsadapter.h"
#include "interfaces/azoth/iclentry.h"
#include "../../xmlsettingsmanager.h"

namespace LC::Azoth
{
	ChatEventsAdapter::ChatEventsAdapter (ICLEntry& entry)
	: QObject { entry.GetQObject () }
	, Entry_ { entry }
	{
		auto& emitter = entry.GetCLEntryEmitter ();
		if (Entry_.GetEntryType () != ICLEntry::EntryType::MUC)
			connect (&emitter,
					&Emitters::CLEntry::chatPartStateChanged,
					this,
					&ChatEventsAdapter::HandleChatState);
	}

	void ChatEventsAdapter::HandleChatState (ChatPartState state, const QString& variant)
	{
		if (state != CPSGone)
			return;

		if (!XmlSettingsManager::Instance ().property ("ShowEndConversations").toBool ())
			return;

		const auto& id = variant.isEmpty () ? Entry_.GetEntryName () : Entry_.GetEntryName () + '/' + variant;
		emit gotEvent ({ .Text_ = Util::TaintedString { tr ("%1 has left the conversation").arg (id) }  });
	}
}
