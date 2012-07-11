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
#include "core.h"
#include <QtDebug>
#include <QStandardItemModel>

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

	void Account::handleFileList (const QList<DriveItem>& items)
	{
		QList<QList<QStandardItem*>> treeItems;

		QHash<QString, QStandardItem*> Id2ItemDir;
		QList<QString> IdThrashed;
		QList<DriveItem> othersItems;
		QList<DriveItem> trashedItems;
		Q_FOREACH (const DriveItem& item, items)
		{
			QList<QStandardItem*> row;
			if (item.Labels_ & DriveItem::ILRemoved)
			{
				trashedItems << item;
				IdThrashed << item.Id_;
				continue;
			}

			if (item.ParentIsRoot_)
			{
				QStandardItem *dirItem = new QStandardItem (item.Name_);
				dirItem->setData (item.DownloadUrl_, ListingRole::URL);
				dirItem->setData (item.Id_, ListingRole::ID);
				row << dirItem;
				if (item.IsFolder_)
				{
					Id2ItemDir [item.Id_] = dirItem;
					dirItem->setIcon (QIcon::fromTheme ("folder"));
				}
				else
				{
					row << new QStandardItem (item.Name_);
					row << new QStandardItem (item.OwnerNames_.join (", "));
					row << new QStandardItem (QString ("%1 (by %2)")
							.arg (item.ModifiedDate_.toString (Qt::ISODate),
									item.LastModifiedBy_));
				}

				treeItems << row;
			}
			else
				othersItems << item;
		}

		int i = 0;
		while (!othersItems.isEmpty ())
		{
			if (othersItems.count () == i)
				break;

			DriveItem item = othersItems.at (i++);
			if (Id2ItemDir.contains (item.ParentId_))
			{
				QList<QStandardItem*> row;
				row << new QStandardItem (item.Name_);
				row [0]->setData (item.DownloadUrl_, ListingRole::URL);
				row [0]->setData (item.Id_, ListingRole::ID);

				if (!item.IsFolder_)
				{
					row << new QStandardItem (item.OwnerNames_.join (", "));
					row << new QStandardItem (QString ("%1 (by %2)")
							.arg (item.ModifiedDate_.toString (Qt::ISODate), item.LastModifiedBy_));
				}
				else
				{
					row [0]->setIcon (QIcon::fromTheme ("folder"));
					Id2ItemDir [item.Id_] = row [0];
				}

				Id2ItemDir [item.ParentId_]->appendRow (row);
				othersItems.removeAt (i - 1);
				i = 0;
			}
		}

		QList<QStandardItem*> row;
		QStandardItem *trash = new QStandardItem (QIcon::fromTheme ("user-trash"),
				tr ("Trash"));
		row << trash;
		treeItems << row;
		i = 0;
		while (!trashedItems.isEmpty ())
		{
			if (trashedItems.count () == i)
				break;

			DriveItem item = trashedItems.at (i++);
			if (Id2ItemDir.contains (item.ParentId_))
			{
				QList<QStandardItem*> row;
				row << new QStandardItem (item.Name_);
				row [0]->setData (item.DownloadUrl_, ListingRole::URL);
				row [0]->setData (item.Id_, ListingRole::ID);

				if (!item.IsFolder_)
				{
					row << new QStandardItem (item.OwnerNames_.join (", "));
					row << new QStandardItem (QString ("%1 (by %2)")
							.arg (item.ModifiedDate_.toString (Qt::ISODate), item.LastModifiedBy_));
				}
				else
				{
					row [0]->setIcon (QIcon::fromTheme ("folder"));
					Id2ItemDir [item.Id_] = row [0];
				}

				if (IdThrashed.contains (item.ParentId_))
					Id2ItemDir [item.ParentId_]->appendRow (row);
				else
					trash->appendRow (row);
				trashedItems.removeAt (i - 1);
				i = 0;
			}
		}

		for (const auto& row : treeItems)
			for (const auto& item : row)
				item->setEditable (false);

		emit gotListing (treeItems);
	}

}
}
}
