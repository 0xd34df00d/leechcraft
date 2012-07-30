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

#include "account.h"
#include <QtDebug>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMainWindow>
#include "core.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	Account::Account (const QString& name, QObject *parentPlugin)
	: QObject (parentPlugin)
	, ParentPlugin_ (parentPlugin)
	, Name_ (name)
	, Trusted_ (false)
	, DriveManager_ (new DriveManager (this, this))
	{
		connect (DriveManager_,
				SIGNAL (gotFiles (const QList<DriveItem>&)),
				this,
				SLOT (handleFileList (const QList<DriveItem>&)));
	}

	QObject* Account::GetObject ()
	{
		return this;
	}

	QObject* Account::GetParentPlugin () const
	{
		return ParentPlugin_;
	}

	AccountFeatures Account::GetAccountFeatures () const
	{
		return FileListings;
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	void Account::Upload (const QString& filepath)
	{
	}

	void Account::Delete (const QList<QStringList>& id)
	{
		const QString& itemId = id [0] [0];
		auto res = QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
				tr ("Remove item"),
				tr ("Are you sure to delete permanently an entry <b>%1</b>?"
					"<br><i>Note: If entry is directory all files in it <b>will be</b> deleted.</i>")
							.arg (Items_ [itemId].Name_),
				QMessageBox::Ok | QMessageBox::Cancel);
		if (res == QMessageBox::Ok)
			DriveManager_->RemoveEntry (itemId);
	}

	QStringList Account::GetListingHeaders () const
	{
		QStringList result;
		result << tr ("Title");
		result << tr ("Owner");
		result << tr ("Last Modified");
		return result;
	}

	ListingOps Account::GetListingOps () const
	{
		return ListingOp::Delete | ListingOp::TrashSupporing;
	}

	void Account::Prolongate (const QList<QStringList>&)
	{
	}

	void Account::MoveToTrash (const QList<QStringList>& id)
	{
		DriveManager_->MoveEntryToTrash (id [0] [0]);
	}

	void Account::RefreshListing ()
	{
		DriveManager_->RefreshListing ();
	}

	QByteArray Account::Serialize ()
	{
		QByteArray result;
		QDataStream str (&result, QIODevice::WriteOnly);
		str << static_cast<quint8> (1)
				<< Name_
				<< Trusted_
				<< RefreshToken_;
		return result;
	}

	Account_ptr Account::Deserialize (const QByteArray& data, QObject* parentPlugin)
	{
		QDataStream str (data);
		quint8 version = 0;
		str >> version;

		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return Account_ptr ();
		}

		QString name;
		str >> name;
		Account_ptr acc (new Account (name, parentPlugin));
		str >> acc->Trusted_
				>> acc->RefreshToken_;
		return acc;
	}

	bool Account::IsTrusted () const
	{
		return Trusted_;
	}

	void Account::SetTrusted (bool trust)
	{
		Trusted_ = trust;
	}

	void Account::SetAccessToken (const QString& token)
	{
		AccessToken_ = token;
	}

	void Account::SetRefreshToken (const QString& token)
	{
		RefreshToken_ = token;
	}

	QString Account::GetRefreshToken () const
	{
		return RefreshToken_;
	}

	namespace
	{
		QList<QStandardItem*> CreateItem (QHash<QString,
				QList<QStandardItem*>>& id2Item, const DriveItem& item)
		{
			QList<QStandardItem*> row;
			if (!id2Item.contains (item.Id_))
			{
				row << new QStandardItem (item.Name_);
				row [0]->setData (item.Id_, ListingRole::ID);
				row [0]->setData (static_cast<bool> (item.Labels_ & DriveItem::ILRemoved),
						ListingRole::InTrash);

				if (!item.IsFolder_)
				{
					row << new QStandardItem (item.OwnerNames_.join (", "));
					row << new QStandardItem (QObject::tr ("%1 (by %2)")
							.arg (item.ModifiedDate_.toString ("dd.MM.yy hh:mm"),
									item.LastModifiedBy_));
				}
				else
				{
					row [0]->setIcon (Core::Instance ().GetProxy ()->GetIcon ("folder"));
					id2Item [item.Id_] = row;
				}
			}

			for (const auto& itm : row)
				itm->setEditable (false);

			return row;
		}

		void CreateChildItem (QHash<QString, QList<QStandardItem*>>& id2Item,
				const DriveItem& parentItem, const DriveItem& childItem)
		{
			QList<QStandardItem*> parentRow = !id2Item.contains (parentItem.Id_) ?
				CreateItem (id2Item, parentItem) :
				id2Item [parentItem.Id_];
			QList<QStandardItem*> childRow = !id2Item.contains (childItem.Id_) ?
				CreateItem (id2Item, childItem) :
				id2Item [childItem.Id_];
			parentRow [0]->appendRow (childRow);
		}
	}

	void Account::handleFileList (const QList<DriveItem>& items)
	{
		QList<QList<QStandardItem*>> treeItems;

		QHash<QString, DriveItem> trashedItems;
		QHash<QString, DriveItem> id2DriveItem_;
		QHash<QString, QList<QStandardItem*>> id2Item_;
		QHash<QString, QList<QStandardItem*>> trashedId2Item_;

		for (const auto& item : items)
		{
			if (item.Labels_ & DriveItem::ILRemoved)
			{
				trashedItems [item.Id_] = item;
				continue;
			}

			id2DriveItem_ [item.Id_] = item;
			if (item.ParentIsRoot_)
				treeItems << CreateItem (id2Item_, item);
		}

		const QStringList& keys = id2DriveItem_.keys ();
		const auto& values = id2DriveItem_.values ();
		for (const auto& item : values)
		{
			if (keys.contains (item.ParentId_))
				CreateChildItem (id2Item_, id2DriveItem_ [item.ParentId_], item);
			else
				CreateItem (id2Item_, item);
		}

		const QStringList& trashedKeys = trashedItems.keys ();
		const auto& trashedValues = trashedItems.values ();
		for (const auto& item : trashedValues)
			if (!trashedKeys.contains (item.ParentId_))
				treeItems << CreateItem (trashedId2Item_, item);

		for (const auto& item : trashedValues)
			if (trashedKeys.contains (item.ParentId_))
				CreateChildItem (trashedId2Item_, trashedItems [item.ParentId_], item);

		treeItems.removeAll (QList<QStandardItem*> ());

		emit gotListing (QList<QList<QStandardItem*>> ());
		emit gotListing (treeItems);
	}

}
}
}
