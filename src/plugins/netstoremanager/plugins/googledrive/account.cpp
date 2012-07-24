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
		if (TrashedItemIds_.contains (itemId))
		{
			auto res = QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
					tr ("Remove item"),
					tr ("Are you sure to delete permanently an entry <b>%1</b>?"
							"<br><i>Note: If entry is directory all files in it will <b>be</b> deleted.</i>")
							.arg (Items_ [itemId].Name_),
					QMessageBox::Ok | QMessageBox::Cancel);
			if (res == QMessageBox::Ok)
				DriveManager_->RemoveEntry (itemId);
		}
		else
			DriveManager_->MoveEntryToTrash (itemId);
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
		return ListingOp::Delete;
	}

	void Account::Prolongate (const QList<QStringList>&)
	{
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
		QList<QStandardItem*> CreateNewItem (const DriveItem& item, QStandardItem *parent,
				QHash<QString, QStandardItem*>& dirs)
		{
			QList<QStandardItem*> row;
			row << new QStandardItem (item.Name_);
			row [0]->setData (item.DownloadUrl_, ListingRole::URL);
			row [0]->setData (item.Id_, ListingRole::ID);

			if (!item.IsFolder_)
			{
				row << new QStandardItem (item.OwnerNames_.join (", "));
				row << new QStandardItem (QObject::tr ("%1 (by %2)")
						.arg (item.ModifiedDate_.toString (Qt::SystemLocaleDate),
								item.LastModifiedBy_));
			}
			else
			{
				row [0]->setIcon (Core::Instance ().GetProxy ()->GetIcon ("folder"));
				dirs [item.Id_] = row [0];
			}

			if (parent)
			{
				parent->appendRow (row);
				for (auto item : row)
					item->setEditable (false);
			}

			return row;
		}

		void CreateItems (QList<DriveItem> items, QHash<QString, QStandardItem*>& dirs,
				const QStringList& trashedItems, QStandardItem *trash)
		{
			int i = 0;
			while (!items.isEmpty ())
			{
				if (items.count () == i)
					break;

				DriveItem item = items.at (i++);
				if (dirs.contains (item.ParentId_))
				{
					QStandardItem *parentItem = 0;
					if (!trash ||
							trashedItems.contains (item.ParentId_))
						parentItem = dirs [item.ParentId_];
					else
						parentItem = trash;

					CreateNewItem (item, parentItem, dirs);

					items.removeAt (i - 1);
					i = 0;
				}
			}
		}
	}

	void Account::handleFileList (const QList<DriveItem>& items)
	{
		QList<QList<QStandardItem*>> treeItems;

		QHash<QString, QStandardItem*> id2ItemDir;
		QList<DriveItem> othersItems;
		QList<DriveItem> trashedItems;

		TrashedItemIds_.clear ();

		Q_FOREACH (const DriveItem& item, items)
		{
			QList<QStandardItem*> row;
			Items_ [item.Id_] = item;
			if (item.Labels_ & DriveItem::ILRemoved)
			{
				trashedItems << item;
				TrashedItemIds_ << item.Id_;
				continue;
			}

			if (item.ParentIsRoot_)
				treeItems << CreateNewItem (item, 0, id2ItemDir);
			else
				othersItems << item;
		}

		CreateItems (othersItems, id2ItemDir, QStringList (), 0);

		QList<QStandardItem*> row;
		row << new QStandardItem (Core::Instance ()
				.GetProxy ()->GetIcon ("user-trash"), tr ("Trash"));
		row [0]->setData ("google.drive-trash", ListingRole::ID);

		treeItems << row;
		CreateItems (trashedItems, id2ItemDir, TrashedItemIds_, row [0]);
		for (auto row : treeItems)
			for (auto item : row)
				item->setEditable (false);

		emit gotListing (QList<QList<QStandardItem*>> ());
		emit gotListing (treeItems);
	}

}
}
}
