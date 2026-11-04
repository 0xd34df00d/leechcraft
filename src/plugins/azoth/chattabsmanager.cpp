/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattabsmanager.h"
#include <QTextDocument>
#include <QWebEngineProfile>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/sll/qobjectrefcast.h>
#include <util/xpc/sessionprops.h>
#include "util/azoth/util.h"
#include "interfaces/azoth/iclentry.h"
#include "components/managers/entriesmanager.h"
#include "core.h"
#include "roles.h"
#include "xmlsettingsmanager.h"
#include "avatarsmanager.h"

namespace LC::Azoth
{
	namespace
	{
		bool ShouldOpenMucJoin (ICLEntry& entry)
		{
			if (entry.GetEntryType () != ICLEntry::EntryType::MUC)
				return false;

			if (!qobject_ref_cast<IMUCEntry> (entry.GetQObject ()).IsAutojoined ())
				return true;

			return XmlSettingsManager::Instance ().property ("OpenTabsForAutojoin").toBool ();
		}
	}

	ChatTabsManager::ChatTabsManager (AvatarsManager *am, Util::WkFontsWidget *fontsWidget, const EntriesManager& entries, QObject *parent)
	: QObject { parent }
	, AvatarsManager_ { am }
	, FontsWidget_ { fontsWidget }
	, Profile_ { new QWebEngineProfile { this } }
	, Entries_ { entries }
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

		connect (&entries,
				&EntriesManager::conventionalIdChanged,
				this,
				[this] (const GlobalConventionalId& oldId, const GlobalConventionalId& newId)
				{
					if (const auto liveTab = Entry2Tab_.take (oldId))
					{
						liveTab->UpdateEntryID (newId);
						// the target of the rename is not in the roster, hence its tab is stale
						if (const auto staleTab = std::exchange (Entry2Tab_ [newId], liveTab))
						{
							staleTab->UpdateEntryID ({});
							MergeTabs (*staleTab, *liveTab);
						}
					}

					if (EverOpened_.remove (oldId))
						EverOpened_ << newId;
				});

		connect (&entries,
				&EntriesManager::added,
				this,
				[this] (const QList<ICLEntry*>& entries)
				{
					for (const auto entry : entries)
					{
						const auto& id = entry->GetGlobalStrongestID ();
						if (const auto tab = Entry2Tab_.value (id))
							tab->ReinitEntry ();

						if (RestoreInfo_.contains (id) || ShouldOpenMucJoin (*entry))
							OpenChat (*entry, false);
					}
				});

