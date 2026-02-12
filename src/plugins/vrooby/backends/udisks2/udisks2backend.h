/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "../../devbackend.h"
#include <memory>
#include <QHash>
#include <interfaces/devices/deviceroles.h>
#include <util/models/itemsmodel.h>
#include <util/threads/coro/taskfwd.h>
#include "dbus/manager.h"

class QDBusObjectPath;
class QStandardItemModel;
class QStandardItem;
class QDBusInterface;
class QDBusPendingCallWatcher;

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LC::Vrooby::UDisks2
{
	class Backend : public DevBackend
	{
		Q_OBJECT

		template<typename T, int RV>
		using R = Util::RoleOf<T, RV>;

		struct Device
		{
			R<QByteArray, CommonDevRole::DevID> DevId_;

			R<QString, Qt::DisplayRole> Name_;
			R<QString, MassStorageRole::VisibleName> VisibleName_;

			R<QStringList, MassStorageRole::MountPoints> MountPoints_;

			R<QByteArray, MassStorageRole::DevFile> DevFile_;
			R<QByteArray, CommonDevRole::DevParentID> DevParentId_ {};
			R<QByteArray, CommonDevRole::DevPersistentID> DevPersistentId_;

			R<qint64, MassStorageRole::AvailableSize> AvailableSize_ = -1;
			R<qint64, MassStorageRole::TotalSize> TotalSize_ = -1;

			R<uint16_t, MassStorageRole::PartType> PartitionType_ = 0;

			R<uint8_t, CommonDevRole::DevType> DevType_ = DeviceType::MassStorage;

			R<bool, MassStorageRole::IsRemovable> IsRemovable_ = false;
			R<bool, MassStorageRole::IsPartition> IsPartition_ = false;
			R<bool, MassStorageRole::IsMountable> IsMountable_ = false;
			R<bool, MassStorageRole::IsMounted> IsMounted_ = false;
			R<bool, MassStorageRole::IsMediaAvailable> IsMediaAvailable_ = false;
		};
		Util::RoledItemsModel<Device> Devices_;
		QHash<QString, int> Id2Row_;

		org::freedesktop::DBus::ObjectManager *UDisksObj_ = nullptr;
	public:
		static bool IsAvailable ();
		static QString GetBackendName ();

		void Start () override;

		bool SupportsDevType (DeviceType) const override;
		QAbstractItemModel* GetDevicesModel () override;
		Util::Task<void> MountDevice (const QString&) override;
	private:
		Util::ContextTask<void> InitialEnumerate ();
		bool AddPath (const QDBusObjectPath&);
		void RemovePath (const QDBusObjectPath&);

		struct ItemInterfaces
		{
			QDBusInterface_ptr Partition_;
			QDBusInterface_ptr Block_;
			QDBusInterface_ptr Drive_;
			QDBusInterface_ptr Props_;
		};
		static Device ToDevice (const ItemInterfaces&);

		void UpdateDeviceSpaces ();
	public slots:
		void toggleMount (const QString&) override;
	private slots:
		void handleDeviceChanged (const QDBusMessage&);
	};

	static_assert (DevBackendType<Backend>);
}
