/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include <QAbstractTableModel>
#include "repoinfo.h"
#include "pendingmanager.h"

namespace LC
{
namespace LackMan
{
	struct UnknownPackage
	{
		int PackageId_;
	};

	struct NotUpgradable
	{
		QString Package_;
	};

	using MarkingError_t = std::variant<UnknownPackage, NotUpgradable, UnsatisfiedDependencies, ResolutionFailed>;
	using MarkingResult_t = Util::Either<MarkingError_t, Util::Void>;

	class PackagesModel : public QAbstractTableModel
	{
		Q_OBJECT

		PendingManager& PendingManager_;
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

		explicit PackagesModel (PendingManager&, QObject* = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		bool setData (const QModelIndex&, const QVariant&, int) override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = {}) const override;

		void AddRow (const ListPackageInfo&);
		void UpdateRow (const ListPackageInfo&);
		void RemovePackage (int);
		std::optional<ListPackageInfo> FindPackage (const QString&) const;
		std::optional<ListPackageInfo> FindPackage (int packageId) const;
		int GetRow (int packageId) const;

		/** For an installed package, unchecking marks it for removal.
		 */
		[[nodiscard]] MarkingResult_t SetInstallChecked (int packageId, bool checked);
		[[nodiscard]] MarkingResult_t SetUpdateChecked (int packageId, bool checked);
		void ResetPending ();
	private:
		bool IsInstallChecked (const ListPackageInfo&) const;
		bool IsUpdateChecked (const ListPackageInfo&) const;
		MarkingResult_t NotifyIfApplied (const QModelIndex& cell, const ToggleResult_t&);
	signals:
		void markingFailed (const MarkingError_t&);
	};
}
}
