/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include "components/chat/chattab.h"

class QWidget;
class QWebEngineProfile;

namespace LC
{
namespace Util
{
	class WkFontsWidget;
}

namespace Azoth
{
	class ICLEntry;
	class EntriesManager;
	class AvatarsManager;

	class ChatTabsManager : public QObject
	{
		Q_OBJECT

		AvatarsManager * const AvatarsManager_;
		Util::WkFontsWidget * const FontsWidget_;
		QWebEngineProfile * const Profile_;

		QSet<QString> StyleParams_;
		QHash<GlobalStrongestId, ChatTab_ptr> Entry2Tab_;
		QSet<GlobalStrongestId> EverOpened_;

		QPointer<ChatTab> LastCurrentTab_;
	public:
		struct RestoreChatInfo
		{
			GlobalStrongestId EntryID_;
			QString Variant_ {};
			QString MsgText_ {};
			DynPropertiesList_t Props_;
		};
	private:
		QHash<GlobalStrongestId, RestoreChatInfo> RestoreInfo_;
	public:
		ChatTabsManager (AvatarsManager*, Util::WkFontsWidget*, EntriesManager&, QObject* = nullptr);

		QWidget* OpenChat (const QModelIndex&);
		ChatTab* OpenChat (const ICLEntry&, bool fromUser, const DynPropertiesList_t& = {});

		void CloseChat (const ICLEntry&, bool fromUser);

		bool IsActiveChat (const ICLEntry*) const;
		bool IsOpenedChat (const GlobalStrongestId&) const;
		ChatTab* GetActiveChatTab () const;
		ChatTab* GetChatTab (const GlobalStrongestId& entryId) const;

		void UpdateEntryMapping (const GlobalStrongestId&);

		void HandleInMessage (IMessage*);

		void ChatMadeCurrent (ChatTab*);

		void EnqueueRestoreInfos (const QList<RestoreChatInfo>&);

		std::optional<QString> GetActiveVariant (ICLEntry*) const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		void RestoreChat (const RestoreChatInfo&, ICLEntry&);
		void CloseChatTab (ChatTab*, bool fromUser);
		void MergeTabs (ChatTab& from, ChatTab& to);
	private slots:
		void updateCurrentTab (QObject*);
		void handleAddingCLEntryEnd (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void chatWindowStyleChanged ();
	signals:
		void entryMadeCurrent (QObject*);
		void entryLostCurrent (QObject*);
	};
}
}
