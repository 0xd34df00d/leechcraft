/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "chattabsmanager.h"
#include <QtDebug>
#include "interfaces/iclentry.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	ChatTabsManager::ChatTabsManager(QObject *parent)
	: QObject (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("ChatWindowStyle",
				this, "chatWindowStyleChanged");
		XmlSettingsManager::Instance ().RegisterObject ("CustomMUCStyle",
				this, "chatWindowStyleChanged");
		XmlSettingsManager::Instance ().RegisterObject ("MUCWindowStyle",
				this, "chatWindowStyleChanged");
	}

	void ChatTabsManager::OpenChat (const QModelIndex& ti)
	{
		if (!ti.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to open a chat with invalid index";
			return;
		}

		QObject *entryObj = ti.data (Core::CLREntryObject).value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "object"
					<< entryObj
					<< "from the index"
					<< ti
					<< "doesn't implement ICLEntry";
			return;
		}

		OpenChat (entry);
	}

	QWidget* ChatTabsManager::OpenChat (const ICLEntry *entry)
	{
		const QString& id = entry->GetEntryID ();
		if (Entry2Tab_.contains (id))
		{
			emit raiseTab (Entry2Tab_ [id]);
			return Entry2Tab_ [id];
		}

		EverOpened_ << id;

		QPointer<ChatTab> tab (new ChatTab (id));
		tab->installEventFilter (this);
		Entry2Tab_ [id] = tab;

		connect (tab,
				SIGNAL (needToClose (ChatTab*)),
				this,
				SLOT (handleNeedToClose (ChatTab*)));
		connect (tab,
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SIGNAL (clearUnreadMsgCount (QObject*)));
		connect (tab,
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SIGNAL (entryMadeCurrent (QObject*)));
		connect (tab,
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SIGNAL (changeTabName (QWidget*, const QString&)));
		connect (tab,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)));

		emit addNewTab (entry->GetEntryName (), tab);

		tab->HasBeenAdded ();

		if (XmlSettingsManager::Instance ()
				.property ("JumpToNewTabOnOpen").toBool ())
			emit raiseTab (tab);

		return tab;
	}

	void ChatTabsManager::CloseChat (const ICLEntry *entry)
	{
		const QString& id = entry->GetEntryID ();
		if (!Entry2Tab_.contains (id))
			return;

		handleNeedToClose (Entry2Tab_ [id]);
	}

	bool ChatTabsManager::IsActiveChat (const ICLEntry *entry) const
	{
		if (!Entry2Tab_.contains (entry->GetEntryID ()))
			return false;

		return Entry2Tab_ [entry->GetEntryID ()]->isVisible () &&
			Entry2Tab_ [entry->GetEntryID ()]->isActiveWindow ();
	}

	bool ChatTabsManager::IsOpenedChat (const QString& id) const
	{
		return EverOpened_.contains (id);
	}

	void ChatTabsManager::UpdateEntryMapping (const QString& id, QObject *obj)
	{
		if (!Entry2Tab_.contains (id))
			return;

		connect (obj,
				SIGNAL (gotMessage (QObject*)),
				Entry2Tab_ [id],
				SLOT (handleEntryMessage (QObject*)),
				Qt::UniqueConnection);
	}

	void ChatTabsManager::HandleEntryAdded (ICLEntry *entry)
	{
		if (entry->GetEntryType () != ICLEntry::ETPrivateChat)
			return;

		QObject *mucObj = entry->GetParentCLEntry ();
		ICLEntry *muc = qobject_cast<ICLEntry*> (mucObj);
		UpdateMUCTab (muc);
	}

	void ChatTabsManager::HandleEntryRemoved (ICLEntry *entry)
	{
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat)
		{
			QObject *mucObj = entry->GetParentCLEntry ();
			ICLEntry *muc = qobject_cast<ICLEntry*> (mucObj);
			UpdateMUCTab (muc);
		}

		if (!Entry2Tab_.contains (entry->GetEntryID ()))
			return;

		SetChatEnabled (entry->GetEntryID (), false);
		disconnect (entry->GetObject (),
				0,
				this,
				0);
		disconnect (entry->GetObject (),
				0,
				Entry2Tab_ [entry->GetEntryID ()],
				0);
	}

	void ChatTabsManager::SetChatEnabled (const QString& id, bool enabled)
	{
		if (!Entry2Tab_.contains (id))
			return;

		Entry2Tab_ [id]->setEnabled (enabled);
	}

	void ChatTabsManager::ChatMadeCurrent (ChatTab *curTab)
	{
		Q_FOREACH (ChatTab_ptr tab, Entry2Tab_.values ())
			if (tab != curTab)
				tab->TabLostCurrent ();

		ICLEntry *entry = qobject_cast<ICLEntry*> (curTab->GetCLEntry ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "chat's tab is not an ICLEntry";
			return;
		}
		entry->MarkMsgsRead ();
	}

	bool ChatTabsManager::eventFilter (QObject* obj, QEvent *event)
	{
		if (event->type () != QEvent::FocusIn &&
				event->type () != QEvent::WindowActivate)
			return false;

		ChatTab *tab = qobject_cast<ChatTab*> (obj);
		if (!tab)
			return false;

		tab->TabMadeCurrent ();

		return false;
	}

	void ChatTabsManager::UpdateMUCTab (ICLEntry *muc)
	{
		if (!muc)
		{
			qWarning () << Q_FUNC_INFO
					<< "passed obj doesn't implement ICLEntry";
			return;
		}

		if (Entry2Tab_.contains (muc->GetEntryID ()))
			Entry2Tab_ [muc->GetEntryID ()]->HandleMUCParticipantsChanged ();
	}

	void ChatTabsManager::handleNeedToClose (ChatTab *tab)
	{
		emit removeTab (tab);

		const QString& entry = Entry2Tab_.key (tab);
		Entry2Tab_.remove (entry);

		tab->deleteLater ();
	}

	void ChatTabsManager::chatWindowStyleChanged ()
	{
		Q_FOREACH (ChatTab_ptr tab, Entry2Tab_.values ())
			tab->PrepareTheme ();
	}
}
}
