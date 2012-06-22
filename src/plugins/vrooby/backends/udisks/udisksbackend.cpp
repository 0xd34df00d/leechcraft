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

#include "udisksbackend.h"
#include <memory>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QtDebug>

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LeechCraft
{
namespace Vrooby
{
namespace UDisks
{
	Backend::Backend (QObject *parent)
	: DevBackend (parent)
	, Valid_ (false)
	, DevicesModel_ (new QStandardItemModel (this))
	, UDisksObj_ (0)
	{
		InitialEnumerate ();
	}

	bool Backend::IsValid () const
	{
		return Valid_;
	}

	QAbstractItemModel* Backend::GetDevicesModel () const
	{
		return DevicesModel_;
	}

	namespace
	{
		QDBusInterface_ptr GetDeviceInterface (const QString& path)
		{
			return QDBusInterface_ptr (new QDBusInterface ("org.freedesktop.UDisks",
						path,
						"org.freedesktop.UDisks.Device",
						QDBusConnection::systemBus ()));
		}
	}

	void Backend::Mount (const QString&)
	{
	}

	void Backend::Umount (const QString&)
	{
	}

	void Backend::InitialEnumerate ()
	{
		auto sb = QDBusConnection::systemBus ();
		auto iface = sb.interface ();
		const auto& services = iface->registeredServiceNames ()
				.value ().filter ("org.freedesktop.UDisks");
		if (services.isEmpty ())
			return;

		UDisksObj_ = new QDBusInterface ("org.freedesktop.UDisks", "/org/freedesktop/UDisks", "org.freedesktop.UDisks", sb);
		auto async = UDisksObj_->asyncCall ("EnumerateDevices");
		auto watcher = new QDBusPendingCallWatcher (async, this);
		connect (watcher,
				SIGNAL (finished (QDBusPendingCallWatcher*)),
				this,
				SLOT (handleEnumerationFinished (QDBusPendingCallWatcher*)));

		connect (UDisksObj_,
				SIGNAL (DeviceAdded (QDBusObjectPath)),
				this,
				SLOT (handleDeviceAdded (QDBusObjectPath)));
		connect (UDisksObj_,
				SIGNAL (DeviceRemoved (QDBusObjectPath)),
				this,
				SLOT (handleDeviceRemoved (QDBusObjectPath)));
		connect (UDisksObj_,
				SIGNAL (DeviceChanged (QDBusObjectPath)),
				this,
				SLOT (handleDeviceChanged (QDBusObjectPath)));
	}

	void Backend::AddPath (const QDBusObjectPath& path)
	{
		const auto& str = path.path ();
		if (Object2Item_.contains (str))
			return;

		auto iface = GetDeviceInterface (str);

		const auto& slaveTo = iface->property ("PartitionSlave").value<QDBusObjectPath> ();
		const bool isSlave = slaveTo.path () != "/";
		const bool isRemovable = iface->property ("DeviceIsRemovable").toBool ();
		qDebug () << str << slaveTo.path () << isSlave;
		if ((!isSlave && !isRemovable) || Unremovables_.contains (slaveTo.path ()))
		{
			Unremovables_ << str;
			return;
		}

		auto item = new QStandardItem;
		Object2Item_ [str] = item;
		SetItemData (iface, item);
		if (!isSlave)
			DevicesModel_->appendRow (item);
		else
		{
			if (!Object2Item_.contains (slaveTo.path ()))
				AddPath (slaveTo);
			Object2Item_ [slaveTo.path ()]->appendRow (item);
		}
	}

	void Backend::RemovePath (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		auto item = Object2Item_.take (path);
		if (!item)
			return;
		else if (item->parent ())
			item->parent ()->removeRow (item->row ());
		else
			DevicesModel_->removeRow (item->row ());
	}

	void Backend::SetItemData (QDBusInterface_ptr iface, QStandardItem *item)
	{
		if (!item)
			return;

		const bool isRemovable = iface->property ("DeviceIsRemovable").toBool ();
		const bool isPartition = iface->property ("DeviceIsPartition").toBool ();

		const QString& name = isPartition ?
			tr ("Partition %1")
				.arg (iface->property ("PartitionNumber").toInt ()) :
			(iface->property ("DriveVendor").toString () +
					" " +
					iface->property ("DriveModel").toString ());

		auto parentIface = iface;
		bool hasRemovableParent = isRemovable;
		while (!hasRemovableParent)
		{
			const auto& slaveTo = parentIface->property ("PartitionSlave").value<QDBusObjectPath> ();
			if (slaveTo.path () == "/")
				break;

			parentIface = GetDeviceInterface (slaveTo.path ());
			hasRemovableParent = parentIface->property ("DeviceIsRemovable").toBool ();
		}

		item->setData (DeviceType::GenericDevice, DeviceRoles::DevType);
		item->setData (iface->property ("DeviceFile").toString (), DeviceRoles::DevFile);
		item->setData (iface->property ("PartitionType").toInt (), DeviceRoles::PartType);
		item->setData (isRemovable, DeviceRoles::IsRemovable);
		item->setData (isPartition, DeviceRoles::IsPartition);
		item->setData (isPartition && hasRemovableParent, DeviceRoles::IsMountable);
		item->setData (iface->property ("DeviceIsMediaAvailable"), DeviceRoles::IsMediaAvailable);
		item->setData (iface->path (), DeviceRoles::DevID);
		item->setData (name, DeviceRoles::VisibleName);
		item->setData (iface->property ("PartitionSize").toLongLong (), DeviceRoles::TotalSize);
		item->setData (iface->property ("DeviceMountPaths").toStringList (), DeviceRoles::MountPoints);
	}

	void Backend::handleEnumerationFinished (QDBusPendingCallWatcher *watcher)
	{
		watcher->deleteLater ();
		QDBusPendingReply<QList<QDBusObjectPath>> reply = *watcher;
		if (reply.isError ())
		{
			Valid_ = false;
			qWarning () << reply.error ().message ();
			return;
		}

		for (const QDBusObjectPath& path : reply.value ())
			AddPath (path);
	}

	void Backend::handleDeviceAdded (const QDBusObjectPath& path)
	{
		AddPath (path);
	}

	void Backend::handleDeviceRemoved (const QDBusObjectPath& path)
	{
		RemovePath (path);
	}

	void Backend::handleDeviceChanged (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		SetItemData (GetDeviceInterface (path), Object2Item_ [path]);
	}
}
}
}