		connect (&entries,
				&EntriesManager::aboutToBeRemoved,
				this,
				[this] (const QList<ICLEntry*>& entries)
				{
					if (XmlSettingsManager::Instance ().property ("CloseConfOnLeave").toBool ())
						for (const auto entry : entries)
							if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
								CloseChat (*entry, false);
				});
	}

	QWidget* ChatTabsManager::OpenChat (const QModelIndex& ti)
	{
		const auto entryObj = ti.data (CLREntryObject).value<QObject*> ();
		return OpenChat (qobject_ref_cast<ICLEntry> (entryObj), true);
	}

	ChatTab* ChatTabsManager::OpenChat (const ICLEntry& entry, bool fromUser)
	{
		const auto& id = entry.GetGlobalStrongestID ();

		std::move_only_function<void (ChatTab&)> restorer = [] (ChatTab&) {};
		if (const auto pos = RestoreInfo_.find (id);
			pos != RestoreInfo_.end ())
		{
			restorer = [info = std::move (*pos)] (ChatTab& tab)
			{
				Util::SetSessionProps (tab, info.Props_);
				if (!info.Variant_.isEmpty ())
					tab.selectVariant (info.Variant_);
				tab.prepareMessageText (info.MsgText_);
			};
			RestoreInfo_.erase (pos);
		}

		if (Entry2Tab_.contains (id))
		{
			auto tab = Entry2Tab_ [id];
			restorer (*tab);
			emit tab->raiseTab ();
			return tab;
		}

		EverOpened_ << id;

		QPointer tab (new ChatTab (entry, entry.GetParentAccount (), AvatarsManager_, Entries_, FontsWidget_, Profile_));
		restorer (*tab);
		tab->installEventFilter (this);
		Entry2Tab_ [id] = tab;

		connect (tab,
				&ChatTab::needToClose,
				this,
				[this, tab] { CloseChatTab (tab, true); });
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

		IRootWindowsManager::AddTabFlags flags {};
		if (!fromUser && !XmlSettingsManager::Instance ().property ("JumpToNewTabOnOpen").toBool ())
			flags |= IRootWindowsManager::AddTabFlag::Background;
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tab->ReformatTitle (), tab, flags);

		tab->HasBeenAdded ();

		return tab;
	}

	void ChatTabsManager::CloseChat (const ICLEntry& entry, bool fromUser)
	{
		if (const auto& id = entry.GetGlobalStrongestID ();
			const auto tab = Entry2Tab_.value (id))
			CloseChatTab (tab, fromUser);
	}

	bool ChatTabsManager::IsActiveChat (const ICLEntry *entry) const
	{
		const auto tab = Entry2Tab_.value (entry->GetGlobalStrongestID ());
		if (!tab)
			return false;

		return tab->isVisible () && tab->isActiveWindow ();
	}

	bool ChatTabsManager::IsOpenedChat (const GlobalStrongestId& id) const
	{
		return EverOpened_.contains (id);
	}

	ChatTab* ChatTabsManager::GetActiveChatTab () const
	{
		return LastCurrentTab_;
	}

	ChatTab* ChatTabsManager::GetChatTab (const GlobalStrongestId& entryId) const
	{
		return Entry2Tab_ [entryId];
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
			if (!Entry2Tab_.contains (entry->GetGlobalStrongestID ()))
				OpenChat (*entry, false);
		}
	}

	void ChatTabsManager::ChatMadeCurrent (ChatTab *curTab)
	{
		const auto entry = curTab->GetEntry<ICLEntry> ();
		if (!entry)
		{
			qWarning () << "chat's tab is not an ICLEntry";
			return;
		}

		if (IsActiveChat (entry))
			entry->MarkMsgsRead ();
	}

	void ChatTabsManager::EnqueueRestoreInfos (const QList<RestoreChatInfo>& infos)
	{
		for (const RestoreChatInfo& info : infos)
		{
			const auto entryObj = Entries_.GetEntryOrNull (info.EntryID_);
			RestoreInfo_.emplace (info.EntryID_, info);
			if (entryObj)
				OpenChat (*entryObj, false);
		}
	}

	std::optional<QString> ChatTabsManager::GetActiveVariant (ICLEntry *entry) const
	{
		const auto tab = Entry2Tab_ [entry->GetGlobalStrongestID ()];
		if (!tab)
			return {};

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

	void ChatTabsManager::CloseChatTab (ChatTab *tab, bool fromUser)
	{
		emit tab->removeTab ();

		Entry2Tab_.removeIf ([tab] (const auto& it) { return it.value () == tab; });

		tab->deleteLater ();

		if (fromUser &&
				XmlSettingsManager::Instance ().property ("LeaveConfOnClose").toBool ())
		{
			if (const auto muc = tab->GetEntry<IMUCEntry> ())
				muc->Leave ();
		}
	}

	void ChatTabsManager::MergeTabs (ChatTab& from, ChatTab& to)
	{
		const auto fromDoc = from.getMsgEdit ()->document ();
		const auto toEdit = to.getMsgEdit ();
		if (!fromDoc->isEmpty () && toEdit->document ()->isEmpty ())
			toEdit->setHtml (fromDoc->toHtml ());

		to.ResyncLogs ();

		if (from.isActiveWindow () && from.isVisible ())
			emit to.raiseTab ();
		CloseChatTab (&from, false);
	}

	void ChatTabsManager::updateCurrentTab (QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		LastCurrentTab_ = entry ? Entry2Tab_.value (entry->GetGlobalStrongestID ()) : nullptr;
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
