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

#include "flatmountableitems.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <interfaces/iremovabledevmanager.h>
#include <util/util.h>

namespace LeechCraft
{
namespace Vrooby
{
	FlatMountableItems::FlatMountableItems (QObject *parent)
	: Util::FlattenFilterModel (parent)
	{
		QHash<int, QByteArray> names;
		names [DeviceRoles::VisibleName] = "devName";
		names [DeviceRoles::DevFile] = "devFile";
		names [DeviceRoles::IsRemovable] = "isRemovable";
		names [DeviceRoles::IsPartition] = "isPartition";
		names [DeviceRoles::IsMountable] = "isMountable";
		names [DeviceRoles::DevID] = "devID";
		names [DeviceRoles::AvailableSize] = "availableSize";
		names [DeviceRoles::TotalSize] = "totalSize";
		names [CustomRoles::FormattedTotalSize] = "formattedTotalSize";
		names [CustomRoles::FormattedFreeSpace] = "formattedFreeSpace";
		names [CustomRoles::UsedPercentage] = "usedPercentage";
		names [CustomRoles::MountButtonIcon] = "mountButtonIcon";
		names [CustomRoles::MountedAt] = "mountedAt";
		setRoleNames (names);
	}

	QVariant FlatMountableItems::data (const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case CustomRoles::FormattedTotalSize:
		{
			const auto size = index.data (DeviceRoles::TotalSize).toLongLong ();
			return tr ("total size: %1")
				.arg (Util::MakePrettySize (size));
		}
		case CustomRoles::FormattedFreeSpace:
		{
			const auto size = index.data (DeviceRoles::AvailableSize).toLongLong ();
			return tr ("available size: %1")
				.arg (Util::MakePrettySize (size));
		}
		case CustomRoles::MountButtonIcon:
			return index.data (DeviceRoles::IsMounted).toBool () ?
					"image://mountIcons/emblem-unmounted" :
					"image://mountIcons/emblem-mounted";
		case CustomRoles::MountedAt:
		{
			const auto& mounts = index.data (DeviceRoles::MountPoints).toStringList ();
			return mounts.isEmpty () ?
					"" :
					tr ("Mounted at %1").arg (mounts.join ("; "));
		}
		case CustomRoles::UsedPercentage:
		{
			const qint64 free = index.data (DeviceRoles::AvailableSize).value<qint64> ();
			if (free < 0)
				return -1;

			const double total = index.data (DeviceRoles::TotalSize).value<qint64> ();
			return (1 - free / total) * 100;
		}
		default:
			return Util::FlattenFilterModel::data (index, role);
		}
	}

	bool FlatMountableItems::IsIndexAccepted (const QModelIndex& child) const
	{
		return child.data (DeviceRoles::IsMountable).toBool ();
	}
}
}
