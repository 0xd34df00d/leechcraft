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
#include <algorithm>
#include <QtDebug>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMainWindow>
#include <QPushButton>
#include "core.h"
#include "uploadmanager.h"
#include "xmlsettingsmanager.h"

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
		connect (DriveManager_,
				SIGNAL (gotSharedFileId (const QString&)),
				this,
				SLOT (handleSharedFileId (const QString&)));
		connect (DriveManager_,
				SIGNAL (gotNewItem (DriveItem)),
				this,
				SLOT (handleGotNewItem (DriveItem)));
		connect (DriveManager_,
				SIGNAL (gotChanges (QList<DriveChanges>, qlonglong)),
				this,
				SLOT (handleGotChanges (QList<DriveChanges>, qlonglong)));
	}

	QObject* Account::GetObject ()
	{
		return this;
	}

	QObject* Account::GetParentPlugin () const
	{
		return ParentPlugin_;
	}

	QByteArray Account::GetUniqueID () const
	{
		return ("NetStoreManager.GoogleDrive_" + Name_).toUtf8 ();
	}

	AccountFeatures Account::GetAccountFeatures () const
	{
		return FileListings;
	}

	QString Account::GetAccountName () const
	{
		return Name_;
	}

	void Account::Upload (const QString& filepath, const QStringList& parentId,
			UploadType ut, const QStringList& id)
	{
		auto uploadManager = new UploadManager (filepath,
				ut, parentId, this, id);

		connect (uploadManager,
				SIGNAL (uploadProgress (quint64, quint64, QString)),
				this,
				SIGNAL (upProgress (quint64, quint64, QString)));
		connect (uploadManager,
				SIGNAL (uploadError (QString, QString)),
				this,
				SIGNAL (upError (QString, QString)));
		connect (uploadManager,
				SIGNAL (finished (QStringList, QString)),
				this,
				SIGNAL (upFinished (QStringList, QString)));
		connect (uploadManager,
				SIGNAL (uploadStatusChanged (QString, QString)),
				this,
				SIGNAL (upStatusChanged (QString, QString)));
	}

	void Account::Download (const QStringList& id, const QString& filepath,
			bool silent)
	{
		if (id.isEmpty ())
			return;

		DriveManager_->Download (id.value (0), filepath, silent);
	}

	void Account::Delete (const QList<QStringList>& ids, bool ask)
	{
		if (ids.isEmpty ())
			return;

		const QString& itemId = ids.value (0).value (0);
		if (!ask)
		{
			DriveManager_->RemoveEntry (itemId);
			return;
		}

		auto res = QMessageBox::warning (Core::Instance ().GetProxy ()->GetMainWindow (),
				tr ("Remove item"),
				tr ("Are you sure you want to delete %1? This action cannot be undone."
						"<br><i>Note: if you delete a directory then all files in it will also be deleted.</i>")
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
		return ListingOp::Delete | ListingOp::TrashSupporting | ListingOp::DirectorySupport;
	}

	void Account::MoveToTrash (const QList<QStringList>& ids)
	{
		if (ids.isEmpty ())
			return;
		DriveManager_->MoveEntryToTrash (ids.value (0).value (0));
	}

	void Account::RestoreFromTrash (const QList<QStringList>& ids)
	{
		if (ids.isEmpty ())
			return;
		DriveManager_->RestoreEntryFromTrash (ids.value (0).value (0));
	}

	void Account::EmptyTrash (const QList<QStringList>& ids)
	{
		for (const auto& id : ids)
			DriveManager_->RemoveEntry (id.value (0));
	}

	void Account::RefreshListing ()
	{
		DriveManager_->RefreshListing ();
	}

	void Account::RequestUrl (const QList<QStringList>& ids)
	{
		if (ids.isEmpty ())
			return;

		if (!XmlSettingsManager::Instance ().property ("AutoShareOnUrlRequest").toBool ())
		{
			QMessageBox mbox (QMessageBox::Question,
					tr ("Share item"),
					tr ("The item needs to be shared to obtain the URL. Do you want to share it?"),
					QMessageBox::Yes | QMessageBox::No,
					Core::Instance ().GetProxy ()->GetMainWindow ());
			mbox.setDefaultButton (QMessageBox::Yes);

			QPushButton always (tr ("Always"));
			mbox.addButton (&always, QMessageBox::AcceptRole);

			if (mbox.exec () == QMessageBox::No)
				return;
			else if (mbox.clickedButton () == &always)
				XmlSettingsManager::Instance ().setProperty ("AutoShareOnUrlRequest", true);
		}

		DriveManager_->ShareEntry (ids.value (0).value (0));
	}

	void Account::CreateDirectory (const QString& name, const QStringList& parentId)
	{
		if (name.isEmpty ())
			return;
		DriveManager_->CreateDirectory (name, parentId.value (0));
	}

	void Account::Copy (const QStringList& id, const QStringList& newParentId)
	{
		if (id.isEmpty ())
			return;
		DriveManager_->Copy (id [0], newParentId.value (0));
	}

	void Account::Move (const QStringList& id, const QStringList& newParentId)
	{
		if (id.isEmpty ())
			return;
		DriveManager_->Move (id [0], newParentId.value (0));
	}

	void Account::Rename (const QStringList& id, const QString& newName)
	{
		if (id.isEmpty ())
			return;
		DriveManager_->Rename (id.value (0), newName);
	}

	void Account::RequestChanges ()
	{
		DriveManager_->RequestFileChanges (XmlSettingsManager::Instance ()
				.Property ("LastChangesId", 0).toLongLong ());
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

	DriveManager* Account::GetDriveManager () const
	{
		return DriveManager_;
	}

	namespace
	{
		QList<QStandardItem*> CreateItem (QHash<QString, QList<QStandardItem*>>& id2Item,
				const DriveItem& item)
		{
			QList<QStandardItem*> row;
			if (!id2Item.contains (item.Id_))
			{
				row << new QStandardItem (item.Name_);
				row [0]->setData (item.Id_, ListingRole::ID);
				row [0]->setData (static_cast<bool> (item.Labels_ & DriveItem::ILRemoved),
						ListingRole::InTrash);
				row [0]->setData (item.ModifiedDate_, ListingRole::ModifiedDate);
				row [0]->setData (item.Md5_, ListingRole::Hash);

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
				row [0]->setData (item.IsFolder_, ListingRole::Directory);
			}

			for (const auto& rowItem : row)
				rowItem->setEditable (false);

			return row;
		}

		void CreateChildItem (QHash<QString, QList<QStandardItem*>>& id2Item,
				const DriveItem& parentItem, const DriveItem& childItem)
		{
			const auto& parentRow = !id2Item.contains (parentItem.Id_) ?
				CreateItem (id2Item, parentItem) :
				id2Item [parentItem.Id_];
			const auto& childRow = !id2Item.contains (childItem.Id_) ?
				CreateItem (id2Item, childItem) :
				id2Item [childItem.Id_];
			parentRow [0]->appendRow (childRow);
		}
	}

	void Account::handleFileList (const QList<DriveItem>& items)
	{
		QList<QList<QStandardItem*>> treeItems;

		QHash<QString, DriveItem> trashedItems;
		QHash<QString, DriveItem> id2DriveItem;
		QHash<QString, QList<QStandardItem*>> id2Item;
		QHash<QString, QList<QStandardItem*>> trashedId2Item;

		for (const auto& item : items)
		{
			if (item.Labels_ & DriveItem::ILRemoved)
			{
				trashedItems [item.Id_] = item;
				continue;
			}

			id2DriveItem [item.Id_] = item;
			if (item.ParentIsRoot_ ||
					item.ParentId_.isEmpty ())
				treeItems << CreateItem (id2Item, item);
		}

		const QStringList& keys = id2DriveItem.keys ();
		const auto& values = id2DriveItem.values ();
		for (const auto& item : values)
		{
			if (keys.contains (item.ParentId_))
				CreateChildItem (id2Item, id2DriveItem [item.ParentId_], item);
			else
				CreateItem (id2Item, item);
		}

		const QStringList& trashedKeys = trashedItems.keys ();
		const auto& trashedValues = trashedItems.values ();
		for (const auto& item : trashedValues)
			if (!trashedKeys.contains (item.ParentId_))
				treeItems << CreateItem (trashedId2Item, item);

		for (const auto& item : trashedValues)
			if (trashedKeys.contains (item.ParentId_))
				CreateChildItem (trashedId2Item, trashedItems [item.ParentId_], item);

		treeItems.removeAll (QList<QStandardItem*> ());

		std::sort (treeItems.begin (), treeItems.end (),
				[] (const QList<QStandardItem*>& leftItem, const QList<QStandardItem*>& rightItem)
				{
					if (leftItem [0]->data (ListingRole::Directory).toBool () &&
							!rightItem [0]->data (ListingRole::Directory).toBool ())
						return true;
					else if (!leftItem [0]->data (ListingRole::Directory).toBool () &&
							rightItem [0]->data (ListingRole::Directory).toBool ())
						return false;
					else
						return QString::localeAwareCompare (leftItem [0]->text (),
								rightItem [0]->text ()) < 0;
				});

		emit gotListing (QList<QList<QStandardItem*>> ());
		emit gotListing (treeItems);
	}

	void Account::handleSharedFileId (const QString& id)
	{
		emit gotFileUrl (QUrl (QString ("https://drive.google.com/uc?export=&confirm=no_antivirus&id=%1")
				.arg (id)), QStringList (id));
	}

	void Account::handleGotNewItem (const DriveItem& item)
	{
		QHash<QString, QList<QStandardItem*>> map;
		auto row = CreateItem (map, item);
		emit gotNewItem (row, QStringList (item.ParentId_));
	}

	void Account::handleGotChanges (const QList<DriveChanges>& driveChanges, qlonglong lastId)
	{
		XmlSettingsManager::Instance ().setProperty ("LastChangesId", lastId);

		QList<Change> changes;
		for (const auto& driveChange : driveChanges)
		{
			//TODO setting for shared files
			if (driveChange.FileResource_.PermissionRole_ != DriveItem::Roles::Owner)
				continue;

			QHash<QString, QList<QStandardItem*>> map;
			QList<QStandardItem*> row = CreateItem (map, driveChange.FileResource_);
			if (row.value (0)->text ().isEmpty ())
				continue;

			Change change;
			change.Deleted_ = driveChange.Deleted_;
			change.Id_ << driveChange.FileId_;
			change.Row_ = row;
			change.ParentId_ << driveChange.FileResource_.ParentId_;
			change.ParentIsRoot_ = driveChange.FileResource_.ParentIsRoot_;

			changes << change;
		}

		emit gotChanges (changes);
	}

}
}
}
