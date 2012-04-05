/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	ChatTabsManager::ChatTabsManager(QObject *parent)
	: QObject (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("CustomMUCStyle",
				this, "chatWindowStyleChanged");

		auto regStyle = [this] (const QByteArray& style)
		{
			XmlSettingsManager::Instance ().RegisterObject (style,
					this, "chatWindowStyleChanged");
			XmlSettingsManager::Instance ().RegisterObject (style + "Variant",
					this, "chatWindowStyleChanged");
		};
		regStyle ("ChatWindowStyle");
		regStyle ("MUCWindowStyle");
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

	QWidget* ChatTabsManager::OpenChat (const ICLEntry *entry, const DynPropertiesList_t& props)
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

		Q_FOREACH (const auto& prop, props)
			tab->setProperty (prop.first, prop.second);

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

	void ChatTabsManager::HandleInMessage (IMessage *msg)
	{
		if (!XmlSettingsManager::Instance ().property ("OpenTabOnNewMsg").toBool ())
			return;

		if (msg->GetMessageType () == IMessage::MTChatMessage)
			OpenChat (qobject_cast<ICLEntry*> (msg->OtherPart ()));
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

	void ChatTabsManager::EnqueueRestoreInfos (const QList<RestoreChatInfo>& infos)
	{
		Q_FOREACH (const RestoreChatInfo& info, infos)
		{
			auto entryObj = Core::Instance ().GetEntry (info.EntryID_);
			qDebug () << Q_FUNC_INFO << info.EntryID_ << entryObj;
			if (entryObj)
				RestoreChat (info, entryObj);
			else
				RestoreInfo_ [info.EntryID_] = info;
		}
	}

	QString ChatTabsManager::GetActiveVariant (ICLEntry *entry) const
	{
		ChatTab_ptr tab = Entry2Tab_ [entry->GetEntryID ()];
		if (!tab)
			return QString ();

		return tab->GetSelectedVariant ();
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

	void ChatTabsManager::RestoreChat (const ChatTabsManager::RestoreChatInfo& info, QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "null entry for"
					<< entryObj;
			return;
		}
		auto tab = qobject_cast<ChatTab*> (OpenChat (entry, info.Props_));
		tab->selectVariant (info.Variant_);
	}

	void ChatTabsManager::handleNeedToClose (ChatTab *tab)
	{
		emit removeTab (tab);

		const QString& entry = Entry2Tab_.key (tab);
		Entry2Tab_.remove (entry);

		tab->deleteLater ();

		if (XmlSettingsManager::Instance ().property ("LeaveConfOnClose").toBool ())
		{
			QObject *entryObj = tab->GetCLEntry ();
			IMUCEntry *muc = qobject_cast<IMUCEntry*> (entryObj);
			if (muc)
				muc->Leave ();
		}
	}

	void ChatTabsManager::handleAddingCLEntryEnd (IHookProxy_ptr,
			QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto& id = entry->GetHumanReadableID ();
		if (!RestoreInfo_.contains (id))
			return;

		RestoreChat (RestoreInfo_.take (id), entryObj);
	}

	void ChatTabsManager::chatWindowStyleChanged ()
	{
		QSet<QString> params;
		auto upSet = [&params] (const QByteArray& style)
		{
			params << XmlSettingsManager::Instance ()
					.property (style).toString ();
			params << XmlSettingsManager::Instance ()
					.property (style + "Variant").toString ();
		};
		upSet ("ChatWindowStyle");
		upSet ("MUCWindowStyle");

		const QString& custId = XmlSettingsManager::Instance ()
					.property ("CustomMUCStyle").toBool () ? "t" : "f";
		params << custId;

		if (params == StyleParams_)
			return;

		StyleParams_ = params;
		Q_FOREACH (ChatTab_ptr tab, Entry2Tab_.values ())
			tab->PrepareTheme ();
	}
}
}
