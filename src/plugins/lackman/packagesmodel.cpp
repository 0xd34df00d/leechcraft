/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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
#include "core.h"
#include "storage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			PackagesModel::PackagesModel (QObject *parent)
			: QAbstractItemModel (parent)
			{
			}

			int PackagesModel::columnCount (const QModelIndex&) const
			{
				return 1;
			}

			QVariant PackagesModel::data (const QModelIndex& index, int role) const
			{
				if (index.column () != 0)
					return QVariant ();

				ListPackageInfo lpi = Packages_.at (index.row ());

				switch (role)
				{
				case Qt::DisplayRole:
					return lpi.Name_;
				case Qt::DecorationRole:
					return Core::Instance ().GetIconForLPI (lpi);
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
					const QList<Image>& images = Core::Instance ()
							.GetStorage ()->GetImages (lpi.Name_);
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

			Qt::ItemFlags PackagesModel::flags (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;
				else
					return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
}
