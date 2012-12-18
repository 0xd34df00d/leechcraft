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

#include "packagesmodel.h"
#include <QIcon>
#include <QApplication>
#include <util/util.h>
#include "core.h"
#include "storage.h"
#include "pendingmanager.h"

namespace LeechCraft
{
namespace LackMan
{
	PackagesModel::PackagesModel (QObject *parent)
	: QAbstractItemModel (parent)
	{
	}

	int PackagesModel::columnCount (const QModelIndex&) const
	{
		return Columns::MaxColumn;
	}

	QVariant PackagesModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Vertical)
			return QVariant ();

		if (role != Qt::DisplayRole)
			return QVariant ();

		switch (section)
		{
		case Columns::Inst:
			return tr ("I");
		case Columns::Upd:
			return tr ("U");
		case Columns::Name:
			return tr ("Name");
		case Columns::Description:
			return tr ("Description");
		case Columns::Version:
			return tr ("Version");
		case Columns::Size:
			return tr ("Size");
		default:
			return "unknown";
		}
	}

	QVariant PackagesModel::data (const QModelIndex& index, int role) const
	{
		const auto& lpi = Packages_.at (index.row ());

		const int col = index.column ();

		switch (role)
		{
		case Qt::DisplayRole:
			switch (col)
			{
			case Columns::Name:
				return lpi.Name_;
			case Columns::Description:
				return lpi.ShortDescription_;
			case Columns::Version:
				return lpi.Version_;
			case Columns::Size:
			{
				const auto size = Core::Instance ().GetStorage ()->GetPackageSize (lpi.PackageID_);
				return size > 0 ? Util::MakePrettySize (size) : tr ("unknown");
			}
			default:
				return QVariant ();
			}
		case Qt::DecorationRole:
			return col != Columns::Name ?
					QVariant () :
					Core::Instance ().GetIconForLPI (lpi);
		case Qt::CheckStateRole:
		{
			auto pm = Core::Instance ().GetPendingManager ();
			switch (col)
			{
			case Columns::Inst:
				if (lpi.IsInstalled_)
					return pm->GetPendingRemove ().contains (lpi.PackageID_) ?
							Qt::Unchecked :
							Qt::Checked;
				else
					return pm->GetPendingInstall ().contains (lpi.PackageID_) ?
							Qt::Checked :
							Qt::Unchecked;
				break;
			case Columns::Upd:
				if (!lpi.HasNewVersion_)
					return QVariant ();
				return pm->GetPendingUpdate ().contains (lpi.PackageID_) ?
						Qt::Checked :
						Qt::Unchecked;
			default:
				return QVariant ();
			}
		}
		case PMRPackageID:
			return lpi.PackageID_;
		case PMRShortDescription:
			return lpi.ShortDescription_;
		case PMRLongDescription:
			return lpi.LongDescription_;
		case PMRTags:
			return lpi.Tags_;
		case PMRInstalled:
			return lpi.IsInstalled_;
		case PMRUpgradable:
			return lpi.HasNewVersion_;
		case PMRVersion:
			return lpi.Version_;
		case PMRThumbnails:
		case PMRScreenshots:
		{
			const auto& images = Core::Instance ().GetStorage ()->GetImages (lpi.Name_);
			QStringList result;
			Q_FOREACH (const Image& img, images)
				if (img.Type_ == (role == PMRThumbnails ? Image::TThumbnail : Image::TScreenshot))
					result << img.URL_;
			return result;
		}
		case PMRSize:
			return Core::Instance ().GetStorage ()->GetPackageSize (lpi.PackageID_);
		default:
			return QVariant ();
		}
	}

	bool PackagesModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (role != Qt::CheckStateRole)
			return false;

		const auto& lpi = Packages_.at (index.row ());

		const Qt::CheckState state = static_cast<Qt::CheckState> (value.toInt ());
		switch (index.column ())
		{
		case Columns::Inst:
		{
			const bool isNewState = (state == Qt::Checked && !lpi.IsInstalled_) ||
					(state == Qt::Unchecked && lpi.IsInstalled_);
			Core::Instance ().GetPendingManager ()->
					ToggleInstallRemove (lpi.PackageID_, isNewState, lpi.IsInstalled_);
			emit dataChanged (index, index);
			return true;
		}
		case Columns::Upd:
			Core::Instance ().GetPendingManager ()->ToggleUpdate (lpi.PackageID_, state == Qt::Checked);
			emit dataChanged (index, index);
			return true;
		default:
			return false;
		}
	}

	Qt::ItemFlags PackagesModel::flags (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return 0;

		auto flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		const auto& lpi = Packages_.at (index.row ());
		switch (index.column ())
		{
		case Columns::Inst:
			flags |= Qt::ItemIsUserCheckable;
			break;
		case Columns::Upd:
			if (lpi.HasNewVersion_)
				flags |= Qt::ItemIsUserCheckable;
			break;
		}
		return flags;
	}

	QModelIndex PackagesModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex PackagesModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int PackagesModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Packages_.size ();
	}

	void PackagesModel::AddRow (const ListPackageInfo& lpi)
	{
		int size = Packages_.size ();
		beginInsertRows (QModelIndex (), size, size);
		Packages_ << lpi;
		endInsertRows ();
	}

	void PackagesModel::UpdateRow (const ListPackageInfo& lpi)
	{
		for (int i = 0, size = Packages_.size ();
				i < size; ++i)
			if (Packages_.at (i).Name_ == lpi.Name_)
			{
				Packages_ [i] = lpi;
				emit dataChanged (index (i, 0),
						index (i, columnCount () - 1));
				break;
			}
	}

	void PackagesModel::RemovePackage (int packageId)
	{
		for (int i = 0, size = Packages_.size ();
				i < size; ++i)
			if (Packages_.at (i).PackageID_ == packageId)
			{
				beginRemoveRows (QModelIndex (), i, i);
				Packages_.removeAt (i);
				endRemoveRows ();
				break;
			}
	}

	ListPackageInfo PackagesModel::FindPackage (const QString& name) const
	{
		Q_FOREACH (const ListPackageInfo& lpi, Packages_)
			if (lpi.Name_ == name)
				return lpi;

		return ListPackageInfo ();
	}

	int PackagesModel::GetRow (int packageId) const
	{
		for (int i = 0, size = Packages_.size (); i < size; ++i)
			if (Packages_.at (i).PackageID_ == packageId)
				return i;

		return -1;
	}

	void PackagesModel::Clear ()
	{
		Packages_.clear ();
		reset ();
	}
}
}
