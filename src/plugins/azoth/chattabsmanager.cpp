/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattabsmanager.h"
#include <QWebEngineProfile>
#include <QtDebug>
#include "interfaces/azoth/iclentry.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "avatarsmanager.h"
#include "azothschemehandler.h"

namespace LC::Azoth
{
	ChatTabsManager::ChatTabsManager (AvatarsManager *am, Util::WkFontsWidget *fontsWidget, QObject *parent)
	: QObject { parent }
	, AvatarsManager_ { am }
	, FontsWidget_ { fontsWidget }
	, Profile_ { new QWebEngineProfile { this } }
	{
		Profile_->installUrlSchemeHandler ("azoth", new AzothSchemeHandler { am, this });
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

	QWidget* ChatTabsManager::OpenChat (const QModelIndex& ti)
	{
		if (!ti.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "tried to open a chat with invalid index";
			return nullptr;
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
			return nullptr;
		}

		return OpenChat (entry, true);
	}

	ChatTab* ChatTabsManager::OpenChat (const ICLEntry *entry,
			bool fromUser, const DynPropertiesList_t& props)
	{
		const auto& id = entry->GetEntryID ();
		if (Entry2Tab_.contains (id))
		{
			auto tab = Entry2Tab_ [id];
			emit raiseTab (tab);
			return tab;
		}

		if (!Core::Instance ().GetEntry (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "the entry"
					<< id
					<< "is obviously alive, but Core doesn't know about it."
					<< "Will wait for re-appearing.";

			EnqueueRestoreInfos ({ { id, {}, {}, {} } });

			return nullptr;
		}

		EverOpened_ << id;

		QPointer<ChatTab> tab (new ChatTab (id, entry->GetParentAccount (), AvatarsManager_, FontsWidget_, Profile_));
		tab->installEventFilter (this);
		Entry2Tab_ [id] = tab;

		for (const auto& prop : props)
			tab->setProperty (prop.first, prop.second);

		connect (tab,
				SIGNAL (needToClose (ChatTab*)),
				this,
				SLOT (handleNeedToClose (ChatTab*)));
		connect (tab,
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SIGNAL (entryMadeCurrent (QObject*)));
		connect (tab,
				SIGNAL (entryMadeCurrent (QObject*)),
				this,
				SLOT (updateCurrentTab (QObject*)));
		connect (tab,
				SIGNAL (entryLostCurrent (QObject*)),
				this,
				SIGNAL (entryLostCurrent (QObject*)));
		connect (tab,
				SIGNAL (changeTabName (QWidget*, const QString&)),
				this,
				SIGNAL (changeTabName (QWidget*, const QString&)));
		connect (tab,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
				this,
				SIGNAL (changeTabIcon (QWidget*, const QIcon&)));

		emit addNewTab (tab->ReformatTitle (), tab);

		tab->HasBeenAdded ();

		if (fromUser || XmlSettingsManager::Instance ()
				.property ("JumpToNewTabOnOpen").toBool ())
			emit raiseTab (tab);

		return tab;
	}

	void ChatTabsManager::CloseChat (const ICLEntry *entry, bool fromUser)
	{
		const QString& id = entry->GetEntryID ();
		if (!Entry2Tab_.contains (id))
			return;

		CloseChatTab (Entry2Tab_ [id], fromUser);
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

	ChatTab* ChatTabsManager::GetActiveChatTab () const
	{
		return LastCurrentTab_;
	}

	ChatTab* ChatTabsManager::GetChatTab (const QString& entryId) const
	{
		return Entry2Tab_ [entryId];
	}

	void ChatTabsManager::UpdateEntryMapping (const QString& id)
	{
		if (!Entry2Tab_.contains (id))
			return;

		const auto tab = Entry2Tab_ [id];
		tab->SetEnabled (true);
		tab->ReinitEntry ();
	}

	void ChatTabsManager::HandleEntryAdded (ICLEntry *entry)
	{
		if (entry->GetEntryType () != ICLEntry::EntryType::PrivateChat)
			return;

		UpdateMUCTab (entry->GetParentCLEntry ());
	}

	void ChatTabsManager::HandleEntryRemoved (ICLEntry *entry)
	{
		if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
			UpdateMUCTab (entry->GetParentCLEntry ());

		if (!Entry2Tab_.contains (entry->GetEntryID ()))
			return;

		const auto tab = Entry2Tab_ [entry->GetEntryID ()];
		tab->SetEnabled (false);
		disconnect (entry->GetQObject (),
				0,
				this,
				0);
		disconnect (entry->GetQObject (),
				0,
				tab,
				0);
	}

	void ChatTabsManager::HandleInMessage (IMessage *msg)
	{
		if (!XmlSettingsManager::Instance ().property ("OpenTabOnNewMsg").toBool ())
			return;

		if (msg->GetMessageType () == IMessage::Type::ChatMessage ||
				(msg->GetMessageType () == IMessage::Type::MUCMessage &&
					Core::Instance ().IsHighlightMessage (msg)))
		{
			auto entry = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
			if (!Entry2Tab_.contains (entry->GetEntryID ()))
				OpenChat (entry, false);
		}
	}

	void ChatTabsManager::ChatMadeCurrent (ChatTab *curTab)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (curTab->GetCLEntry ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "chat's tab is not an ICLEntry";
			return;
		}

		if (IsActiveChat (entry))
			entry->MarkMsgsRead ();
	}

	void ChatTabsManager::EnqueueRestoreInfos (const QList<RestoreChatInfo>& infos)
	{
		for (const RestoreChatInfo& info : infos)
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

	void ChatTabsManager::RestoreChat (const RestoreChatInfo& info, QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);

		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "null entry for"
					<< entryObj;
			return;
		}
		auto tab = qobject_cast<ChatTab*> (OpenChat (entry, false, info.Props_));

		if (!info.Variant_.isEmpty ())
			tab->selectVariant (info.Variant_);

		tab->prepareMessageText (info.MsgText_);
	}

	void ChatTabsManager::CloseChatTab (ChatTab *tab, bool fromUser)
	{
		emit removeTab (tab);

		const auto& entry = Entry2Tab_.key (tab);
		Entry2Tab_.remove (entry);

		tab->deleteLater ();

		if (fromUser &&
				XmlSettingsManager::Instance ().property ("LeaveConfOnClose").toBool ())
		{
			const auto entryObj = tab->GetCLEntry ();
			if (const auto muc = qobject_cast<IMUCEntry*> (entryObj))
				muc->Leave ();
		}
	}

	void ChatTabsManager::handleNeedToClose (ChatTab *tab)
	{
		CloseChatTab (tab, true);
	}

	void ChatTabsManager::updateCurrentTab (QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		LastCurrentTab_ = entry ?
				Entry2Tab_.value (entry->GetEntryID ()) :
				0;
	}

	void ChatTabsManager::handleAddingCLEntryEnd (IHookProxy_ptr,
			QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto& id = entry->GetEntryID ();
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
		for (const auto& tab : Entry2Tab_)
			tab->PrepareTheme ();
	}
}
