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
namespace UDisks
{
	class Backend : public DevBackend
	{
		Q_OBJECT

		QStandardItemModel *DevicesModel_;

		QDBusInterface *UDisksObj_ = nullptr;
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
		void SetItemData (QDBusInterface_ptr, QStandardItem*);
	public slots:
		void toggleMount (const QString&);
	private slots:
		void startInitialEnumerate ();
		void mountCallFinished (QDBusPendingCallWatcher*);
		void umountCallFinished (QDBusPendingCallWatcher*);
		void handleEnumerationFinished (QDBusPendingCallWatcher*);
		void handleDeviceAdded (const QDBusObjectPath&);
		void handleDeviceRemoved (const QDBusObjectPath&);
		void handleDeviceChanged (const QDBusObjectPath&);
		void updateDeviceSpaces ();
	};
}
}
}
