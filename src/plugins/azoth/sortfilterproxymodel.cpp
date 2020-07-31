/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sortfilterproxymodel.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucperms.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
	SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel { parent }
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);

		XmlSettingsManager::Instance ().RegisterObject ("OrderByStatus",
				this, "handleStatusOrderingChanged");
		handleStatusOrderingChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("HideMUCPartsInWholeCL",
				this, "handleHideMUCPartsChanged");
		handleHideMUCPartsChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("ShowSelfContacts",
				this, "handleShowSelfContactsChanged");
		handleShowSelfContactsChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("HideErrorContactsWithOffline",
				this, "handleHideErrorContactsChanged");
		handleHideErrorContactsChanged ();
	}

	void SortFilterProxyModel::SetMUCMode (bool muc)
	{
		MUCMode_ = muc;
		invalidate ();

		if (muc)
		  emit mucMode ();
	}

	bool SortFilterProxyModel::IsMUCMode () const
	{
		return MUCMode_;
	}

	void SortFilterProxyModel::SetMUC (QObject *mucEntry)
	{
		if (MUCEntry_)
			disconnect (MUCEntry_,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleMUCDestroyed ()));

		MUCEntry_ = qobject_cast<IMUCEntry*> (mucEntry) ? mucEntry : 0;
		if (MUCEntry_)
			connect (MUCEntry_,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleMUCDestroyed ()));

		invalidateFilter ();
	}

	void SortFilterProxyModel::showOfflineContacts (bool show)
	{
		ShowOffline_ = show;
		invalidate ();
	}

	void SortFilterProxyModel::handleStatusOrderingChanged ()
	{
		OrderByStatus_ = XmlSettingsManager::Instance ().property ("OrderByStatus").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleHideMUCPartsChanged ()
	{
		HideMUCParts_ = XmlSettingsManager::Instance ().property ("HideMUCPartsInWholeCL").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleShowSelfContactsChanged ()
	{
		ShowSelfContacts_ = XmlSettingsManager::Instance ().property ("ShowSelfContacts").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleHideErrorContactsChanged ()
	{
		HideErroring_ = XmlSettingsManager::Instance ().property ("HideErrorContactsWithOffline").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleMUCDestroyed ()
	{
		SetMUC (nullptr);
		SetMUCMode (false);
		emit wholeMode ();
	}

	namespace
	{
		Core::CLEntryType GetType (const QModelIndex& idx)
		{
			return idx.data (Core::CLREntryType).value<Core::CLEntryType> ();
		}

		ICLEntry* GetEntry (const QModelIndex& idx)
		{
			return qobject_cast<ICLEntry*> (idx.data (Core::CLREntryObject).value<QObject*> ());
		}
	}

	bool SortFilterProxyModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		return MUCMode_ ?
				FilterAcceptsMucMode (row, parent) :
				FilterAcceptsNonMucMode (row, parent);
	}

	bool SortFilterProxyModel::lessThan (const QModelIndex& right,
			const QModelIndex& left) const			// sort in reverse order ok
	{
		const auto leftType = GetType (left);
		if (leftType == Core::CLETAccount)
			return QSortFilterProxyModel::lessThan (left, right);
		else if (leftType == Core::CLETCategory)
		{
			const bool leftIsMuc = left.data (Core::CLRIsMUCCategory).toBool ();
			const bool rightIsMuc = right.data (Core::CLRIsMUCCategory).toBool ();
			if (leftIsMuc == rightIsMuc)
				return QSortFilterProxyModel::lessThan (left, right);
			else
				return rightIsMuc;
		}

		const auto lE = GetEntry (left);
		const auto rE = GetEntry (right);

		if (lE->GetEntryType () == ICLEntry::EntryType::PrivateChat &&
				rE->GetEntryType () == ICLEntry::EntryType::PrivateChat &&
				lE->GetParentCLEntry () == rE->GetParentCLEntry ())
			if (const auto lp = qobject_cast<IMUCPerms*> (lE->GetParentCLEntryObject ()))
			{
				bool less = lp->IsLessByPerm (lE->GetQObject (), rE->GetQObject ());
				bool more = lp->IsLessByPerm (rE->GetQObject (), lE->GetQObject ());
				if (less || more)
					return more;
			}

		const auto lState = lE->GetStatus ().State_;
		const auto rState = rE->GetStatus ().State_;
		if (lState == rState ||
				!OrderByStatus_)
			return lE->GetEntryName ().localeAwareCompare (rE->GetEntryName ()) < 0;
		else
			return IsLess (lState, rState);
	}

	bool SortFilterProxyModel::FilterAcceptsMucMode (int row, const QModelIndex& parent) const
	{
		if (!MUCEntry_)
			return false;

		const auto& idx = sourceModel ()->index (row, 0, parent);
		switch (GetType (idx))
		{
		case Core::CLETAccount:
		{
			const auto acc = qobject_cast<ICLEntry*> (MUCEntry_)->GetParentAccount ();
			return acc == idx.data (Core::CLRAccountObject).value<IAccount*> ();
		}
		case Core::CLETCategory:
		{
			const QString& gName = idx.data ().toString ();
			return gName == qobject_cast<IMUCEntry*> (MUCEntry_)->GetGroupName () ||
				   qobject_cast<ICLEntry*> (MUCEntry_)->Groups ().contains (gName);
		}
		default:
			return QSortFilterProxyModel::filterAcceptsRow (row, parent);
		}
	}

	bool SortFilterProxyModel::FilterAcceptsNonMucMode (int row, const QModelIndex& parent) const
	{
		const auto& idx = sourceModel ()->index (row, 0, parent);
		if (!filterRegExp ().isEmpty ())
			return GetType (idx) == Core::CLETContact ?
					idx.data ().toString ().contains (filterRegExp ()) :
					true;

		if (idx.data (Core::CLRUnreadMsgCount).toInt ())
			return true;

		const auto type = GetType (idx);

		if (type == Core::CLETContact)
		{
			const auto entry = GetEntry (idx);
			const auto state = entry->GetStatus ().State_;

			if (!ShowOffline_ &&
					HideErroring_ &&
					state == SError)
				return false;

			if (!ShowOffline_ &&
					state == SOffline &&
					!idx.data (Core::CLRUnreadMsgCount).toInt ())
				return false;

			if (HideMUCParts_ &&
					entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
				return false;

			if (!ShowSelfContacts_ &&
					entry->GetEntryFeatures () & ICLEntry::FSelfContact)
				return false;
		}
		else if (type == Core::CLETCategory)
		{
			if (!ShowOffline_ &&
					!idx.data (Core::CLRNumOnline).toInt ())
				return false;

			for (int subRow = 0; subRow < sourceModel ()->rowCount (idx); ++subRow)
				if (filterAcceptsRow (subRow, idx))
					return true;

			return false;
		}
		else if (type == Core::CLETAccount)
			return idx.data (Core::CLRAccountObject).value<IAccount*> ()->IsShownInRoster ();

		return QSortFilterProxyModel::filterAcceptsRow (row, parent);
	}
}
}
