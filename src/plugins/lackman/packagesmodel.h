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

#ifndef PLUGINS_LACKMAN_PACKAGESMODEL_H
#define PLUGINS_LACKMAN_PACKAGESMODEL_H
#include <QAbstractItemModel>
#include "repoinfo.h"

namespace LeechCraft
{
	namespace Plugins
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
					PMRUpgradable
				};
				PackagesModel (QObject* = 0);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				void AddRow (const ListPackageInfo&);
				void Clear ();
			};
		}
	}
}

#endif
