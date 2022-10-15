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
#include <QFileInfo>
#include <QMessageBox>
#include <util/xpc/defaulthookproxy.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "components/dialogs/mucinvitedialog.h"
#include "core.h"
#include "transferjobmanager.h"
#include "dndutil.h"
#include "cltooltipmanager.h"
#include "roles.h"

namespace LC
{
namespace Azoth
{
	CLModel::CLModel (CLTooltipManager *manager, QObject *parent)
	: QStandardItemModel { parent }
	, TooltipManager_ { manager }
	{
		connect (manager,
				SIGNAL (rebuiltTooltip ()),
				this,
				SIGNAL (rebuiltTooltip ()));
	}

	QVariant CLModel::data (const QModelIndex& index, int role) const
	{
		CheckRequestUpdateTooltip (index, role);
		return QStandardItemModel::data (index, role);
	}

	QStringList CLModel::mimeTypes () const
	{
		return { DndUtil::GetFormatId (), "text/uri-list", "text/plain" };
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

			auto entryObj = index.data (CLREntryObject).value<QObject*> ();
			auto entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
				continue;

			const auto& thisGroup = index.parent ().data (CLREntryCategory).toString ();

			entries.append ({ entry, thisGroup });
			names << entry->GetEntryName ();
			urls << QUrl (entry->GetHumanReadableID ());
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

		if (PerformHooks (mime, row, parent))
			return true;

		if (TryInvite (mime, row, parent))
			return true;

		if (TryDropContact (mime, row, parent) ||
				TryDropContact (mime, parent.row (), parent.parent ()))
			return true;

		if (TryDropFile (mime, parent))
			return true;

		return false;
	}

	Qt::DropActions CLModel::supportedDropActions () const
	{
		return static_cast<Qt::DropActions> (Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
	}

	void CLModel::CheckRequestUpdateTooltip (const QModelIndex& index, int role) const
	{
		if (role != Qt::ToolTipRole)
			return;

		if (index.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return;

		const auto entryObj = index.data (CLREntryObject).value<QObject*> ();
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
			return;

		TooltipManager_->RebuildTooltip (entry);
	}

	bool CLModel::PerformHooks (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (CheckHookDnDEntry2Entry (mime, row, parent))
			return true;

		return false;
	}

	bool CLModel::CheckHookDnDEntry2Entry (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (row != -1 ||
				!DndUtil::HasContacts (mime) ||
				parent.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return false;

		const auto source = DndUtil::DecodeEntryObj (mime);
		if (!source)
			return false;

		QObject *target = parent.data (CLREntryObject).value<QObject*> ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookDnDEntry2Entry (proxy, source, target);
		return proxy->IsCancelled ();
	}

	bool CLModel::TryInvite (const QMimeData *mime, int, const QModelIndex& parent)
	{
		if (!DndUtil::HasContacts (mime))
			return false;

		if (parent.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return false;

		const auto targetObj = parent.data (CLREntryObject).value<QObject*> ();
		const auto targetEntry = qobject_cast<ICLEntry*> (targetObj);
		const auto targetMuc = qobject_cast<IMUCEntry*> (targetObj);

		bool accepted = false;

		for (const auto& serializedObj : DndUtil::DecodeEntryObjs (mime))
		{
			const auto serializedEntry = qobject_cast<ICLEntry*> (serializedObj);
			if (!serializedEntry)
				continue;

			const auto serializedMuc = qobject_cast<IMUCEntry*> (serializedObj);
			if (static_cast<bool> (targetMuc) == static_cast<bool> (serializedMuc))
				continue;

			auto muc = serializedMuc ? serializedMuc : targetMuc;
			auto entry = serializedMuc ? targetEntry : serializedEntry;

			MUCInviteDialog dia (entry->GetParentAccount ());
			dia.SetID (entry->GetHumanReadableID ());
			if (dia.exec () == QDialog::Accepted)
				muc->InviteToMUC (dia.GetID (), dia.GetInviteMessage ());

			accepted = true;
		}

		return accepted;
	}

	bool CLModel::TryDropContact (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (!DndUtil::HasContacts (mime))
			return false;

		if (parent.data (CLREntryType).value<CLEntryType> () != CLETAccount)
			return false;

		const auto acc = parent.data (CLRAccountObject).value<IAccount*> ();
		if (!acc)
			return false;

		const auto& newGrp = index (row, 0, parent).data (CLREntryCategory).toString ();

		for (const auto& info : DndUtil::DecodeMimeInfos (mime))
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

	bool CLModel::TryDropFile (const QMimeData* mime, const QModelIndex& parent)
	{
		// If MIME has CLEntryFormat, it's another serialized entry, we probably
		// don't want to send it.
		if (DndUtil::HasContacts (mime))
			return false;

		if (parent.data (CLREntryType).value<CLEntryType> () != CLETContact)
			return false;

		QObject *entryObj = parent.data (CLREntryObject).value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		const auto& urls = mime->urls ();
		if (urls.isEmpty ())
			return false;

		return Core::Instance ().GetTransferJobManager ()->OfferURLs (entry, urls);
	}
}
}
