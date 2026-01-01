/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trayproxymodel.h"
#include <QCoreApplication>
#include <QSettings>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>
#include <util/util.h>

namespace LC::Vrooby
{
	namespace
	{
		QSettings MakeSettings ()
		{
			return QSettings { QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Vrooby"_qs };
		}
	}

	TrayProxyModel::TrayProxyModel (QObject *parent)
	: QSortFilterProxyModel { parent }
	{
		auto settings = MakeSettings ();
		settings.beginGroup ("HiddenDevices");
		Hidden_ = Util::AsSet (settings.value ("List").toStringList ());
		settings.endGroup ();
	}

	QVariant TrayProxyModel::data (const QModelIndex& index, int role) const
	{
		switch (role)
		{
		case CustomRoles::ToggleHiddenIcon:
		{
			const auto& id = index.data (CommonDevRole::DevPersistentID).toString ();
			return Hidden_.contains (id) ?
					"image://ThemeIcons/list-add"_qs :
					"image://ThemeIcons/list-remove"_qs;
		}
		case CustomRoles::FormattedTotalSize:
		{
			const auto size = index.data (MassStorageRole::TotalSize).toLongLong ();
			return tr ("total size: %1").arg (Util::MakePrettySize (size));
		}
		case CustomRoles::FormattedFreeSpace:
		{
			const auto size = index.data (MassStorageRole::AvailableSize).toLongLong ();
			return tr ("available size: %1").arg (Util::MakePrettySize (size));
		}
		case CustomRoles::MountButtonIcon:
			return index.data (MassStorageRole::IsMounted).toBool () ?
					"image://ThemeIcons/emblem-unmounted"_qs :
					"image://ThemeIcons/emblem-mounted"_qs;
		case CustomRoles::MountedAt:
		{
			const auto& mounts = index.data (MassStorageRole::MountPoints).toStringList ();
			return mounts.isEmpty () ?
					QString {} :
					tr ("Mounted at %1").arg (mounts.join ("; "_qs));
		}
		case CustomRoles::UsedPercentage:
		{
			const auto free = index.data (MassStorageRole::AvailableSize).value<qint64> ();
			if (free < 0)
				return -1;

			const double total = index.data (MassStorageRole::TotalSize).value<qint64> ();
			return (1 - free / total) * 100;
		}
		default:
			return QSortFilterProxyModel::data (index, role);
		}
	}

	QHash<int, QByteArray> TrayProxyModel::roleNames () const
	{
		static const QHash<int, QByteArray> names
		{
			{ MassStorageRole::VisibleName, "devName"_qba },
			{ MassStorageRole::DevFile, "devFile"_qba },
			{ MassStorageRole::IsRemovable, "isRemovable"_qba },
			{ MassStorageRole::IsPartition, "isPartition"_qba },
			{ MassStorageRole::IsMountable, "isMountable"_qba },
			{ CommonDevRole::DevID, "devID"_qba },
			{ CommonDevRole::DevPersistentID, "devPersistentID"_qba },
			{ MassStorageRole::AvailableSize, "availableSize"_qba },
			{ MassStorageRole::TotalSize, "totalSize"_qba },
			{ CustomRoles::FormattedTotalSize, "formattedTotalSize"_qba },
			{ CustomRoles::FormattedFreeSpace, "formattedFreeSpace"_qba },
			{ CustomRoles::UsedPercentage, "usedPercentage"_qba },
			{ CustomRoles::MountButtonIcon, "mountButtonIcon"_qba },
			{ CustomRoles::ToggleHiddenIcon, "toggleHiddenIcon"_qba },
			{ CustomRoles::MountedAt, "mountedAt"_qba },
		};
		return names;
	}

	void TrayProxyModel::ToggleHidden (const QString& id)
	{
		if (!Hidden_.remove (id))
			Hidden_ << id;

		auto settings = MakeSettings ();
		settings.beginGroup ("HiddenDevices");
		settings.setValue ("List", QStringList { Hidden_.values () });
		settings.endGroup ();

		if (FilterEnabled_)
			invalidateFilter ();
		else
		{
			for (int i = 0; i < rowCount (); ++i)
			{
				const auto& idx = sourceModel ()->index (i, 0);
				if (id != idx.data (CommonDevRole::DevPersistentID).toString ())
					continue;

				const auto& mapped = mapFromSource (idx);
				emit dataChanged (mapped, mapped);
			}
		}

		if (Hidden_.isEmpty ())
		{
			FilterEnabled_ = true;
			invalidateFilter ();
		}
	}

	int TrayProxyModel::GetHiddenCount () const
	{
		return Hidden_.size ();
	}

	void TrayProxyModel::ToggleFilter ()
	{
		FilterEnabled_ = !FilterEnabled_;
		invalidateFilter ();
	}

	bool TrayProxyModel::filterAcceptsRow (int row, const QModelIndex&) const
	{
		const auto& idx = sourceModel ()->index (row, 0);
		if (!idx.data (MassStorageRole::IsMountable).toBool ())
			return false;

		if (!FilterEnabled_)
			return true;

		const auto& id = idx.data (CommonDevRole::DevPersistentID).toString ();
		return !Hidden_.contains (id);
	}
}
