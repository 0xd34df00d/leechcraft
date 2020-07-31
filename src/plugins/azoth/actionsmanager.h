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
#include <QMetaType>
#include <interfaces/core/ihookproxy.h>

class QAction;

namespace LC
{
namespace Azoth
{
	class ICLEntry;
	class IAuthable;
	class ServerHistoryWidget;
	class AvatarsManager;

	class ActionsManager : public QObject
	{
		Q_OBJECT

		AvatarsManager * const AvatarsManager_;

		typedef QHash<const ICLEntry*, QHash<QByteArray, QAction*>> Entry2Actions_t;
		Entry2Actions_t Entry2Actions_;

		struct ActionsVectors;
		std::shared_ptr<ActionsVectors> ActionsVectors_;
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
		typedef QHash<const QAction*, QList<CLEntryActionArea>> Action2Areas_t;
		Action2Areas_t Action2Areas_;
	public:
		ActionsManager (AvatarsManager*, QObject* = nullptr);

		QList<QAction*> GetEntryActions (ICLEntry *entry);
		QList<QAction*> CreateEntriesActions (QList<ICLEntry*> entries, QObject *parent);
		QList<CLEntryActionArea> GetAreasForAction (const QAction *action) const;

		void HandleEntryRemoved (ICLEntry*);
	private:
		void CreateActionsForEntry (ICLEntry*);
		void UpdateActionsForEntry (ICLEntry*);
	private slots:
		void handleActoredActionTriggered ();

		void handleActionNotifyChangesState ();
		void handleActionNotifyBecomesOnline ();
		void handleActionNotifyParticipantEnter ();
	signals:
		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRemoved (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entry);

		void gotServerHistoryTab (ServerHistoryWidget*);
	};
}
}

Q_DECLARE_METATYPE (LC::Azoth::ActionsManager::CLEntryActionArea)
