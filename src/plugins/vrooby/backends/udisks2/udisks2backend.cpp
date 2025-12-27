/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "udisks2backend.h"
#include <memory>
#include <QStandardItemModel>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QStorageInfo>
#include <QTimer>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/threads/coro/dbus.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/devices/deviceroles.h>
#include "udisks2types.h"

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LC::Vrooby::UDisks2
{
	Backend::Backend ()
	: DevicesModel_ { new QStandardItemModel { this } }
	{
	}

	namespace
	{
		const auto UDisks2Service = "org.freedesktop.UDisks2"_qs;
	}

	bool Backend::IsAvailable ()
	{
		const auto sb = QDBusConnection::systemBus ();
		const auto iface = sb.interface ();

		auto services = iface->registeredServiceNames ().value ().filter (UDisks2Service);
		if (!services.isEmpty ())
			return true;

		iface->startService (UDisks2Service);
		services = iface->registeredServiceNames ().value ().filter (UDisks2Service);
		return !services.isEmpty ();
	}

	QString Backend::GetBackendName ()
	{
		return "UDisks2";
	}

	void Backend::Start ()
	{
		qDBusRegisterMetaType<VariantMapMap_t> ();
		qDBusRegisterMetaType<EnumerationResult_t> ();
		qDBusRegisterMetaType<ByteArrayList_t> ();

		InitialEnumerate ();

		const auto timer = new QTimer { this };
		timer->callOnTimeout ([this] { UpdateDeviceSpaces (); });
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
		namespace Interfaces
		{
			const auto Block = "org.freedesktop.UDisks2.Block"_qs;
			const auto Partition = "org.freedesktop.UDisks2.Partition"_qs;
			const auto Filesystem = "org.freedesktop.UDisks2.Filesystem"_qs;
			const auto Drive = "org.freedesktop.UDisks2.Drive"_qs;
			const auto Props = "org.freedesktop.DBus.Properties"_qs;
		}

		QDBusInterface_ptr GetInterface (const QString& path, const QString& iface)
		{
			return std::make_shared<QDBusInterface> (UDisks2Service,
					path,
					iface,
					QDBusConnection::systemBus ());
		}

		bool IsMounted (QStandardItem *item)
		{
			return !item->data (MassStorageRole::MountPoints).toStringList ().isEmpty ();
		}

		QString GetErrorText (const QString& errorCode)
		{
			static const QMap<QString, QString> texts
			{
				{ "org.freedesktop.UDisks.Error.PermissionDenied"_qs, Backend::tr ("permission denied") },
				{ "org.freedesktop.PolicyKit.Error.NotAuthorized"_qs, Backend::tr ("not authorized") },
				{ "org.freedesktop.PolicyKit.Error.Busy"_qs, Backend::tr ("the device is busy") },
				{ "org.freedesktop.PolicyKit.Error.Failed"_qs, Backend::tr ("the operation has failed") },
				{ "org.freedesktop.PolicyKit.Error.Cancelled"_qs, Backend::tr ("the operation has been cancelled") },
				{ "org.freedesktop.PolicyKit.Error.InvalidOption"_qs, Backend::tr ("invalid mount options were given") },
				{ "org.freedesktop.PolicyKit.Error.FilesystemDriverMissing"_qs, Backend::tr ("unsupported filesystem") },
			};
			return texts.value (errorCode, Backend::tr ("unknown error"));
		}

		void HandleEntity (const Entity& e)
		{
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}

		Util::Task<void> RunMount (QString id)
		{
			const auto iface = GetInterface (id, Interfaces::Filesystem);
			Util::Visit (co_await Util::Typed<QString> (iface->asyncCall ("Mount"_qs, QVariantMap {})),
					[] (const QString& path)
					{
						HandleEntity (Util::MakeNotification ("Vrooby",
								Backend::tr ("Device has been successfully mounted at %1.").arg (path),
								Priority::Info));
					},
					[] (const QDBusError& error)
					{
						qWarning () << error.name () << error.message ();
						HandleEntity (Util::MakeNotification ("Vrooby",
								Backend::tr ("Failed to mount the device: %1 (%2).")
									.arg (GetErrorText (error.name ()), error.message ()),
								Priority::Critical));
					});
		}

		Util::Task<void> RunUnmount (QString id)
		{
			const auto iface = GetInterface (id, Interfaces::Filesystem);
			Util::Visit (co_await Util::Typed<> (iface->asyncCall ("Unmount"_qs, QVariantMap {})),
					[] (Util::Void)
					{
						HandleEntity (Util::MakeNotification ("Vrooby",
								Backend::tr ("Device has been successfully unmounted."),
								Priority::Info));
					},
					[] (const QDBusError& error)
					{
						qWarning () << error.name () << error.message ();
						HandleEntity (Util::MakeNotification ("Vrooby",
								Backend::tr ("Failed to unmount the device: %1 (%2).")
										.arg (GetErrorText (error.name ()), error.message ()),
								Priority::Critical));
					});
		}
	}

	void Backend::MountDevice (const QString& id)
	{
		const auto item = Object2Item_.value (id);
		if (!item)
			return;

		if (!IsMounted (item))
			RunMount (id);
	}

	Util::ContextTask<void> Backend::InitialEnumerate ()
	{
		namespace dbus = org::freedesktop::DBus;

		co_await Util::AddContextObject { *this };

		const auto sb = QDBusConnection::systemBus ();
		UDisksObj_ = new dbus::ObjectManager (UDisks2Service, "/org/freedesktop/UDisks2"_qs, sb);
		connect (UDisksObj_,
				&dbus::ObjectManager::InterfacesAdded,
				this,
				&Backend::AddPath);
		connect (UDisksObj_,
				&dbus::ObjectManager::InterfacesRemoved,
				this,
				[this] (const QDBusObjectPath& path, const QStringList& removed)
				{
					if (removed.contains ("org.freedesktop.UDisks2.Block"_ql))
						RemovePath (path);
				});

		const auto result = co_await Util::Typed<EnumerationResult_t> (UDisksObj_->GetManagedObjects ());
		Util::Visit (co_await Util::Typed<EnumerationResult_t> (UDisksObj_->GetManagedObjects ()),
				[] (const QDBusError& error)
				{
					qWarning () << error.message ();
				},
				[this] (const EnumerationResult_t& devices)
				{
					for (const auto& [path, _] : devices.asKeyValueRange ())
						AddPath (path);
				});
	}

	bool Backend::AddPath (const QDBusObjectPath& path)
	{
		const auto& str = path.path ();
		if (Object2Item_.contains (str))
			return true;

		const auto blockIface = GetInterface (str, Interfaces::Block);
		if (!blockIface->isValid ())
		{
			qWarning () << "invalid interface for" << str << blockIface->lastError ().message ();
			return false;
		}

		const auto& driveId = blockIface->property ("Drive").value<QDBusObjectPath> ().path ();

		const auto driveIface = driveId.isEmpty () ? QDBusInterface_ptr {} : GetInterface (driveId, Interfaces::Drive);
		if (!driveIface || !driveIface->isValid ())
			return false;

		const auto partitionIface = GetInterface (str, Interfaces::Partition);

		QDBusConnection::systemBus ().connect (UDisks2Service,
				path.path (),
				"org.freedesktop.DBus.Properties"_qs,
				"PropertiesChanged"_qs,
				this,
				SLOT (handleDeviceChanged (QDBusMessage)));

		const auto item = new QStandardItem;
		Object2Item_ [str] = item;
		SetItemData ({
					partitionIface,
					blockIface,
					driveIface,
					GetInterface (str, Interfaces::Props),
				}, item);
		if (const auto& slaveTo = partitionIface->property ("Table").value<QDBusObjectPath> ();
			!slaveTo.path ().isEmpty () && !AddPath (slaveTo))
			return false;

		DevicesModel_->appendRow (item);
		return true;
	}

	void Backend::RemovePath (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		if (const auto item = Object2Item_.take (path))
			DevicesModel_->removeRow (item->row ());
	}

	namespace
	{
		QString GetPartitionName (const QDBusInterface& partition, const QDBusInterface& block)
		{
			auto result = block.property ("IdLabel").toString ().trimmed ();
			if (!result.isEmpty ())
				return result;

			result = partition.property ("Name").toString ().trimmed ();
			if (!result.isEmpty ())
				return result;

			return Backend::tr ("Partition %1").arg (partition.property ("Number").toInt ());
		}

		auto GetMountPaths (QDBusInterface& props)
		{
			QStringList mountPaths;

			// This needs to be a `Get` DBus method call since querying `property()` on the Filesystem interface
			// won't work: Qt DBus doesn't know how to demarshall "aay" into QByteArrayList in `property()` automagic.
			const auto msg = props.call ("Get"_qs, "org.freedesktop.UDisks2.Filesystem"_qs, "MountPoints"_qs);
			if (const QDBusReply<QDBusVariant> reply { msg };
				reply.isValid ())
				for (auto point : qdbus_cast<ByteArrayList_t> (reply.value ().variant ()))
				{
					if (point.endsWith ('\0'))
						point.chop (1);
					mountPaths << QString::fromUtf8 (point);
				}

			return mountPaths;
		}
	}

	void Backend::SetItemData (const ItemInterfaces& ifaces, QStandardItem *item)
	{
		if (!item)
			return;

		const auto& path = ifaces.Block_->path ();
		const auto& slaveTo = ifaces.Partition_->property ("Table").value<QDBusObjectPath> ();
		const bool isRemovable = ifaces.Drive_->property ("Removable").toBool ();
		const bool isPartition = ifaces.Block_->property ("IdUsage").toString () == "filesystem"_ql;

		static const bool debugUdisks = qgetenv ("LC_VROOBY_DEBUG_UDISKS") == "1";
		if (debugUdisks)
			qDebug () << path << slaveTo.path () << isPartition << isRemovable;

		const auto& vendor = ifaces.Drive_->property ("Vendor").toString () +
				" " +
				ifaces.Drive_->property ("Model").toString ();
		const auto& partName = GetPartitionName (*ifaces.Partition_, *ifaces.Block_);
		const auto& name = isPartition ? partName : vendor;
		const auto& fullName = isPartition ?
				"%1: %2"_qs.arg (vendor, partName) :
				vendor;

		DevicesModel_->blockSignals (true);

		const auto& mountPaths = GetMountPaths (*ifaces.Props_);
		if (!mountPaths.isEmpty ())
			item->setData (QStorageInfo { mountPaths.value (0) }.bytesAvailable (), MassStorageRole::AvailableSize);
		else
			item->setData (-1, MassStorageRole::AvailableSize);

		item->setText (name);
		item->setData (DeviceType::MassStorage, CommonDevRole::DevType);
		item->setData (ifaces.Block_->property ("Device").toByteArray (), MassStorageRole::DevFile);
		item->setData (ifaces.Partition_->property ("PartitionType").toInt (), MassStorageRole::PartType);
		item->setData (isRemovable, MassStorageRole::IsRemovable);
		item->setData (isPartition, MassStorageRole::IsPartition);
		item->setData (isPartition && isRemovable, MassStorageRole::IsMountable);
		item->setData (!mountPaths.isEmpty (), MassStorageRole::IsMounted);
		item->setData (ifaces.Drive_->property ("MediaAvailable"), MassStorageRole::IsMediaAvailable);
		item->setData (path, CommonDevRole::DevID);
		if (!slaveTo.path ().isEmpty ())
			item->setData (slaveTo.path (), CommonDevRole::DevParentID);
		item->setData (ifaces.Block_->property ("IdUUID"), CommonDevRole::DevPersistentID);
		item->setData (fullName, MassStorageRole::VisibleName);
		item->setData (ifaces.Block_->property ("Size").toLongLong (), MassStorageRole::TotalSize);
		DevicesModel_->blockSignals (false);
		item->setData (mountPaths, MassStorageRole::MountPoints);
	}

	void Backend::toggleMount (const QString& id)
	{
		const auto item = Object2Item_.value (id);
		if (!item)
			return;

		if (IsMounted (item))
			RunUnmount (id);
		else
			RunMount (id);
	}

	void Backend::handleDeviceChanged (const QDBusMessage& msg)
	{
		const auto& path = msg.path ();

		const auto item = Object2Item_.value (path);
		if (!item)
		{
			qWarning () << "no item for path" << path;
			return;
		}

		const auto blockIface = GetInterface (path, Interfaces::Block);
		const ItemInterfaces faces =
		{
			GetInterface (path, Interfaces::Partition),
			blockIface,
			GetInterface (blockIface->property ("Drive").value<QDBusObjectPath> ().path (), Interfaces::Drive),
			GetInterface (path, Interfaces::Props)
		};
		SetItemData (faces, item);
	}

	void Backend::UpdateDeviceSpaces ()
	{
		for (const auto item : Object2Item_)
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
