/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "udisksbackend.h"
#include <memory>
#include <QStorageInfo>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QTimer>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/devices/deviceroles.h>

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LC
{
namespace Vrooby
{
namespace UDisks
{
	Backend::Backend (const ICoreProxy_ptr& proxy)
	: DevBackend (proxy)
	, DevicesModel_ (new QStandardItemModel (this))
	{
	}

	QString Backend::GetBackendName () const
	{
		return "UDisks";
	}

	bool Backend::IsAvailable ()
	{
		const auto iface = QDBusConnection::systemBus ().interface ();

		const auto interfaceName = "org.freedesktop.UDisks"_qs;
		if (iface->registeredServiceNames ().value ().contains (interfaceName))
			return true;

		iface->startService (interfaceName);
		return iface->registeredServiceNames ().value ().contains (interfaceName);
	}

	void Backend::Start ()
	{
		QTimer::singleShot (1000,
				this,
				SLOT (startInitialEnumerate ()));

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (updateDeviceSpaces ()));
		timer->start (10000);
	}

	bool Backend::SupportsDevType (DeviceType type) const
	{
		return type == DeviceType::MassStorage;
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

	void Backend::MountDevice (const QString& id)
	{
		auto iface = GetDeviceInterface (id);
		if (!iface)
			return;

		if (!iface->property ("DeviceIsMounted").toBool ())
		{
			auto async = iface->asyncCall ("FilesystemMount", QString (), QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (mountCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	void Backend::InitialEnumerate ()
	{
		auto sb = QDBusConnection::systemBus ();

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

	bool Backend::AddPath (const QDBusObjectPath& path)
	{
		const auto& str = path.path ();
		if (Object2Item_.contains (str))
			return true;

		auto iface = GetDeviceInterface (str);

		const auto& slaveTo = iface->property ("PartitionSlave").value<QDBusObjectPath> ();
		const bool isSlave = slaveTo.path () != "/";
		const bool isRemovable = iface->property ("DeviceIsRemovable").toBool () ||
				iface->property ("DriveCanDetach").toBool ();
		qDebug () << str << slaveTo.path () << isSlave << isRemovable;
		if ((!isSlave && !isRemovable) || Unremovables_.contains (slaveTo.path ()))
		{
			Unremovables_ << str;
			return false;
		}

		auto item = new QStandardItem;
		Object2Item_ [str] = item;
		SetItemData (iface, item);
		if (!isSlave)
			DevicesModel_->appendRow (item);
		else
		{
			if (!Object2Item_.contains (slaveTo.path ()))
				if (!AddPath (slaveTo))
					return false;
			Object2Item_ [slaveTo.path ()]->appendRow (item);
		}
		return true;
	}

	void Backend::RemovePath (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		auto item = Object2Item_.take (path);
		if (!item)
			return;

		auto getChildren = [] (QStandardItem *item)
		{
			QList<QStandardItem*> result;
			for (int i = 0; i < item->rowCount (); ++i)
				result << item->child (i);
			return result;
		};

		QList<QStandardItem*> toRemove = getChildren (item);
		for (int i = 0; i < toRemove.size (); ++i)
			toRemove += getChildren (toRemove [i]);

		for (QStandardItem *item : toRemove)
			Object2Item_.remove (Object2Item_.key (item));

		if (item->parent ())
			item->parent ()->removeRow (item->row ());
		else
			DevicesModel_->removeRow (item->row ());
	}

	void Backend::SetItemData (QDBusInterface_ptr iface, QStandardItem *item)
	{
		if (!item)
			return;

		const bool isRemovable = iface->property ("DeviceIsRemovable").toBool () ||
				iface->property ("DriveCanDetach").toBool ();
		const bool isPartition = iface->property ("DeviceIsPartition").toBool ();

		const auto& vendor = iface->property ("DriveVendor").toString () +
				" " +
				iface->property ("DriveModel").toString ();
		const auto& partLabel = iface->property ("PartitionLabel").toString ().trimmed ();
		const auto& partName = partLabel.isEmpty () ?
				tr ("Partition %1")
						.arg (iface->property ("PartitionNumber").toInt ()) :
				partLabel;
		const auto& name = isPartition ? partName : vendor;
		const auto& fullName = isPartition ?
				QString ("%1: %2").arg (vendor, partName) :
				vendor;

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

		DevicesModel_->blockSignals (true);

		const auto& mountPaths = iface->property ("DeviceMountPaths").toStringList ();
		if (!mountPaths.isEmpty ())
			item->setData (QStorageInfo { mountPaths.value (0) }.bytesAvailable (), MassStorageRole::AvailableSize);
		else
			item->setData (-1, MassStorageRole::AvailableSize);

		item->setText (name);
		item->setData (DeviceType::MassStorage, CommonDevRole::DevType);
		item->setData (iface->property ("DeviceFile").toString (), MassStorageRole::DevFile);
		item->setData (iface->property ("PartitionType").toInt (), MassStorageRole::PartType);
		item->setData (isRemovable, MassStorageRole::IsRemovable);
		item->setData (isPartition, MassStorageRole::IsPartition);
		item->setData (isPartition && hasRemovableParent, MassStorageRole::IsMountable);
		item->setData (iface->property ("DeviceIsMounted").toBool (), MassStorageRole::IsMounted);
		item->setData (iface->property ("DeviceIsMediaAvailable"), MassStorageRole::IsMediaAvailable);
		item->setData (iface->path (), CommonDevRole::DevID);
		item->setData (fullName, MassStorageRole::VisibleName);
		item->setData (iface->property ("PartitionSize").toLongLong (), MassStorageRole::TotalSize);
		DevicesModel_->blockSignals (false);
		item->setData (mountPaths, MassStorageRole::MountPoints);
	}

	void Backend::toggleMount (const QString& id)
	{
		auto iface = GetDeviceInterface (id);
		if (!iface)
			return;

		const bool isMounted = iface->property ("DeviceIsMounted").toBool ();
		if (isMounted)
		{
			auto async = iface->asyncCall ("FilesystemUnmount", QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (umountCallFinished (QDBusPendingCallWatcher*)));
		}
		else
		{
			auto async = iface->asyncCall ("FilesystemMount", QString (), QStringList ());
			connect (new QDBusPendingCallWatcher (async, this),
					SIGNAL (finished (QDBusPendingCallWatcher*)),
					this,
					SLOT (mountCallFinished (QDBusPendingCallWatcher*)));
		}
	}

	void Backend::startInitialEnumerate ()
	{
		InitialEnumerate ();
	}

	namespace
	{
		QString GetErrorText (const QString& errorCode)
		{
			QMap<QString, QString> texts;
			texts ["org.freedesktop.UDisks.Error.PermissionDenied"] = Backend::tr ("permission denied");
			texts ["org.freedesktop.PolicyKit.Error.NotAuthorized"] = Backend::tr ("not authorized");
			texts ["org.freedesktop.PolicyKit.Error.Busy"] = Backend::tr ("the device is busy");
			texts ["org.freedesktop.PolicyKit.Error.Failed"] = Backend::tr ("the operation has failed");
			texts ["org.freedesktop.PolicyKit.Error.Cancelled"] = Backend::tr ("the operation has been cancelled");
			texts ["org.freedesktop.PolicyKit.Error.InvalidOption"] = Backend::tr ("invalid mount options were given");
			texts ["org.freedesktop.PolicyKit.Error.FilesystemDriverMissing"] = Backend::tr ("unsupported filesystem");
			return texts.value (errorCode, Backend::tr ("unknown error"));
		}
	}

	void Backend::mountCallFinished (QDBusPendingCallWatcher *watcher)
	{
		qDebug () << Q_FUNC_INFO;
		watcher->deleteLater ();
		QDBusPendingReply<QString> reply = *watcher;

		if (!reply.isError ())
		{
			HandleEntity (Util::MakeNotification ("Vrooby",
						tr ("Device has been successfully mounted at %1.")
							.arg (reply.value ()),
						Priority::Info));
			return;
		}

		const auto& error = reply.error ();
		qWarning () << Q_FUNC_INFO
				<< error.name ()
				<< error.message ();
		HandleEntity (Util::MakeNotification ("Vrooby",
					tr ("Failed to mount the device: %1 (%2).")
						.arg (GetErrorText (error.name ()))
						.arg (error.message ()),
					Priority::Critical));
	}

	void Backend::umountCallFinished (QDBusPendingCallWatcher *watcher)
	{
		qDebug () << Q_FUNC_INFO;
		watcher->deleteLater ();
		QDBusPendingReply<void> reply = *watcher;

		if (!reply.isError ())
		{
			HandleEntity (Util::MakeNotification ("Vrooby",
						tr ("Device has been successfully unmounted."),
						Priority::Info));
			return;
		}

		const auto& error = reply.error ();
		qWarning () << Q_FUNC_INFO
				<< error.name ()
				<< error.message ();
		HandleEntity (Util::MakeNotification ("Vrooby",
					tr ("Failed to unmount the device: %1 (%2).")
						.arg (GetErrorText (error.name ()))
						.arg (error.message ()),
					Priority::Critical));
	}

	void Backend::handleEnumerationFinished (QDBusPendingCallWatcher *watcher)
	{
		watcher->deleteLater ();
		QDBusPendingReply<QList<QDBusObjectPath>> reply = *watcher;
		if (reply.isError ())
		{
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

		auto item = Object2Item_.value (path);
		SetItemData (GetDeviceInterface (path), item);
	}

	void Backend::updateDeviceSpaces ()
	{
		for (QStandardItem *item : Object2Item_.values ())
		{
			const auto& mountPaths = item->data (MassStorageRole::MountPoints).toStringList ();
			if (mountPaths.isEmpty ())
				continue;

			const auto free = QStorageInfo { mountPaths.value (0) }.bytesAvailable ();
			if (free != item->data (MassStorageRole::AvailableSize).value<qint64> ())
				item->setData (free, MassStorageRole::AvailableSize);
		}
	}
}
}
}
