/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unreadqueuemanager.h"
#include <QMainWindow>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "chattabsmanager.h"

uint qHash (const QPointer<QObject>& ptr)
{
	return qHash (ptr.data ());
}

namespace LC
{
namespace Azoth
{
	UnreadQueueManager::UnreadQueueManager (QObject *parent)
	: QObject (parent)
	{
	}

	QObject* UnreadQueueManager::GetFirstUnreadMessage (QObject* entryObj) const
	{
		return Entry2FirstUnread_.value (entryObj);
	}

	void UnreadQueueManager::AddMessage (QObject *msgObj)
	{
		const auto msg = qobject_cast<IMessage*> (msgObj);
		const auto entryObj = msg->ParentCLEntry ();
		if (!Queue_.contains (entryObj))
		{
			Queue_ << entryObj;
			Entry2FirstUnread_ [entryObj] = msgObj;
		}

		UnreadMessages_ << msgObj;
	}

	bool UnreadQueueManager::IsMessageRead (QObject *msgObj) const
	{
		return !UnreadMessages_.contains (msgObj);
	}

	void UnreadQueueManager::ShowNext ()
	{
		QObject *entryObj = nullptr;
		while (!Queue_.isEmpty () && !entryObj)
			entryObj = Queue_.takeFirst ();
		if (!entryObj)
			return;

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto chatWidget = Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true);

		const auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		const auto idx = rootWM->GetWindowForTab (qobject_cast<ITabWidget*> (chatWidget));
		if (const auto mw = rootWM->GetMainWindow (idx))
		{
			mw->show ();
			mw->raise ();
			mw->activateWindow ();
		}
	}

	void UnreadQueueManager::clearMessagesForEntry (QObject *entryObj)
	{
		Queue_.removeAll (entryObj);

		for (auto pos = UnreadMessages_.begin (); pos != UnreadMessages_.end (); )
			if (!*pos || qobject_cast<IMessage*> (*pos)->ParentCLEntry () == entryObj)
				pos = UnreadMessages_.erase (pos);
			else
				++pos;

		emit messagesCleared (entryObj);
	}
}
}
