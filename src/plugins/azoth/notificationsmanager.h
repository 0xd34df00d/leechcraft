/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantMap>
#include <interfaces/core/ihookproxy.h>
#include "interfaces/azoth/azothcommon.h"

class IEntityManager;

namespace LC
{
namespace Azoth
{
	class IMessage;
	class ICLEntry;
	class IAccount;
	class AvatarsManager;
	struct EntryStatus;

	class NotificationsManager : public QObject
	{
		Q_OBJECT

		IEntityManager * const EntityMgr_;
		AvatarsManager * const AvatarsMgr_;
		QHash<ICLEntry*, int> UnreadCounts_;

		QHash<IAccount*, QDateTime> LastAccountStatusChange_;

		QHash<QString, bool> ShouldNotifyNextTyping_;
	public:
		NotificationsManager (IEntityManager*, AvatarsManager*, QObject* = nullptr);

		void AddAccount (QObject*);
		void RemoveAccount (QObject*);

		void AddCLEntry (QObject*);
		void RemoveCLEntry (QObject*);

		void HandleMessage (IMessage*);
	private:
		void NotifyWithReason (QObject*, const QString&,
				const char*, const QString&,
				const QString&, const QString&);
		void HandleStatusChanged (ICLEntry*, const EntryStatus&, const QString&);
	public slots:
		void handleClearUnreadMsgCount (QObject*);
	private slots:
		void handleItemSubscribed (QObject*, const QString&);
		void handleItemUnsubscribed (QObject*, const QString&);
		void handleItemUnsubscribed (const QString&, const QString&);
		void handleItemCancelledSubscription (QObject*, const QString&);
		void handleItemGrantedSubscription (QObject*, const QString&);

		void handleAccountStatusChanged (const EntryStatus&);
		void handleStatusChanged (const EntryStatus&, const QString&);

		void handleTuneChanged (const QString&);
		void handleActivityChanged (const QString&);
		void handleMoodChanged (const QString&);
		void handleLocationChanged (const QString&);

		void handleAttentionDrawn (const QString&, const QString&);
		void handleAuthorizationRequested (QObject*, const QString&);

		void handleMUCInvitation (const QVariantMap&, const QString&, const QString&);

		void handleChatPartStateChanged (ChatPartState, const QString&);

		void handleEntryMadeCurrent (QObject*);
	signals:
		void hookGotAuthRequest (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);
	};
}
}
