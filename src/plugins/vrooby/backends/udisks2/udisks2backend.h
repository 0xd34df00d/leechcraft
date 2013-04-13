/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

namespace LeechCraft
{
namespace Vrooby
{
namespace UDisks2
{
	class Backend : public DevBackend
	{
		Q_OBJECT

		QStandardItemModel *DevicesModel_;

		org::freedesktop::DBus::ObjectManager *UDisksObj_;
		QHash<QString, QStandardItem*> Object2Item_;
		QSet<QString> Unremovables_;
	public:
		Backend (QObject* = 0);

		QString GetBackendName () const;
		bool IsAvailable ();
		void Start ();

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
