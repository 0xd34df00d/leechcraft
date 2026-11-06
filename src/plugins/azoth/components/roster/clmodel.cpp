/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clmodel.h"
#include <QMimeData>
#include <QUrl>
#include <util/sll/qtutil.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "components/transfers/transferjobmanager.h"
#include "components/util/dnd.h"
#include "core.h"
#include "cltooltipmanager.h"
#include "roles.h"

namespace LC::Azoth
{
	CLModel::CLModel (CLTooltipManager *manager, const EntriesManager& entries, QObject *parent)
	: QStandardItemModel { parent }
	, TooltipManager_ { manager }
	, Entries_ { entries }
	{
		connect (manager,
				&CLTooltipManager::rebuiltTooltip,
				this,
				&CLModel::rebuiltTooltip);
	}

	QVariant CLModel::data (const QModelIndex& index, int role) const
	{
		CheckRequestUpdateTooltip (index, role);
		return QStandardItemModel::data (index, role);
	}

	QStringList CLModel::mimeTypes () const
	{
		return { DndUtil::GetFormatId (), "text/uri-list"_qs, "text/plain"_qs };
	}

	QMimeData* CLModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *result = new QMimeData;

		QStringList names;
		QList<QUrl> urls;
		QList<DndUtil::MimeContactInfo> entries;

		for (const auto& index : indexes)
		{
			if (index.data (CLREntryType).value<CLEntryType> () != CLETContact)
				continue;

			const auto entry = index.data (CLRIEntry).value<ICLEntry*> ();
			if (!entry)
				continue;

			const auto& thisGroup = index.parent ().data (CLREntryCategory).toString ();

			entries.append ({ entry, thisGroup });
			names << entry->GetEntryName ();
			urls << QUrl (entry->GetHumanReadableId ());		// TODO add a QUrl-shaped getter to ICLEntry?
		}

		DndUtil::Encode (entries, result);
		result->setText (names.join ("; "));
		result->setUrls (urls);

		return result;
	}

	bool CLModel::dropMimeData (const QMimeData *mime,
			Qt::DropAction action, int row, int, const QModelIndex& parent)
	{
		if (action == Qt::IgnoreAction)
			return true;

		if (TryDropOntoContact (mime, row, parent))
			return true;

		if (TryDropRegroup (mime, row, parent) ||
				TryDropRegroup (mime, parent.row (), parent.parent ()))
			return true;

		return false;
	}

	Qt::DropActions CLModel::supportedDropActions () const
	{
		return Qt::CopyAction | Qt::LinkAction;
	}

	void CLModel::CheckRequestUpdateTooltip (const QModelIndex& index, int role) const
	{
		if (role != Qt::ToolTipRole)
			return;

		if (index.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return;

		if (const auto entry = index.data (CLRIEntry).value<ICLEntry*> ())
			TooltipManager_->RebuildTooltip (entry);
	}

	bool CLModel::TryDropOntoContact (const QMimeData *mime, int, const QModelIndex& parent)
	{
		if (parent.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return false;

		return DndUtil::HandleDroppedOnto (*mime, { parent.data (CLRIEntry).value<ICLEntry*> () },
				{
					.Entries_ = Entries_,
					.Tabs_ = *Core::Instance ().GetChatTabsManager (),
					.Transfers_ = *Core::Instance ().GetTransferJobManager (),
					.Parent_ = nullptr,
				});
	}

	bool CLModel::TryDropRegroup (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (!DndUtil::HasContacts (mime))
			return false;

		if (parent.data (CLREntryType).value<CLEntryType> () != CLETAccount)
			return false;

		const auto acc = parent.data (CLRAccountObject).value<IAccount*> ();
		if (!acc)
			return false;

		const auto& newGrp = index (row, 0, parent).data (CLREntryCategory).toString ();

		for (const auto& info : DndUtil::DecodeMimeInfos (Entries_, mime))
		{
			const auto entry = info.Entry_;
			const auto& oldGroup = info.Group_;

			if (oldGroup == newGrp)
				continue;

			auto groups = entry->Groups ();
			groups.removeAll (oldGroup);
			groups << newGrp;

			entry->SetGroups (groups);
		}

		return true;
	}
}
