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
		QList<QStandardItem*> CreateNewItem (const DriveItem& item,
				QHash<QString, QStandardItem*>& dirs)
		{
			QList<QStandardItem*> row;
			row << new QStandardItem (item.Name_);
			row [0]->setData (item.DownloadUrl_, ListingRole::URL);
			row [0]->setData (item.Id_, ListingRole::ID);
			row [0]->setData (static_cast<bool> (item.Labels_ & DriveItem::ILRemoved),
					ListingRole::InTrash);

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

			for (auto it : row)
				it->setEditable (false);

			return row;
		}
	}

	void Account::handleFileList (const QList<DriveItem>& items)
	{
		QList<QList<QStandardItem*>> treeItems;

		QHash<QString, QStandardItem*> id2ItemDir;
		QHash<QString, DriveItem> trashedItems;
		QList<DriveItem> othersItems;

		Q_FOREACH (const DriveItem& item, items)
		{
			if (item.Labels_ & DriveItem::ILRemoved)
			{
				trashedItems [item.Id_] = item;
				continue;
			}

			if (item.ParentIsRoot_)
				treeItems << CreateNewItem (item, id2ItemDir);
			else
				othersItems << item;
		}

		int i = 0;
		while (!othersItems.isEmpty ())
		{
			if (othersItems.count () == i)
				break;

			DriveItem item = othersItems.at (i++);
			if (id2ItemDir.contains (item.ParentId_))
			{
				auto row = CreateNewItem (item, id2ItemDir);
				id2ItemDir [item.ParentId_]->appendRow (row);
				othersItems.removeAt (i - 1);
				i = 0;
			}
		}

		QList<DriveItem> result;
		QStringList removedKeys;
		Q_FOREACH (const auto& key, trashedItems.keys ())
		{
			const auto& item = trashedItems [key];
			if (!trashedItems.contains (item.ParentId_))
			{
				treeItems << CreateNewItem (trashedItems [key], id2ItemDir);
				removedKeys << key;
			}
		}

		for (const auto& key : removedKeys)
			trashedItems.remove (key);

		i = 0;
		while (!trashedItems.isEmpty ())
		{
			if (trashedItems.count () == i)
				break;

			auto key = trashedItems.keys ().at (i++);
			DriveItem item = trashedItems [key];
			if (id2ItemDir.contains (item.ParentId_))
			{
				auto row = CreateNewItem (item, id2ItemDir);
				id2ItemDir [item.ParentId_]->appendRow (row);
				trashedItems.remove (key);
				i = 0;
			}
		}

		emit gotListing (QList<QList<QStandardItem*>> ());
		emit gotListing (treeItems);
	}

}
}
}
