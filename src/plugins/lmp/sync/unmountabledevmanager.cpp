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

#include "unmountabledevmanager.h"
#include <QStandardItemModel>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "../interfaces/lmp/iunmountablesync.h"
#include "../core.h"

namespace LeechCraft
{
namespace LMP
{
	UnmountableDevManager::UnmountableDevManager (QObject *parent)
	: QObject (parent)
	, DevListModel_ (new QStandardItemModel (this))
	{
	}

	QAbstractItemModel* UnmountableDevManager::GetDevListModel () const
	{
		return DevListModel_;
	}

	void UnmountableDevManager::InitializePlugins ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		Managers_ = pm->GetAllCastableRoots<IUnmountableSync*> ();
		Q_FOREACH (auto mgr, Managers_)
			connect (mgr,
					SIGNAL (availableDevicesChanged ()),
					this,
					SLOT (rebuildAvailableDevices ()));

		rebuildAvailableDevices ();
	}

	QObject* UnmountableDevManager::GetDeviceManager (int row) const
	{
		auto item = DevListModel_->item (row);
		if (!item)
			return 0;

		return item->data (Roles::ManagerObj).value<QObject*> ();
	}

	UnmountableDevInfo UnmountableDevManager::GetDeviceInfo (int row) const
	{
		auto item = DevListModel_->item (row);
		if (!item)
			return UnmountableDevInfo ();

		return item->data (Roles::DeviceInfo).value<UnmountableDevInfo> ();
	}

	void UnmountableDevManager::rebuildAvailableDevices ()
	{
		DevListModel_->clear ();

		Q_FOREACH (auto mgrObj, Managers_)
		{
			auto mgr = qobject_cast<IUnmountableSync*> (mgrObj);
			for (const auto& device : mgr->AvailableDevices ())
			{
				auto item = new QStandardItem (device.Name_);
				item->setData (QVariant::fromValue (mgrObj), Roles::ManagerObj);
				item->setData (device.ID_, Roles::DeviceID);
				item->setData (QVariant::fromValue (device), Roles::DeviceInfo);
				DevListModel_->appendRow (item);
			}
		}
	}
}
}
