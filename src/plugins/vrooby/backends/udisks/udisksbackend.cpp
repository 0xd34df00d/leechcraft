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

typedef std::unique_ptr<QDBusInterface> QDBusInterface_ptr;

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

		const bool isPartition = iface->property ("DeviceIsPartition").toBool ();

		const QString& name = isPartition ?
				tr ("Partition %1")
					.arg (iface->property ("PartitionNumber").toInt ()) :
				(iface->property ("DriveVendor").toString () +
						" " +
						iface->property ("DriveModel").toString ());
		QStandardItem *item = new QStandardItem (name);
		Object2Item_ [str] = item;
		if (!isSlave)
			DevicesModel_->appendRow (item);
		else
		{
			if (!Object2Item_.contains (slaveTo.path ()))
				AddPath (slaveTo);
			Object2Item_ [slaveTo.path ()]->appendRow (item);
		}
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

		qDebug () << "begin enumeration";
		for (const QDBusObjectPath& path : reply.value ())
			AddPath (path);
		qDebug () << "end";
	}
}
}
}
