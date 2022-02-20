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
#include <QPersistentModelIndex>
#include "chattab.h"

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
	class AvatarsManager;

	class ChatTabsManager : public QObject
	{
		Q_OBJECT

		AvatarsManager * const AvatarsManager_;
		Util::WkFontsWidget * const FontsWidget_;
		QWebEngineProfile * const Profile_;

		QSet<QString> StyleParams_;
		QHash<QString, ChatTab_ptr> Entry2Tab_;
		QSet<QString> EverOpened_;

		QPointer<ChatTab> LastCurrentTab_;
	public:
		struct RestoreChatInfo
		{
			QString EntryID_;
			QString Variant_;
			QString MsgText_;
			DynPropertiesList_t Props_;
		};
	private:
		QHash<QString, RestoreChatInfo> RestoreInfo_;
	public:
		ChatTabsManager (AvatarsManager*, Util::WkFontsWidget*, QObject* = nullptr);

		QWidget* OpenChat (const QModelIndex&);
		ChatTab* OpenChat (const ICLEntry*, bool fromUser,
				const DynPropertiesList_t& = DynPropertiesList_t ());

		void CloseChat (const ICLEntry*, bool fromUser);

		bool IsActiveChat (const ICLEntry*) const;
		bool IsOpenedChat (const QString&) const;
		ChatTab* GetActiveChatTab () const;
		ChatTab* GetChatTab (const QString& entryId) const;

		void UpdateEntryMapping (const QString&);

		void HandleEntryAdded (ICLEntry*);
		void HandleEntryRemoved (ICLEntry*);
		void HandleInMessage (IMessage*);

		void ChatMadeCurrent (ChatTab*);

		void EnqueueRestoreInfos (const QList<RestoreChatInfo>&);

		QString GetActiveVariant (ICLEntry*) const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private:
		void UpdateMUCTab (ICLEntry*);
		void RestoreChat (const RestoreChatInfo&, QObject*);
		void CloseChatTab (ChatTab*, bool fromUser);
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
