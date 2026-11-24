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
#include "components/notifications/types.h"

namespace LC::AN
{
	struct FieldData;
}

class IEntityManager;

namespace LC
{
namespace Azoth
{
	class IMessage;
	class ICLEntry;
	class IMUCEntry;
	class IAccount;

	class AvatarsManager;
	class ChatTabsManager;
	class EntriesManager;

	struct EntryStatus;

	class NotificationsManager : public QObject
	{
		Q_OBJECT

		IEntityManager& EntityMgr_;
		AvatarsManager * const AvatarsMgr_;
		const EntriesManager& EntriesMgr_;
		const ChatTabsManager& TabsMgr_;
		QHash<ICLEntry*, int> UnreadCounts_;

		QHash<IAccount*, QDateTime> LastAccountStatusChange_;

		struct EntryEventNotification;
	public:
		static QList<AN::FieldData> GetANFields ();
		static QVariantMap SourceFields (const ICLEntry&);
		static QVariantMap RoomFields (const IMUCEntry&);

		NotificationsManager (AvatarsManager*, const EntriesManager&, const ChatTabsManager&, QObject* = nullptr);

		void AddAccount (QObject*);

		void AddCLEntry (QObject*);

		void CreateChangesStateRule (const ICLEntry&);
		void CreateBecomesOnlineRule (const ICLEntry&);
		void CreateParticipantEnterRule (const IMUCEntry&, const QString& nick);

		void HandleMessage (IMessage*);
		void Notify (ICLEntry& conversation, const Notifications::Typing& event);
	private:
		template<typename Event>
		void SendNotification (const Event&, const EntryEventNotification& n);

		void HandleStatusChanged (ICLEntry*, const EntryStatus&, const QString&);
	public slots:
		void handleClearUnreadMsgCount (QObject*);
	private slots:
		void handleTuneChanged (const QString&);
		void handleActivityChanged (const QString&);
		void handleMoodChanged (const QString&);
		void handleLocationChanged (const QString&);

		void handleAttentionDrawn (const QString&, const QString&);
	signals:
		void hookGotAuthRequest (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);
	};
}
}
