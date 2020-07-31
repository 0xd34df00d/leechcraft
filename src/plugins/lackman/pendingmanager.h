/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <QMap>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LC
{
namespace LackMan
{
	class PendingManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel *PendingModel_;

		enum Action
		{
			Install,
			Remove,
			Update,
			MAX
		};
		QMap<Action, QSet<int>> ScheduledForAction_;
		QMap<Action, QStandardItem*> RootItemForAction_;

		QMap<int, QList<int>> Deps_;
		QMap<int, QStandardItem*> ID2ModelRow_;

		bool NotifyFetchListUpdateScheduled_;
	public:
		PendingManager (QObject* = 0);

		QAbstractItemModel* GetPendingModel () const;
		void Reset ();
		void ToggleInstallRemove (int id, bool enable, bool installed);
		void ToggleUpdate (int id, bool enable);

		QSet<int> GetPendingInstall () const;
		QSet<int> GetPendingRemove () const;
		QSet<int> GetPendingUpdate () const;

		void SuccessfullyInstalled (int);
		void SuccessfullyRemoved (int);
		void SuccessfullyUpdated (int);
	private:
		void EnablePackageInto (int, Action);
		void DisablePackageFrom (int, Action);
		void ReinitRootItems ();

		void NotifyHasPendingActionsChanged ();
		void ScheduleNotifyFetchListUpdate ();
	private slots:
		void notifyFetchListUpdate ();
	signals:
		void packageInstallRemoveToggled (int, bool);
		void packageUpdateToggled (int, bool);
		void fetchListUpdated (const QList<int>&);
		void hasPendingActionsChanged (bool);
	};
}
}
