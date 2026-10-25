/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>
#include <QSet>
#include <QMap>
#include <QStringList>
#include <util/sll/either.h>
#include <util/sll/void.h>

class QStandardItemModel;
class QStandardItem;
class QAbstractItemModel;

namespace LC
{
namespace LackMan
{
	struct UnsatisfiedDependencies
	{
		QString Package_;
		QStringList Dependencies_;
	};

	struct PackageLookupFailed
	{
		int PackageId_;
		QString What_;
	};

	using MarkingError_t = std::variant<UnsatisfiedDependencies, PackageLookupFailed>;
	using MarkingResult_t = Util::Either<MarkingError_t, Util::Void>;

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
		[[nodiscard]] MarkingResult_t ToggleInstallRemove (int id, bool enable, bool installed);
		[[nodiscard]] MarkingResult_t ToggleUpdate (int id, bool enable);

		QSet<int> GetPendingInstall () const;
		QSet<int> GetPendingRemove () const;
		QSet<int> GetPendingUpdate () const;

		void SuccessfullyInstalled (int);
		void SuccessfullyRemoved (int);
		void SuccessfullyUpdated (int);
	private:
		MarkingResult_t Toggle (int, bool enable, Action);
		MarkingResult_t EnablePackageInto (int, Action);
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
