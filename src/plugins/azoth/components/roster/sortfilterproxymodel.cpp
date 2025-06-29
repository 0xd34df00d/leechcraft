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
#include "interfaces/azoth/imucentry.h"
#include "interfaces/azoth/imucperms.h"
#include "roles.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth
{
	SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
	: FixedStringFilterProxyModel { Qt::CaseInsensitive, parent }
	{
		setRecursiveFilteringEnabled (false);

		auto& xsm = XmlSettingsManager::Instance ();
		xsm.RegisterObject ("OrderByStatus", this,
				[this] (bool orderByStatus)
				{
					OrderByStatus_ = orderByStatus;
					invalidate ();
				});

		xsm.RegisterObject ("HideMUCPartsInWholeCL", this,
				[this] (bool hide)
				{
					HideMUCParts_ = hide;
					invalidate ();
				});

		xsm.RegisterObject ("ShowSelfContacts", this,
				[this] (bool showSelf)
				{
					ShowSelfContacts_ = showSelf;
					invalidate ();
				});

		xsm.RegisterObject ("HideErrorContactsWithOffline", this,
				[this] (bool hideErroring)
				{
					HideErroring_ = hideErroring;
					invalidate ();
				});
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
					&QObject::destroyed,
					this,
					&SortFilterProxyModel::handleMUCDestroyed);

		MUCEntry_ = qobject_cast<IMUCEntry*> (mucEntry) ? mucEntry : nullptr;
		if (MUCEntry_)
			connect (MUCEntry_,
					&QObject::destroyed,
					this,
					&SortFilterProxyModel::handleMUCDestroyed);

		invalidateFilter ();
	}

	void SortFilterProxyModel::showOfflineContacts (bool show)
	{
		ShowOffline_ = show;
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
		CLEntryType GetType (const QModelIndex& idx)
		{
			return idx.data (CLREntryType).value<CLEntryType> ();
		}

		ICLEntry* GetEntry (const QModelIndex& idx)
		{
			return qobject_cast<ICLEntry*> (idx.data (CLREntryObject).value<QObject*> ());
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
		if (leftType == CLETAccount)
			return FixedStringFilterProxyModel::lessThan (left, right);
		if (leftType == CLETCategory)
		{
			const bool leftIsMuc = left.data (CLRIsMUCCategory).toBool ();
			const bool rightIsMuc = right.data (CLRIsMUCCategory).toBool ();
			if (leftIsMuc == rightIsMuc)
				return FixedStringFilterProxyModel::lessThan (left, right);
			return rightIsMuc;
		}

		const auto lE = GetEntry (left);
		const auto rE = GetEntry (right);

		if (lE->GetEntryType () == ICLEntry::EntryType::PrivateChat &&
				rE->GetEntryType () == ICLEntry::EntryType::PrivateChat &&
				lE->GetParentCLEntry () == rE->GetParentCLEntry ())
			if (const auto lp = qobject_cast<IMUCPerms*> (lE->GetParentCLEntryObject ()))
			{
				const bool less = lp->IsLessByPerm (lE->GetQObject (), rE->GetQObject ());
				const bool more = lp->IsLessByPerm (rE->GetQObject (), lE->GetQObject ());
				if (less || more)
					return more;
			}

		const auto lState = lE->GetStatus ().State_;
		const auto rState = rE->GetStatus ().State_;
		if (lState == rState ||
				!OrderByStatus_)
			return lE->GetEntryName ().localeAwareCompare (rE->GetEntryName ()) < 0;

		return IsLess (lState, rState);
	}

	bool SortFilterProxyModel::FilterAcceptsMucMode (int row, const QModelIndex& parent) const
	{
		if (!MUCEntry_)
			return false;

		const auto& idx = sourceModel ()->index (row, 0, parent);
		switch (GetType (idx))
		{
		case CLETAccount:
		{
			const auto acc = qobject_cast<ICLEntry*> (MUCEntry_)->GetParentAccount ();
			return acc == idx.data (CLRAccountObject).value<IAccount*> ();
		}
		case CLETCategory:
		{
			const auto& gName = idx.data ().toString ();
			return gName == qobject_cast<IMUCEntry*> (MUCEntry_)->GetGroupName () ||
				   qobject_cast<ICLEntry*> (MUCEntry_)->Groups ().contains (gName);
		}
		default:
			return FixedStringFilterProxyModel::filterAcceptsRow (row, parent);
		}
	}

	bool SortFilterProxyModel::FilterAcceptsNonMucMode (int row, const QModelIndex& parent) const
	{
		const auto& idx = sourceModel ()->index (row, 0, parent);
		if (IsFilterSet ())
			return GetType (idx) != CLETContact || IsMatch (idx.data ().toString ());

		if (idx.data (CLRUnreadMsgCount).toInt ())
			return true;

		switch (GetType (idx))
		{
		case CLETContact:
		{
			const auto entry = GetEntry (idx);
			const auto state = entry->GetStatus ().State_;

			if (!ShowOffline_ && state == SOffline)
				return false;

			if (!ShowOffline_ && state == SError && HideErroring_)
				return false;

			if (HideMUCParts_ &&
					entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
				return false;

			if (!ShowSelfContacts_ &&
					entry->GetEntryFeatures () & ICLEntry::FSelfContact)
				return false;

			break;
		}
		case CLETCategory:
			if (!ShowOffline_ &&
					!idx.data (CLRNumOnline).toInt ())
				return false;

			for (int subRow = 0; subRow < sourceModel ()->rowCount (idx); ++subRow)
				if (filterAcceptsRow (subRow, idx))
					return true;

			return false;
		case CLETAccount:
			return idx.data (CLRAccountObject).value<IAccount*> ()->IsShownInRoster ();
		}

		return FixedStringFilterProxyModel::filterAcceptsRow (row, parent);
	}
}
