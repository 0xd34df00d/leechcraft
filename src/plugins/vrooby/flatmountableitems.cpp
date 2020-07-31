/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flatmountableitems.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <interfaces/devices/deviceroles.h>
#include <util/util.h>

namespace LC
{
namespace Vrooby
{
	FlatMountableItems::FlatMountableItems (QObject *parent)
	: RoleNamesMixin<FlattenFilterModel> (parent)
	{
		QHash<int, QByteArray> names;
		names [MassStorageRole::VisibleName] = "devName";
		names [MassStorageRole::DevFile] = "devFile";
		names [MassStorageRole::IsRemovable] = "isRemovable";
		names [MassStorageRole::IsPartition] = "isPartition";
		names [MassStorageRole::IsMountable] = "isMountable";
		names [CommonDevRole::DevID] = "devID";
		names [CommonDevRole::DevPersistentID] = "devPersistentID";
		names [MassStorageRole::AvailableSize] = "availableSize";
		names [MassStorageRole::TotalSize] = "totalSize";
		names [CustomRoles::FormattedTotalSize] = "formattedTotalSize";
		names [CustomRoles::FormattedFreeSpace] = "formattedFreeSpace";
		names [CustomRoles::UsedPercentage] = "usedPercentage";
		names [CustomRoles::MountButtonIcon] = "mountButtonIcon";
		names [CustomRoles::ToggleHiddenIcon] = "toggleHiddenIcon";
		names [CustomRoles::MountedAt] = "mountedAt";
		setRoleNames (names);
	}

	QVariant FlatMountableItems::data (const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case CustomRoles::FormattedTotalSize:
		{
			const auto size = index.data (MassStorageRole::TotalSize).toLongLong ();
			return tr ("total size: %1")
				.arg (Util::MakePrettySize (size));
		}
		case CustomRoles::FormattedFreeSpace:
		{
			const auto size = index.data (MassStorageRole::AvailableSize).toLongLong ();
			return tr ("available size: %1")
				.arg (Util::MakePrettySize (size));
		}
		case CustomRoles::MountButtonIcon:
			return index.data (MassStorageRole::IsMounted).toBool () ?
					"image://ThemeIcons/emblem-unmounted" :
					"image://ThemeIcons/emblem-mounted";
		case CustomRoles::MountedAt:
		{
			const auto& mounts = index.data (MassStorageRole::MountPoints).toStringList ();
			return mounts.isEmpty () ?
					"" :
					tr ("Mounted at %1").arg (mounts.join ("; "));
		}
		case CustomRoles::UsedPercentage:
		{
			const qint64 free = index.data (MassStorageRole::AvailableSize).value<qint64> ();
			if (free < 0)
				return -1;

			const double total = index.data (MassStorageRole::TotalSize).value<qint64> ();
			return (1 - free / total) * 100;
		}
		default:
			return Util::FlattenFilterModel::data (index, role);
		}
	}

	bool FlatMountableItems::IsIndexAccepted (const QModelIndex& child) const
	{
		return child.data (MassStorageRole::IsMountable).toBool ();
	}
}
}
