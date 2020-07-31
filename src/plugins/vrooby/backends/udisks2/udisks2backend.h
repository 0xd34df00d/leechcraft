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
#include <QSet>
#include <QVariantMap>
#include "dbus/manager.h"

class QDBusObjectPath;
class QStandardItemModel;
class QStandardItem;
class QDBusInterface;
class QDBusPendingCallWatcher;

typedef std::shared_ptr<QDBusInterface> QDBusInterface_ptr;

namespace LC
{
namespace Vrooby
{
namespace UDisks2
{
	class Backend : public DevBackend
	{
		Q_OBJECT

		QStandardItemModel *DevicesModel_;

		org::freedesktop::DBus::ObjectManager *UDisksObj_ = nullptr;
		QHash<QString, QStandardItem*> Object2Item_;
		QSet<QString> Unremovables_;
	public:
		Backend (const ICoreProxy_ptr&);

		QString GetBackendName () const;
		bool IsAvailable ();
		void Start ();

		bool SupportsDevType (DeviceType) const;
		QAbstractItemModel* GetDevicesModel () const;
		void MountDevice (const QString&);
	private:
		void InitialEnumerate ();
		bool AddPath (const QDBusObjectPath&);
		void RemovePath (const QDBusObjectPath&);

		struct ItemInterfaces
		{
			QDBusInterface_ptr Partition_;
			QDBusInterface_ptr FS_;
			QDBusInterface_ptr Block_;
			QDBusInterface_ptr Drive_;
			QDBusInterface_ptr Props_;
		};
		void SetItemData (const ItemInterfaces&, QStandardItem*);
	public slots:
		void toggleMount (const QString&);
	private slots:
		void mountCallFinished (QDBusPendingCallWatcher*);
		void umountCallFinished (QDBusPendingCallWatcher*);
		void handleEnumerationFinished (QDBusPendingCallWatcher*);
		void handleDeviceAdded (const QDBusObjectPath&, const VariantMapMap_t&);
		void handleDeviceRemoved (const QDBusObjectPath&);
		void handleDeviceChanged (const QDBusMessage&);
		void updateDeviceSpaces ();
	};
}
}
}
