/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "udisks2backend.h"
#include <memory>
#include <ranges>
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

using QDBusInterface_ptr = std::shared_ptr<QDBusInterface>;

namespace LC::Vrooby::UDisks2
{
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
		return "UDisks2"_qs;
	}

	void Backend::Start ()
	{
		qDBusRegisterMetaType<VariantMapMap_t> ();
		qDBusRegisterMetaType<EnumerationResult_t> ();
		qDBusRegisterMetaType<ByteArrayList_t> ();

		InitialEnumerate ();

		using namespace std::chrono_literals;

		const auto timer = new QTimer { this };
		timer->callOnTimeout ([this] { UpdateDeviceSpaces (); });
		timer->start (10s);
	}

	bool Backend::SupportsDevType (DeviceType type) const
	{
		return type == DeviceType::MassStorage;
	}

	QAbstractItemModel* Backend::GetDevicesModel ()
	{
		return &Devices_;
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

		const auto NotificationTitle = "Vrooby"_qs;

		Util::Task<void> RunMount (QString id)
		{
			const auto iface = GetInterface (id, Interfaces::Filesystem);
			Util::Visit (co_await Util::Typed<QString> (iface->asyncCall ("Mount"_qs, QVariantMap {})),
					[] (const QString& path)
					{
						HandleEntity (Util::MakeNotification (NotificationTitle,
								Backend::tr ("Device has been successfully mounted at %1.").arg (path),
								Priority::Info));
					},
					[] (const QDBusError& error)
					{
						qWarning () << error.name () << error.message ();
						HandleEntity (Util::MakeNotification (NotificationTitle,
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
						HandleEntity (Util::MakeNotification (NotificationTitle,
								Backend::tr ("Device has been successfully unmounted."),
								Priority::Info));
					},
					[] (const QDBusError& error)
					{
						qWarning () << error.name () << error.message ();
						HandleEntity (Util::MakeNotification (NotificationTitle,
								Backend::tr ("Failed to unmount the device: %1 (%2).")
										.arg (GetErrorText (error.name ()), error.message ()),
								Priority::Critical));
					});
		}
	}

	Util::Task<void> Backend::MountDevice (const QString& id)
	{
		const auto pos = Id2Row_.value (id, -1);
		if (pos == -1)
			co_return;

		if (!Devices_.GetItems ().value (pos).IsMounted_)
			co_return co_await RunMount (id);
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
		if (Id2Row_.contains (str))
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

		if (const auto& slaveTo = partitionIface->property ("Table").value<QDBusObjectPath> ();
			!slaveTo.path ().isEmpty () && !AddPath (slaveTo))
			return false;

		Id2Row_ [str] = Devices_.AddItem (ToDevice ({
					.Partition_ = partitionIface,
					.Block_ = blockIface,
					.Drive_ = driveIface,
					.Props_ = GetInterface (str, Interfaces::Props),
				}));
		return true;
	}

	void Backend::RemovePath (const QDBusObjectPath& pathObj)
	{
		const auto& path = pathObj.path ();
		if (const auto it = Id2Row_.find (path);
			it != Id2Row_.end ())
		{
			const auto row = *it;
			Id2Row_.erase (it);
			Devices_.RemoveItem (row);
		}
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

		QByteArray FixupTrailingZero (QByteArray ba)
		{
			if (ba.endsWith ('\0'))
				ba.chop (1);
			return ba;
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
					mountPaths << QString::fromUtf8 (FixupTrailingZero (point));

			return mountPaths;
		}
	}

	Backend::Device Backend::ToDevice (const ItemInterfaces& ifaces)
	{
		const auto& path = ifaces.Block_->path ();
		const auto& slaveTo = ifaces.Partition_->property ("Table").value<QDBusObjectPath> ();
		const bool isRemovable = ifaces.Drive_->property ("Removable").toBool ();
		const bool isPartition = ifaces.Block_->property ("IdUsage").toString () == "filesystem"_ql;

		static const bool debugUdisks = qgetenv ("LC_VROOBY_DEBUG_UDISKS") == "1";
		if (debugUdisks)
			qDebug () << path << slaveTo.path () << isPartition << isRemovable;

		const auto& vendor = ifaces.Drive_->property ("Vendor").toString () +
				' ' +
				ifaces.Drive_->property ("Model").toString ();
		const auto& partName = GetPartitionName (*ifaces.Partition_, *ifaces.Block_);
		const auto& name = isPartition ? partName : vendor;
		const auto& fullName = isPartition ?
				"%1: %2"_qs.arg (vendor, partName) :
				vendor;

		const auto& mountPaths = GetMountPaths (*ifaces.Props_);
		return Device
		{
			.DevId_ { path.toUtf8 () },
			.Name_ { name },
			.VisibleName_ { fullName },
			.MountPoints_ { mountPaths },
			.DevFile_ { FixupTrailingZero (ifaces.Block_->property ("Device").toByteArray ()) },
			.DevParentId_ { slaveTo.path ().toUtf8 () },
			.DevPersistentId_ { ifaces.Block_->property ("IdUUID").toByteArray () },
			.AvailableSize_ { mountPaths.isEmpty () ? -1 : QStorageInfo { mountPaths [0] }.bytesAvailable () },
			.TotalSize_ { ifaces.Block_->property ("Size").toLongLong () },
			.PartitionType_ { ifaces.Partition_->property ("PartitionType").value<uint16_t> () },
			.IsRemovable_ { isRemovable },
			.IsPartition_ { isPartition },
			.IsMountable_ { isPartition && isRemovable },
			.IsMounted_ { !mountPaths.isEmpty () },
			.IsMediaAvailable_ { ifaces.Drive_->property ("MediaAvailable").toBool () },
		};
	}

	void Backend::toggleMount (const QString& id)
	{
		const auto row = Id2Row_.value (id, -1);
		if (row < 0)
		{
			qWarning () << "unknown id" << id;
			return;
		}

		if (Devices_.GetItems ().value (row).IsMounted_)
			RunUnmount (id);
		else
			RunMount (id);
	}

	void Backend::handleDeviceChanged (const QDBusMessage& msg)
	{
		const auto& path = msg.path ();

		const auto row = Id2Row_.value (path, -1);
		if (row < 0)
		{
			qWarning () << "no item for path" << path;
			return;
		}

		const auto blockIface = GetInterface (path, Interfaces::Block);
		const ItemInterfaces faces =
		{
			.Partition_ = GetInterface (path, Interfaces::Partition),
			.Block_ = blockIface,
			.Drive_ = GetInterface (blockIface->property ("Drive").value<QDBusObjectPath> ().path (), Interfaces::Drive),
			.Props_ = GetInterface (path, Interfaces::Props)
		};
		Devices_.SetItem (row, ToDevice (faces));
	}

	void Backend::UpdateDeviceSpaces ()
	{
		for (auto&& [idx, device] : std::ranges::views::enumerate (Devices_.GetItems ()))
		{
			const auto& mountPaths = device.MountPoints_;
			if (mountPaths->isEmpty ())
				continue;

			if (const auto free = QStorageInfo { mountPaths->value (0) }.bytesAvailable ();
				free != device.AvailableSize_)
				Devices_.SetField<&Device::AvailableSize_> (idx, free);
		}
	}
}
