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
	: QAbstractItemModel (parent)
	, Source_ (0)
	{
		QHash<int, QByteArray> names;
		names [DeviceRoles::VisibleName] = "devName";
		names [DeviceRoles::DevFile] = "devFile";
		names [DeviceRoles::IsRemovable] = "isRemovable";
		names [DeviceRoles::IsPartition] = "isPartition";
		names [DeviceRoles::IsMountable] = "isMountable";
		names [DeviceRoles::DevID] = "devID";
		names [CustomRoles::FormattedTotalSize] = "formattedTotalSize";
		names [CustomRoles::MountButtonIcon] = "mountButtonIcon";
		names [CustomRoles::MountedAt] = "mountedAt";
		setRoleNames (names);
	}

	QModelIndex FlatMountableItems::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid ())
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex FlatMountableItems::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int FlatMountableItems::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : SourceIndexes_.size ();
	}

	int FlatMountableItems::columnCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : 1;
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
		case CustomRoles::MountButtonIcon:
			return index.data (DeviceRoles::IsMounted).toBool () ?
					"image://mountIcons/emblem-unmounted" :
					"image://mountIcons/emblem-mounted";
		case CustomRoles::MountedAt:
		{
			const auto& mounts = index.data (DeviceRoles::MountPoints).toStringList ();
			return mounts.isEmpty () ?
					QVariant () :
					tr ("Mounted at %1").arg (mounts.join ("; "));
		}
		default:
			return SourceIndexes_.value (index.row ()).data (role);
		}
	}

	void FlatMountableItems::SetSource (QAbstractItemModel *model)
	{
		if (Source_)
		{
			disconnect (Source_,
					SIGNAL (rowsInserted (QModelIndex, int, int)),
					this,
					SLOT (handleRowsInserted (QModelIndex, int, int)));
			disconnect (Source_,
					SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
					this,
					SLOT (handleRowsAboutRemoved (QModelIndex, int, int)));
		}

		SourceIndexes_.clear ();
		Source_ = model;
		reset ();
		connect (Source_,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));
		connect (Source_,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
				this,
				SLOT (handleRowsAboutRemoved (QModelIndex, int, int)));
	}

	void FlatMountableItems::handleDataChanged (const QModelIndex& top, const QModelIndex& bottom)
	{
		const auto& parent = top.parent ();
		for (int i = top.row (); i <= bottom.row (); ++i)
		{
			const auto& child = Source_->index (i, 0, parent);
			const int pos = SourceIndexes_.indexOf (child);
			if (pos < 0)
				continue;

			const auto& ourIdx = index (pos, 0);
			emit dataChanged (ourIdx, ourIdx);
		}
	}

	void FlatMountableItems::handleRowsInserted (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const auto& child = Source_->index (i, 0, parent);
			if (!child.data (DeviceRoles::IsMountable).toBool ())
				continue;

			beginInsertRows (QModelIndex (), SourceIndexes_.size (), SourceIndexes_.size ());
			SourceIndexes_ << child;
			endInsertRows ();

			if (int rc = Source_->rowCount (child))
				handleRowsInserted (child, 0, rc - 1);
		}
	}

	void FlatMountableItems::handleRowsAboutRemoved (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const auto& child = Source_->index (i, 0, parent);
			const int pos = SourceIndexes_.indexOf (child);

			if (pos >= 0)
			{
				beginRemoveRows (QModelIndex (), pos, pos);
				SourceIndexes_.removeAt (pos);
				endRemoveRows ();
			}

			if (int rc = Source_->rowCount (child))
				handleRowsAboutRemoved (child, 0, rc - 1);
		}
	}
}
}
