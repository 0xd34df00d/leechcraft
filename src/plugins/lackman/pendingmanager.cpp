/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingmanager.h"
#include <exception>
#include <memory>
#include <QStandardItemModel>
#include <QtDebug>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "deptreebuilder.h"
#include "core.h"

namespace LC
{
namespace LackMan
{
	namespace
	{
		std::unique_ptr<QStandardItem> MakeItem (const ListPackageInfo& info)
		{
			return std::make_unique<QStandardItem> (QString ("%1 (%2)").arg (info.Name_, info.ShortDescription_));
		}
	}

	PendingManager::PendingManager (QObject *parent)
	: QObject (parent)
	, PendingModel_ (new QStandardItemModel)
	, NotifyFetchListUpdateScheduled_ (false)
	{
	}

	QAbstractItemModel* PendingManager::GetPendingModel () const
	{
		return PendingModel_;
	}

	void PendingManager::Reset ()
	{
		ReinitRootItems ();

		for (int i = Action::Install; i < Action::MAX; ++i)
			ScheduledForAction_ [static_cast<Action> (i)].clear ();

		Deps_.clear ();
		ID2ModelRow_.clear ();

		fetchListUpdated ({});
		hasPendingActionsChanged (false);
	}

	ToggleResult_t PendingManager::ToggleInstallRemove (int id, bool enable, bool installed)
	{
		return Toggle (id, enable, installed ? Action::Remove : Action::Install);
	}

	ToggleResult_t PendingManager::ToggleUpdate (int id, bool enable)
	{
		return Toggle (id, enable, Action::Update);
	}

	QSet<int> PendingManager::GetPendingInstall () const
	{
		return ScheduledForAction_ [Action::Install];
	}

	QSet<int> PendingManager::GetPendingRemove () const
	{
		return ScheduledForAction_ [Action::Remove];
	}

	QSet<int> PendingManager::GetPendingUpdate () const
	{
		return ScheduledForAction_ [Action::Update];
	}

	void PendingManager::SuccessfullyInstalled (int packageId)
	{
		DisablePackageFrom (packageId, Action::Install);
	}

	void PendingManager::SuccessfullyRemoved (int packageId)
	{
		DisablePackageFrom (packageId, Action::Remove);
	}

	void PendingManager::SuccessfullyUpdated (int packageId)
	{
		DisablePackageFrom (packageId, Action::Update);
	}

	ToggleResult_t PendingManager::Toggle (int id, bool enable, Action action)
	{
		if (enable)
			return EnablePackageInto (id, action);

		DisablePackageFrom (id, action);
		return Util::Void {};
	}

	ToggleResult_t PendingManager::EnablePackageInto (int id, Action action)
	{
		std::unique_ptr<QStandardItem> packageItem;
		QList<int> deps;
		try
		{
			const DepTreeBuilder builder { id };
			const auto& info = Core::Instance ().GetListPackageInfo (id);
			if (!builder.IsFulfilled ())
				return Util::Left { UnsatisfiedDependencies { info.Name_, builder.GetUnfulfilled () } };

			deps = builder.GetPackagesToInstall ();
			packageItem = MakeItem (info);
			packageItem->setIcon (Core::Instance ().GetIconForLPI (info));
			for (const auto dep : deps)
				packageItem->appendRow (MakeItem (Core::Instance ().GetListPackageInfo (dep)).release ());
		}
		catch (const std::exception& e)
		{
			return Util::Left { ResolutionFailed { id, QString::fromUtf8 (e.what ()) } };
		}

		Deps_ [id] = deps;
		ScheduledForAction_ [action] << id;
		ID2ModelRow_ [id] = packageItem.get ();
		RootItemForAction_ [action]->appendRow (packageItem.release ());

		NotifyHasPendingActionsChanged ();
		if (action != Action::Remove)
			ScheduleNotifyFetchListUpdate ();
		return Util::Void {};
	}

	void PendingManager::DisablePackageFrom (int id, PendingManager::Action action)
	{
		Deps_.remove (id);
		ScheduledForAction_ [action].remove (id);

		if (!ID2ModelRow_.contains (id))
		{
			qWarning () << Q_FUNC_INFO
					<< "strange, seems like"
					<< id
					<< "hasn't been added to the model";
			return;
		}

		const auto item = ID2ModelRow_.take (id);
		item->parent ()->removeRow (item->row ());

		NotifyHasPendingActionsChanged ();
		if (action != Action::Remove)
			ScheduleNotifyFetchListUpdate ();
	}

	void PendingManager::ReinitRootItems ()
	{
		PendingModel_->clear ();
		RootItemForAction_.clear ();

		const auto mgr = Core::Instance ().GetProxy ()->GetIconThemeManager ();

		RootItemForAction_ [Action::Install] =
				new QStandardItem (mgr->GetIcon ("list-add"),
						tr ("To be installed"));
		RootItemForAction_ [Action::Remove] =
				new QStandardItem (mgr->GetIcon ("list-remove"),
						tr ("To be removed"));
		RootItemForAction_ [Action::Update] =
				new QStandardItem (mgr->GetIcon ("system-software-update"),
						tr ("To be updated"));

		for (int i = Action::Install; i < Action::MAX; ++i)
		{
			QStandardItem *item = RootItemForAction_ [static_cast<Action> (i)];
			item->setEditable (false);
			PendingModel_->appendRow (item);
		}
	}

	void PendingManager::NotifyHasPendingActionsChanged ()
	{
		const bool hasPending = std::any_of (ScheduledForAction_.begin (),
				ScheduledForAction_.end (),
				[] (const QSet<int>& set) { return !set.isEmpty (); });
		emit hasPendingActionsChanged (hasPending);
	}

	void PendingManager::ScheduleNotifyFetchListUpdate ()
	{
		if (NotifyFetchListUpdateScheduled_)
			return;

		NotifyFetchListUpdateScheduled_ = true;
		QTimer::singleShot (0,
				this,
				SLOT (notifyFetchListUpdate ()));
	}

	void PendingManager::notifyFetchListUpdate ()
	{
		NotifyFetchListUpdateScheduled_ = false;

		auto ids = (ScheduledForAction_ [Action::Install] +
					ScheduledForAction_ [Action::Update]).values ();
		for (const int id : ids)
			ids << Deps_ [id];
		emit fetchListUpdated (ids);
	}
}
}
