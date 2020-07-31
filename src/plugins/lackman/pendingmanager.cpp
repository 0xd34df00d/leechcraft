/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingmanager.h"
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

		for (const auto id : ScheduledForAction_.value (Action::Update))
			packageUpdateToggled (id, false);

		for (const auto id : ScheduledForAction_.value (Action::Install))
			packageInstallRemoveToggled (id, false);
		for (const auto id : ScheduledForAction_.value (Action::Remove))
			packageInstallRemoveToggled (id, false);

		for (int i = Action::Install; i < Action::MAX; ++i)
			ScheduledForAction_ [static_cast<Action> (i)].clear ();

		Deps_.clear ();
		ID2ModelRow_.clear ();

		fetchListUpdated ({});
		hasPendingActionsChanged (false);
	}

	void PendingManager::ToggleInstallRemove (int id, bool enable, bool installed)
	{
		if (enable)
			EnablePackageInto (id, installed ? Action::Remove : Action::Install);
		else
			DisablePackageFrom (id, installed ? Action::Remove : Action::Install);

		emit packageInstallRemoveToggled (id, enable);
	}

	void PendingManager::ToggleUpdate (int id, bool enable)
	{
		if (enable)
			EnablePackageInto (id, Action::Update);
		else
			DisablePackageFrom (id, Action::Update);

		emit packageUpdateToggled (id, enable);
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

	void PendingManager::EnablePackageInto (int id, PendingManager::Action action)
	{
		DepTreeBuilder builder (id);
		if (!builder.IsFulfilled ())
		{
			QStringList unful = builder.GetUnfulfilled ();
			qWarning () << Q_FUNC_INFO
					<< id
					<< "isn't fulfilled, aborting:"
					<< unful;
			QString list = QString ("<ul><li>%1</li></ul>")
					.arg (unful.join ("</li><li>"));
			throw std::runtime_error (tr ("Package dependencies "
					"could not be fulfilled: %1").arg (list)
					.toUtf8 ().constData ());
		}

		const auto& deps = builder.GetPackagesToInstall ();
		Deps_ [id] = deps;

		ScheduledForAction_ [action] << id;

		ListPackageInfo info = Core::Instance ().GetListPackageInfo (id);
		QStandardItem *packageItem = new QStandardItem (QString ("%1 (%2)")
				.arg (info.Name_)
				.arg (info.ShortDescription_));
		packageItem->setIcon (Core::Instance ().GetIconForLPI (info));
		for (int dep : deps)
		{
			info = Core::Instance ().GetListPackageInfo (dep);
			QStandardItem *item = new QStandardItem (QString ("%1 (%2)")
					.arg (info.Name_)
					.arg (info.ShortDescription_));
			packageItem->appendRow (item);
		}

		ID2ModelRow_ [id] = packageItem;
		RootItemForAction_ [action]->appendRow (packageItem);

		NotifyHasPendingActionsChanged ();
		if (action != Action::Remove)
			ScheduleNotifyFetchListUpdate ();
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
