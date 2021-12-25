/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unmountabledevmanager.h"
#include <QStandardItemModel>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "../interfaces/lmp/iunmountablesync.h"

namespace LC
{
namespace LMP
{
	UnmountableDevManager::UnmountableDevManager (QObject *parent)
	: QObject (parent)
	, DevListModel_ (new QStandardItemModel (this))
	{
		DevListModel_->setColumnCount (1);
	}

	QAbstractItemModel* UnmountableDevManager::GetDevListModel () const
	{
		return DevListModel_;
	}

	void UnmountableDevManager::InitializePlugins ()
	{
		auto pm = GetProxyHolder ()->GetPluginsManager ();
		Managers_ = pm->GetAllCastableRoots<IUnmountableSync*> ();
		for (auto mgr : Managers_)
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

	void UnmountableDevManager::Refresh ()
	{
		for (auto mgrObj : Managers_)
			qobject_cast<IUnmountableSync*> (mgrObj)->Refresh ();
	}

	void UnmountableDevManager::rebuildAvailableDevices ()
	{
		auto items = Items_;

		for (auto mgrObj : Managers_)
		{
			auto& mgrDevices = items [mgrObj];

			auto mgr = qobject_cast<IUnmountableSync*> (mgrObj);
			for (const auto& device : mgr->AvailableDevices ())
			{
				auto name = device.Name_;
				if (device.BatteryCharge_ >= 0)
					name += " (" + tr ("%1% charged").arg (device.BatteryCharge_) + ")";

				if (const auto item = mgrDevices.take (device.ID_))
				{
					item->setText (name);
					continue;
				}

				auto item = new QStandardItem { name };
				item->setData (QVariant::fromValue (mgrObj), Roles::ManagerObj);
				item->setData (device.ID_, CommonDevRole::DevPersistentID);
				item->setData (QVariant::fromValue (device), Roles::DeviceInfo);
				DevListModel_->appendRow (item);

				Items_ [mgrObj] [device.ID_] = item;
			}
		}

		for (const auto& mgrPair : Util::Stlize (items))
			for (const auto& pair : Util::Stlize (mgrPair.second))
			{
				Items_ [mgrPair.first].remove (pair.first);
				DevListModel_->removeRow (pair.second->row ());
			}
	}
}
}
