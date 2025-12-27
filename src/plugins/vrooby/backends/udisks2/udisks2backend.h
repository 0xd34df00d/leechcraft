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

		QStandardItemModel *DevicesModel_;

		org::freedesktop::DBus::ObjectManager *UDisksObj_ = nullptr;
		QHash<QString, QStandardItem*> Object2Item_;
	public:
		explicit Backend ();

		static bool IsAvailable ();
		static QString GetBackendName ();

		void Start () override;

		bool SupportsDevType (DeviceType) const override;
		QAbstractItemModel* GetDevicesModel () const override;
		void MountDevice (const QString&) override;
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
		void SetItemData (const ItemInterfaces&, QStandardItem*);

		void UpdateDeviceSpaces ();
	public slots:
		void toggleMount (const QString&) override;
	private slots:
		void handleDeviceChanged (const QDBusMessage&);
	};

	static_assert (DevBackendType<Backend>);
}
