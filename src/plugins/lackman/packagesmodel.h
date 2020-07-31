/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include "repoinfo.h"

namespace LC
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
	public slots:
		void handlePackageInstallRemoveToggled (int);
		void handlePackageUpdateToggled (int);
	};
}
}
