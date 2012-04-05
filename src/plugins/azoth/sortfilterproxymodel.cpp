/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "sortfilterproxymodel.h"
#include <QTimer>
#include "core.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucperms.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	SortFilterProxyModel::SortFilterProxyModel (QObject *parent)
	: QSortFilterProxyModel (parent)
	, ShowOffline_ (true)
	, MUCMode_ (false)
	, OrderByStatus_ (true)
	, HideMUCParts_ (false)
	, MUCEntry_ (0)
	{
		setDynamicSortFilter (true);
		setFilterCaseSensitivity (Qt::CaseInsensitive);

		XmlSettingsManager::Instance ().RegisterObject ("OrderByStatus",
				this, "handleStatusOrderingChanged");
		handleStatusOrderingChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("HideMUCPartsInWholeCL",
				this, "handleHideMUCPartsChanged");
		handleHideMUCPartsChanged ();
	}

	void SortFilterProxyModel::SetMUCMode (bool muc)
	{
		MUCMode_ = muc;
		invalidateFilter ();

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
		invalidateFilter ();
	}

	void SortFilterProxyModel::handleStatusOrderingChanged ()
	{
		OrderByStatus_ = XmlSettingsManager::Instance ()
				.property ("OrderByStatus").toBool ();
		QTimer::singleShot (0,
				this,
				SLOT (invalidate ()));
	}

	void SortFilterProxyModel::handleHideMUCPartsChanged ()
	{
		HideMUCParts_ = XmlSettingsManager::Instance ()
				.property ("HideMUCPartsInWholeCL").toBool ();
		invalidate ();
	}

	void SortFilterProxyModel::handleMUCDestroyed ()
	{
		SetMUC (0);
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
			return qobject_cast<ICLEntry*> (idx
						.data (Core::CLREntryObject).value<QObject*> ());
		}
	}

	bool SortFilterProxyModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		if (MUCMode_)
		{
			if (!MUCEntry_)
				return false;

			const QModelIndex& idx = sourceModel ()->index (row, 0, parent);
			switch (GetType (idx))
			{
			case Core::CLETAccount:
			{
				QObject *acc = qobject_cast<ICLEntry*> (MUCEntry_)->GetParentAccount ();
				return acc == idx.data (Core::CLRAccountObject).value<QObject*> ();
			}
			case Core::CLETCategory:
			{
				const QString& gName = idx.data ().toString ();
				return gName == qobject_cast<IMUCEntry*> (MUCEntry_)->GetGroupName () ||
						qobject_cast<ICLEntry*> (MUCEntry_)->Groups ().contains (gName);
			}
			default:
				break;
			}
		}
		else
		{
			const QModelIndex& idx = sourceModel ()->index (row, 0, parent);
			if (!filterRegExp ().isEmpty ())
				return GetType (idx) == Core::CLETContact ?
						idx.data ().toString ().contains (filterRegExp ()) :
						true;

			if (idx.data (Core::CLRUnreadMsgCount).toInt ())
				return true;

			if (GetType (idx) == Core::CLETContact)
			{
				ICLEntry *entry = GetEntry (idx);
				const State state = entry->GetStatus ().State_;

				if (!ShowOffline_ &&
						state == SOffline &&
						!idx.data (Core::CLRUnreadMsgCount).toInt ())
					return false;

				if (HideMUCParts_ &&
						entry->GetEntryType () == ICLEntry::ETPrivateChat)
					return false;
			}
			else if (GetType (idx) == Core::CLETCategory)
			{
				if (!sourceModel ()->rowCount (idx))
					return false;

				if (!ShowOffline_ &&
						!idx.data (Core::CLRNumOnline).toInt ())
					return false;
			}
		}

		return QSortFilterProxyModel::filterAcceptsRow (row, parent);
	}

	bool SortFilterProxyModel::lessThan (const QModelIndex& right,
			const QModelIndex& left) const			// sort in reverse order ok
	{
		if (GetType (left) != Core::CLETContact ||
				GetType (right) != Core::CLETContact)
			return QSortFilterProxyModel::lessThan (left, right);

		ICLEntry *lE = GetEntry (left);
		ICLEntry *rE = GetEntry (right);

		if (lE->GetEntryType () == ICLEntry::ETPrivateChat &&
				rE->GetEntryType () == ICLEntry::ETPrivateChat &&
				lE->GetParentCLEntry () == rE->GetParentCLEntry ())
		{
			IMUCPerms *lp = qobject_cast<IMUCPerms*> (lE->GetParentCLEntry ());
			if (lp)
			{
				bool less = lp->IsLessByPerm (lE->GetObject (), rE->GetObject ());
				bool more = lp->IsLessByPerm (rE->GetObject (), lE->GetObject ());
				if (less || more)
					return more;
			}
		}

		State lState = lE->GetStatus ().State_;
		State rState = rE->GetStatus ().State_;
		if (lState == rState ||
				!OrderByStatus_)
			return lE->GetEntryName ().localeAwareCompare (rE->GetEntryName ()) < 0;
		else
			return IsLess (lState, rState);
	}
}
}
