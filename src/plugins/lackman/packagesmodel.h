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

#ifndef PLUGINS_LACKMAN_PACKAGESMODEL_H
#define PLUGINS_LACKMAN_PACKAGESMODEL_H
#include <QAbstractItemModel>
#include "repoinfo.h"

namespace LeechCraft
{
namespace LackMan
{
	class PackagesModel : public QAbstractItemModel
	{
		Q_OBJECT

		QList<ListPackageInfo> Packages_;
	public:
		enum PackageModelRole
		{
			PMRShortDescription = Qt::UserRole + 1,
			PMRLongDescription,
			PMRTags,
			PMRType,
			PMRPackageID,
			PMRInstalled,
			PMRUpgradable,
			PMRVersion,
			PMRThumbnails,
			PMRScreenshots,
			PMRSize
		};
		enum Columns
		{
			Inst,
			Upd,
			Name,
			Description,
			Version,
			Size,
			MaxColumn
		};
		PackagesModel (QObject* = 0);

		int columnCount (const QModelIndex& = QModelIndex ()) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		bool setData (const QModelIndex&, const QVariant&, int);
		Qt::ItemFlags flags (const QModelIndex&) const;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex& = QModelIndex ()) const;

		void AddRow (const ListPackageInfo&);
		void UpdateRow (const ListPackageInfo&);
		void RemovePackage (int);
		ListPackageInfo FindPackage (const QString&) const;
		int GetRow (int packageId) const;
		void Clear ();
	};
}
}

#endif
