/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clmodel.h"
#include <algorithm>
#include <QMimeData>
#include <util/gui/asyncdialogs.h>
#include <util/sll/qtutil.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "components/util/dnd.h"
#include "components/util/gui.h"
#include "cltooltipmanager.h"
#include "roles.h"

namespace LC::Azoth
{
	CLModel::CLModel (CLTooltipManager *manager,
			const EntriesManager& entries, ChatTabsManager& tabs, TransferJobManager& transfers, QObject *parent)
	: QStandardItemModel { parent }
	, TooltipManager_ { manager }
	, Entries_ { entries }
	, Tabs_ { tabs }
	, Transfers_ { transfers }
	{
	}

	QVariant CLModel::data (const QModelIndex& index, int role) const
	{
		if (role == Qt::ToolTipRole)
			if (const auto entry = index.data (CLRIEntry).value<ICLEntry*> ())
				return TooltipManager_->GetTooltip (entry);

		return QStandardItemModel::data (index, role);
	}

	QStringList CLModel::mimeTypes () const
	{
		return { DndUtil::GetFormatId (), "text/uri-list"_qs, "text/plain"_qs };
	}

	QMimeData* CLModel::mimeData (const QModelIndexList& indexes) const
	{
		QModelIndexList contactRows;
		const auto addContactRow = [&] (const QModelIndex& row)
		{
			if (!contactRows.contains (row))
				contactRows << row;
		};
		for (const auto& index : indexes)
			switch (index.data (CLREntryType).value<CLEntryType> ())
			{
			case CLETContact:
				addContactRow (index);
				break;
			case CLETCategory:
				for (int row = 0; row < rowCount (index); ++row)
					addContactRow (this->index (row, 0, index));
				break;
			case CLETAccount:
				break;
			}

		QList<DndUtil::MimeContactInfo> entries;
		for (const auto& row : contactRows)
			if (const auto entry = row.data (CLRIEntry).value<ICLEntry*> ())
				entries.append ({ entry, row.parent ().data (CLREntryCategory).toString () });

		if (entries.isEmpty ())
			return nullptr;

		const auto result = new QMimeData;
		DndUtil::Encode (entries, *result);
		return result;
	}

	namespace
	{
		bool IsCategory (const QModelIndex& index)
		{
			return index.data (CLREntryType).value<CLEntryType> () == CLETCategory;
		}

		QModelIndex GetTargetCategory (const QAbstractItemModel& model, int row, const QModelIndex& parent)
		{
			if (IsCategory (parent))
				return parent;

			const auto& child = model.index (row, 0, parent);
			return IsCategory (child) ? child : QModelIndex {};
		}

		struct GroupsChange
		{
			Util::TrackingPointer<ICLEntry> Entry_;
			QStringList NewGroups_;
		};

		QList<GroupsChange> PlanRegroup (const EntriesManager& entries, const QMimeData *mime, const QModelIndex& category)
		{
			if (!category.isValid ())
				return {};

			const auto account = category.data (CLRAccountObject).value<IAccount*> ();
			const auto& newGroup = category.data (CLREntryCategory).toString ();

			QList<GroupsChange> result;
			for (const auto& [entry, oldGroup] : DndUtil::DecodeMimeInfos (entries, mime))
			{
				if (entry->GetParentAccount () != account ||
						!(entry->GetEntryFeatures () & ICLEntry::FSupportsGrouping))
					continue;

				auto pos = std::ranges::find (result, entry, &GroupsChange::Entry_);
				if (pos == result.end ())
					pos = result.insert (pos, { entry, entry->Groups () });

				auto& newGroups = pos->NewGroups_;
				if (oldGroup && oldGroup != newGroup)
					newGroups.removeAll (*oldGroup);
				if (!newGroups.contains (newGroup))
					newGroups << newGroup;
			}

			result.removeIf ([] (const GroupsChange& change) { return change.NewGroups_ == change.Entry_->Groups (); });
			return result;
		}

		Util::Task<void> Regroup (QList<GroupsChange> changes, QString group)
		{
			if (changes.size () > 1)
			{
				const auto& question = CLModel::tr ("Do you want to move %n contacts to the group %1?", nullptr, changes.size ())
						.arg (group);
				if (co_await Util::Dialogs::Question (GetDialogParent (), CLModel::tr ("Move contacts"), question) != QMessageBox::Yes)
					co_return;
			}

			for (const auto& [entry, newGroups] : changes)
				if (entry)
					entry->SetGroups (newGroups);
		}
	}

	bool CLModel::canDropMimeData (const QMimeData *mime,
			Qt::DropAction action, int row, int column, const QModelIndex& parent) const
	{
		if (!QStandardItemModel::canDropMimeData (mime, action, row, column, parent))
			return false;

		if (!DndUtil::HasContacts (mime))
			return true;

		if (parent.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return !PlanRegroup (Entries_, mime, GetTargetCategory (*this, row, parent)).isEmpty ();

		const auto target = parent.data (CLRIEntry).value<ICLEntry*> ();
		return target && DndUtil::CanDropContacts (Entries_, *mime, *target);
	}

	bool CLModel::dropMimeData (const QMimeData *mime,
			Qt::DropAction action, int row, int, const QModelIndex& parent)
	{
		if (action == Qt::IgnoreAction)
			return true;

		return TryDropOntoContact (mime, row, parent) || TryDropRegroup (mime, row, parent);
	}

	Qt::DropActions CLModel::supportedDropActions () const
	{
		return Qt::CopyAction;
	}

	bool CLModel::TryDropOntoContact (const QMimeData *mime, int, const QModelIndex& parent)
	{
		if (parent.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return false;

		return DndUtil::HandleDroppedOnto (*mime, { parent.data (CLRIEntry).value<ICLEntry*> () },
				{
					.Entries_ = Entries_,
					.Tabs_ = Tabs_,
					.Transfers_ = Transfers_,
					.Parent_ = GetDialogParent (),
				});
	}

	bool CLModel::TryDropRegroup (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		const auto& category = GetTargetCategory (*this, row, parent);
		const auto& changes = PlanRegroup (Entries_, mime, category);
		if (changes.isEmpty ())
			return false;

		Regroup (changes, category.data (CLREntryCategory).toString ());
		return true;
	}
}
