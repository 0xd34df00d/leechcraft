/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "pendingmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "deptreebuilder.h"
#include "core.h"

namespace LeechCraft
{
namespace LackMan
{
	PendingManager::PendingManager (QObject *parent)
	: QObject (parent)
	, PendingModel_ (new QStandardItemModel)
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
	}

	void PendingManager::ToggleInstallRemove (int id, bool enable, bool installed)
	{
		if (enable)
			EnablePackageInto (id, installed ? Action::Remove : Action::Install);
		else
			DisablePackageFrom (id, installed ? Action::Remove : Action::Install);
	}

	void PendingManager::ToggleUpdate (int id, bool enable)
	{
		if (ScheduledForAction_ [Action::Update].contains (id))
			return;

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
		Q_FOREACH (int dep, deps)
		{
			info = Core::Instance ().GetListPackageInfo (dep);
			QStandardItem *item = new QStandardItem (QString ("%1 (%2)")
					.arg (info.Name_)
					.arg (info.ShortDescription_));
			packageItem->appendRow (item);
		}

		ID2ModelRow_ [id] = packageItem;
		RootItemForAction_ [action]->appendRow (packageItem);

		if (action != Action::Remove)
			NotifyFetchListUpdate ();
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

		QStandardItem *item = ID2ModelRow_.take (id);
		item->parent ()->takeRow (item->row ());
		delete item;

		if (action != Action::Remove)
			NotifyFetchListUpdate ();
	}

	void PendingManager::ReinitRootItems ()
	{
		PendingModel_->clear ();
		RootItemForAction_.clear ();

		RootItemForAction_ [Action::Install] =
				new QStandardItem (Core::Instance ().GetProxy ()->GetIcon ("list-add"),
						tr ("To be installed"));
		RootItemForAction_ [Action::Remove] =
				new QStandardItem (Core::Instance ().GetProxy ()->GetIcon ("list-remove"),
						tr ("To be removed"));
		RootItemForAction_ [Action::Update] =
				new QStandardItem (Core::Instance ().GetProxy ()->GetIcon ("system-software-update"),
						tr ("To be updated"));

		for (int i = Action::Install; i < Action::MAX; ++i)
		{
			QStandardItem *item = RootItemForAction_ [static_cast<Action> (i)];
			item->setEditable (false);
			PendingModel_->appendRow (item);
		}
	}

	void PendingManager::NotifyFetchListUpdate ()
	{
		auto ids = (ScheduledForAction_ [Action::Install] +
					ScheduledForAction_ [Action::Update]).toList ();
		Q_FOREACH (const int id, ids)
			ids << Deps_ [id];
		emit fetchListUpdated (ids);
	}
}
}
