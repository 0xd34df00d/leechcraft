/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractTableModel>
#include "repoinfo.h"
#include "pendingmanager.h"

namespace LC
{
namespace LackMan
{
	class PackagesModel : public QAbstractTableModel
	{
		Q_OBJECT

		QList<ListPackageInfo> Packages_;
	public:
		enum PackageModelRole
		{
			PMRName = Qt::UserRole + 1,
			PMRShortDescription,
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

		explicit PackagesModel (QObject* = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		bool setData (const QModelIndex&, const QVariant&, int) override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;

		void AddRow (const ListPackageInfo&);
		void UpdateRow (const ListPackageInfo&);
		void RemovePackage (int);
		ListPackageInfo FindPackage (const QString&) const;
		int GetRow (int packageId) const;
		void Clear ();
	public slots:
		void handlePackageInstallRemoveToggled (int);
		void handlePackageUpdateToggled (int);
	signals:
		void markingFailed (const MarkingError_t&);
	};
}
}
