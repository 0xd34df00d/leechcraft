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

#ifndef PLUGINS_AZOTH_ACTIONSMANAGER_H
#define PLUGINS_AZOTH_ACTIONSMANAGER_H
#include <functional>
#include <QObject>
#include <QHash>
#include <QMetaType>
#include <interfaces/core/ihookproxy.h>

class QAction;

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;
	class IAuthable;

	class ActionsManager : public QObject
	{
		Q_OBJECT

		typedef QHash<const ICLEntry*, QHash<QByteArray, QAction*> > Entry2Actions_t;
		Entry2Actions_t Entry2Actions_;
	public:
		enum CLEntryActionArea
		{
			CLEAATabCtxtMenu,
			CLEAAContactListCtxtMenu,
			CLEAAApplicationMenu,
			CLEAAToolbar,
			CLEAAChatCtxtMenu,
			CLEAAMAX
		};
	private:
		typedef QHash<const QAction*, QList<CLEntryActionArea> > Action2Areas_t;
		Action2Areas_t Action2Areas_;
	public:
		ActionsManager (QObject* = 0);

		QList<QAction*> GetEntryActions (ICLEntry *entry);
		QList<CLEntryActionArea> GetAreasForAction (const QAction *action) const;

		void HandleEntryRemoved (ICLEntry*);
	private:
		QString GetReason (const QString& id, const QString& text);
		void ManipulateAuth (const QString& id, const QString& text,
				std::function<void (IAuthable*, const QString&)> func);
		void CreateActionsForEntry (ICLEntry*);
		void UpdateActionsForEntry (ICLEntry*);
	private slots:
		void handleActionOpenChatTriggered ();
		void handleActionDrawAttention ();
		void handleActionRenameTriggered ();
		void handleActionChangeGroupsTriggered ();
		void handleActionRemoveTriggered ();
		void handleActionGrantAuthTriggered ();
		void handleActionRevokeAuthTriggered ();
		void handleActionUnsubscribeTriggered ();
		void handleActionRerequestTriggered ();
#ifdef ENABLE_CRYPT
		void handleActionManagePGPTriggered ();
#endif
		void handleActionShareContactsTriggered ();
		void handleActionVCardTriggered ();
		void handleActionInviteTriggered ();
		void handleActionLeaveTriggered ();
		void handleActionAuthorizeTriggered ();
		void handleActionDenyAuthTriggered ();
		void handleActionAddContactFromMUC ();
		void handleActionCopyMUCPartID ();
		void handleActionPermTriggered ();
	signals:
		void hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
	};
}
}

Q_DECLARE_METATYPE (LeechCraft::Azoth::ActionsManager::CLEntryActionArea);

#endif
