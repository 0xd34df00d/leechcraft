/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "packagesmodel.h"
#include <algorithm>
#include <QIcon>
#include <util/models/modelsanitizer.h>
#include <util/sll/visitor.h>
#include <util/util.h>
#include "core.h"
#include "storage.h"
#include "pendingmanager.h"

namespace LC
{
namespace LackMan
{
	PackagesModel::PackagesModel (PendingManager& pm, QObject *parent)
	: QAbstractTableModel { parent }
	, PendingManager_ { pm }
	{
		Util::InstallModelSanitizer (*this);
	}

	int PackagesModel::columnCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Columns::MaxColumn;
	}

	QVariant PackagesModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Vertical || role != Qt::DisplayRole)
			return {};

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
			return {};
		}
	}

	QVariant PackagesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

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
			switch (col)
			{
			case Columns::Inst:
				return IsInstallChecked (lpi) ? Qt::Checked : Qt::Unchecked;
			case Columns::Upd:
				if (!lpi.HasNewVersion_)
					return {};
				return IsUpdateChecked (lpi) ? Qt::Checked : Qt::Unchecked;
			default:
				return {};
			}
		case PMRPackageID:
			return lpi.PackageID_;
		case PMRName:
			return lpi.Name_;
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
			QStringList result;
			for (const auto& img : Core::Instance ().GetStorage ()->GetImages (lpi.Name_))
				if (img.Type_ == (role == PMRThumbnails ? Image::TThumbnail : Image::TScreenshot))
					result << img.URL_;
			return result;
		}
		case PMRSize:
			return Core::Instance ().GetStorage ()->GetPackageSize (lpi.PackageID_);
		default:
			return {};
		}
	}

	bool PackagesModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (role != Qt::CheckStateRole || !(flags (index) & Qt::ItemIsUserCheckable))
			return false;

		const auto id = Packages_.at (index.row ()).PackageID_;
		const bool checked = value.value<Qt::CheckState> () == Qt::Checked;
		const auto result = index.column () == Columns::Inst ?
				SetInstallChecked (id, checked) :
				SetUpdateChecked (id, checked);
		if (const auto error = result.MaybeLeft ())
			emit markingFailed (*error);
		return result.IsRight ();
	}

	Qt::ItemFlags PackagesModel::flags (const QModelIndex& index) const
	{
		auto flags = QAbstractTableModel::flags (index);
		if (!index.isValid ())
			return flags;

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

	std::optional<ListPackageInfo> PackagesModel::FindPackage (const QString& name) const
	{
		const auto pos = std::find_if (Packages_.begin (), Packages_.end (),
				[&name] (const auto& lpi) { return lpi.Name_ == name; });
		if (pos == Packages_.end ())
			return {};
		return *pos;
	}

	std::optional<ListPackageInfo> PackagesModel::FindPackage (int packageId) const
	{
		const auto row = GetRow (packageId);
		if (row == -1)
			return {};
		return Packages_.at (row);
	}

	int PackagesModel::GetRow (int packageId) const
	{
		for (int i = 0, size = Packages_.size (); i < size; ++i)
			if (Packages_.at (i).PackageID_ == packageId)
				return i;

		return -1;
	}

	MarkingResult_t PackagesModel::SetInstallChecked (int packageId, bool checked)
	{
		const auto row = GetRow (packageId);
		if (row == -1)
			return Util::Left { UnknownPackage { packageId } };

		const auto& lpi = Packages_.at (row);
		if (IsInstallChecked (lpi) == checked)
			return Util::Void {};

		return NotifyIfApplied (index (row, Columns::Inst),
				PendingManager_.ToggleInstallRemove (packageId, checked != lpi.IsInstalled_, lpi.IsInstalled_));
	}

	MarkingResult_t PackagesModel::SetUpdateChecked (int packageId, bool checked)
	{
		const auto row = GetRow (packageId);
		if (row == -1)
			return Util::Left { UnknownPackage { packageId } };

		const auto& lpi = Packages_.at (row);
		if (!lpi.HasNewVersion_)
			return Util::Left { NotUpgradable { lpi.Name_ } };
		if (IsUpdateChecked (lpi) == checked)
			return Util::Void {};

		return NotifyIfApplied (index (row, Columns::Upd), PendingManager_.ToggleUpdate (packageId, checked));
	}

	void PackagesModel::ResetPending ()
	{
		PendingManager_.Reset ();
		if (!Packages_.isEmpty ())
			emit dataChanged (index (0, Columns::Inst), index (Packages_.size () - 1, Columns::Upd), { Qt::CheckStateRole });
	}

	bool PackagesModel::IsInstallChecked (const ListPackageInfo& lpi) const
	{
		return lpi.IsInstalled_ ?
				!PendingManager_.GetPendingRemove ().contains (lpi.PackageID_) :
				PendingManager_.GetPendingInstall ().contains (lpi.PackageID_);
	}

	bool PackagesModel::IsUpdateChecked (const ListPackageInfo& lpi) const
	{
		return PendingManager_.GetPendingUpdate ().contains (lpi.PackageID_);
	}

	MarkingResult_t PackagesModel::NotifyIfApplied (const QModelIndex& cell, const ToggleResult_t& result)
	{
		if (const auto error = result.MaybeLeft ())
			return Util::Left { Util::Visit (*error, [] (const auto& e) -> MarkingError_t { return e; }) };

		emit dataChanged (cell, cell, { Qt::CheckStateRole });
		return Util::Void {};
	}
}
}
