/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "managertab.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QInputDialog>
#include <QComboBox>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"
#include "filestreemodel.h"
#include "xmlsettingsmanager.h"
#include "filesproxymodel.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	ManagerTab::ManagerTab (const TabClassInfo& tc, AccountsManager *am,
			ICoreProxy_ptr proxy, QObject *obj)
	: Parent_ (obj)
	, Info_ (tc)
	, Proxy_ (proxy)
	, ToolBar_ (new QToolBar (this))
	, AM_ (am)
	, ProxyModel_ (new FilesProxyModel (this))
	, TreeModel_ (new FilesTreeModel (this))
	, ViewMode_ (static_cast<ViewMode> (XmlSettingsManager::Instance ()
			.Property ("ViewMode", VMTree).toInt ()))
	, ViewModeAction_ (0)
	, AccountsBox_ (0)
	{
		Ui_.setupUi (this);

		Ui_.FilesView_->setModel (ProxyModel_);
		ProxyModel_->setSourceModel (TreeModel_);
		TreeModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Modify") });
		Ui_.FilesView_->header ()->setResizeMode (Columns::Name, QHeaderView::Interactive);

		connect (Ui_.FilesView_->header (),
				 SIGNAL (sectionResized (int, int, int)),
				 this,
		   SLOT (handleFilesViewSectionResized (int, int, int)));
		Ui_.FilesView_->setContextMenuPolicy (Qt::CustomContextMenu);

		FillToolbar ();

		CopyURL_ = new QAction (Proxy_->GetIcon ("edit-copy"),
				tr ("Copy URL..."), this);
		connect (CopyURL_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopyUrl ()));
		Copy_ = new QAction (Proxy_->GetIcon ("edit-copy"),
				tr ("Copy..."), this);
		connect (Copy_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopy ()));
		Move_ = new QAction (Proxy_->GetIcon ("transform-move"),
				tr ("Move..."), this);
		connect (Move_,
				SIGNAL (triggered ()),
				this,
				SLOT (flMove ()));
		Rename_ = new QAction (Proxy_->GetIcon ("edit-rename"),
				tr ("Rename..."), this);
		connect (Rename_,
				SIGNAL (triggered ()),
				this,
				SLOT (flRename ()));
		Paste_ = new QAction (Proxy_->GetIcon ("edit-paste"),
				tr ("Paste"), this);
		connect (Paste_,
				SIGNAL (triggered ()),
				this,
				SLOT (flPaste ()));
		DeleteFile_ = new QAction (Proxy_->GetIcon ("edit-delete"),
				tr ("Delete..."), this);
		connect (DeleteFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flDelete ()));
		MoveToTrash_ = new QAction (Proxy_->GetIcon ("edit-clear"),
				tr ("Move to trash"), this);
		connect (MoveToTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flMoveToTrash ()));
		UntrashFile_ = new QAction (Proxy_->GetIcon ("edit-undo"),
				tr ("Restore from trash"), this);
		connect (UntrashFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flRestoreFromTrash ()));
		EmptyTrash_ = new QAction (Proxy_->GetIcon ("trash-empty"),
				tr ("Empty trash"), this);
		connect (EmptyTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flEmptyTrash ()));
		CreateDir_ = new QAction (Proxy_->GetIcon ("folder-new"),
				tr ("Create directory"), this);
		connect (CreateDir_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCreateDir ()));
		UploadInCurrentDir_ = new QAction (Proxy_->GetIcon ("svn-commit"),
				tr ("Upload in this directory"), this);
		connect (UploadInCurrentDir_,
				SIGNAL (triggered ()),
				this,
				SLOT (flUploadInCurrentDir ()));
		Download_ = new QAction (Proxy_->GetIcon ("download"),
				tr ("Download"), this);
		connect (Download_,
				SIGNAL (triggered ()),
				this,
				SLOT (flDownload ()));
		Trash_ = new QAction (Proxy_->GetIcon ("user-trash"),
				tr ("Trash"), this);
		connect (Trash_,
				SIGNAL (triggered ()),
				this,
				SLOT (showTrashContent ()));

		connect (AM_,
				SIGNAL(accountAdded (QObject*)),
				this,
				SLOT (handleAccountAdded (QObject*)));
		connect (AM_,
				SIGNAL(accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));

		connect (Ui_.FilesView_,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleContextMenuRequested (const QPoint&)));
		connect (Ui_.FilesView_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handleDoubleClicked (QModelIndex)));

		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeCopied (QList<QByteArray>, QByteArray)),
				this,
				SLOT (handleItemsAboutToBeCopied (QList<QByteArray>,QByteArray)));
		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeMoved (QList<QByteArray>, QByteArray)),
				this,
				SLOT (handleItemsAboutToBeMoved (QList<QByteArray>, QByteArray)));
		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeRestoredFromTrash (QList<QByteArray>)),
				this,
				SLOT (handleItemsAboutToBeRestoredFromTrash (QList<QByteArray>)));
		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeTrashed (QList<QByteArray>)),
				this,
				SLOT (handleItemsAboutToBeTrashed (QList<QByteArray>)));
	}

	TabClassInfo ManagerTab::GetTabClassInfo () const
	{
		return Info_;
	}

	QObject* ManagerTab::ParentMultiTabs ()
	{
		return Parent_;
	}

	void ManagerTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* ManagerTab::GetToolBar () const
	{
		return ToolBar_;
	}

	void ManagerTab::FillToolbar ()
	{
		ViewModeAction_ = new QAction (this);
		ViewModeAction_->setCheckable (true);
		connect (ViewModeAction_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (changeViewMode (bool)));

		ToolBar_->addAction (ViewModeAction_);
		ToolBar_->addSeparator ();

		AccountsBox_ = new QComboBox (this);
		Q_FOREACH (auto acc, AM_->GetAccounts ())
		{
			auto stP = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
			AccountsBox_->addItem (stP->GetStorageIcon (),
					acc->GetAccountName (),
					QVariant::fromValue<IStorageAccount*> (acc));

			if (acc->GetAccountFeatures () & AccountFeature::FileListings)
			{
				connect (acc->GetQObject (),
						SIGNAL (gotListing (const QList<StorageItem*>&)),
						this,
						SLOT (handleGotListing (const QList<StorageItem*>&)));
				connect (acc->GetQObject (),
						SIGNAL (gotFileUrl (QUrl, QByteArray)),
						this,
						SLOT (handleGotFileUrl (QUrl, QByteArray)));
			}
		}

		ToolBar_->addWidget (AccountsBox_);

		Refresh_ = new QAction (Proxy_->GetIcon ("view-refresh"), tr ("Refresh"), this);
		connect (Refresh_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRefresh ()));
		Upload_ = new QAction (Proxy_->GetIcon ("svn-commit"), tr ("Upload"), this);
		connect (Upload_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleUpload ()));

		ToolBar_->addActions ({ Refresh_, Upload_ });
		ToolBar_->addSeparator ();

		ShowAccountActions (AccountsBox_->count ());

		connect (AccountsBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleCurrentIndexChanged (int)));

		changeViewMode (ViewMode_ == VMList);
	}

	void ManagerTab::ShowAccountActions (bool show)
	{
		if (Upload_)
			Upload_->setVisible (show);

		if (Refresh_)
			Refresh_->setVisible (show);
	}

	IStorageAccount* ManagerTab::GetCurrentAccount () const
	{
		const int idx = AccountsBox_->currentIndex ();
		if (idx < 0)
			return 0;
		return AccountsBox_->itemData (idx).value<IStorageAccount*> ();
	}

	void ManagerTab::ClearModel ()
	{
		TreeModel_->removeRows (0, TreeModel_->rowCount ());
	}

	void ManagerTab::FillModel (IStorageAccount *acc)
	{
		switch (ViewMode_)
		{
			case VMTree:
				ClearModel ();
				FillTreeModel (acc);
				break;
			case VMList:
				ClearModel ();
				FillListModel (acc);
				break;
			default:
				break;
		}
	}

	namespace
	{
		QList<QStandardItem*> CreateItems (StorageItem *storageItem, ICoreProxy_ptr proxy)
		{
			QList<QStandardItem*> result;

			QStandardItem *name = new QStandardItem (storageItem->Name_);

			name->setEditable (false);
			name->setData (storageItem->ID_, ListingRole::ID);
			name->setData (storageItem->MD5_, ListingRole::Hash);
			name->setData (storageItem->IsDirectory_, ListingRole::Directory);
			name->setData (storageItem->IsTrashed_, ListingRole::InTrash);
			name->setIcon (proxy->GetIcon (storageItem->IsDirectory_ ?
					"inode-directory" :
					storageItem->MimeType_));
			if (name->icon ().isNull ())
				qDebug () << "[NetStoreManager]"
						<< "Unknown mime type:"
						<< storageItem->MimeType_
						<< "for file"
						<< storageItem->Name_
						<< storageItem->ID_;

			QStandardItem *modify = new QStandardItem (storageItem->ModifyDate_
					.toString ("dd.MM.yy hh:mm"));
			modify->setEditable (false);

			result.append (name);
			result.append (modify);
			return result;
		}
	}

	void ManagerTab::FillTreeModel (IStorageAccount *acc)
	{
		if (acc != GetCurrentAccount ())
			return;

		QHash<QByteArray, QList<QStandardItem*>> resultItems;
		QHash<QByteArray, QList<QStandardItem*>> trashedItems;
		resultItems ["rootItem"] = { new QStandardItem };

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		const bool trashSupport = sfl->GetListingOps () & ListingOp::TrashSupporting;
		QStandardItem *trashItem = 0;
		QStandardItem *trashModifyItem = 0;
		if (trashSupport)
		{
			trashItem = new QStandardItem (Proxy_->GetIcon ("user-trash"), tr ("Trash"));
			trashItem->setData ("netstoremanager.item_trash", ListingRole::ID);
			trashItem->setFlags (Qt::ItemIsEnabled);

			trashModifyItem = new QStandardItem;
			trashModifyItem->setFlags (Qt::ItemIsEnabled);
		}

		QList<QByteArray> addedChildDirs;
		QList<StorageItem*> items = Id2Item_.values ();
		for (int i = items.count () - 1; i >= 0; --i)
		{
			auto item = items [i];
			QList<QStandardItem*> resItems;

			if (item->IsDirectory_)
			{
				resItems = CreateItems (item, Proxy_);
				if (item->IsTrashed_)
					trashedItems [item->ID_] = resItems;
				else
					resultItems [item->ID_] = resItems;
				items.removeAt (i);
			}
		}

		for (const auto& key : resultItems.keys ())
		{
			auto item = Id2Item_.value (key);
			if (item &&
					resultItems.contains (item->ParentID_))
			{
				resultItems [item->ParentID_].at (0)->appendRow (resultItems [key]);
				addedChildDirs << key;
			}
		}

		for (const auto& key : trashedItems.keys ())
		{
			auto item = Id2Item_.value (key);
			if (item &&
					trashedItems.contains (item->ParentID_))
			{
				if (!item->IsTrashed_)
					continue;

				if (Id2Item_ [item->ParentID_]->IsTrashed_)
					trashedItems [item->ParentID_].at (0)->appendRow (trashedItems [key]);
				else
					trashItem->appendRow (trashedItems [key]);

				if (item->IsDirectory_)
					addedChildDirs << key;
			}
		}

		for (int i = items.count () - 1; i >= 0 ; --i)
		{
			auto item = items [i];

			auto resItems = CreateItems (item, Proxy_);
			if (!item->IsTrashed_)
			{
				if (resultItems.contains (item->ParentID_))
					resultItems [item->ParentID_].at (0)->appendRow (resItems);
				else
					resultItems ["rootItem"].at (0)->appendRow (resItems);
			}
			else
			{
				if (trashedItems.contains (item->ParentID_))
					trashedItems [item->ParentID_].at (0)->appendRow (resItems);
				else
					trashItem->appendRow (resItems);
			}

			items.removeAt (i);
		}

		for (const auto& id : addedChildDirs)
		{
			resultItems.remove (id);
			trashedItems.remove (id);
		}

		for (auto itemKey : resultItems.keys ())
		{
			if (itemKey == "rootItem")
			{
				auto res = resultItems ["rootItem"];
				for (int i = res.at (0)->rowCount () - 1; i >= 0 ; --i)
					TreeModel_->appendRow (res.at (0)->takeRow (i));
			}
			else
				TreeModel_->appendRow (resultItems [itemKey]);
		}

		for (auto item : trashedItems.values ())
			trashItem->appendRow (item);

		if (trashSupport)
			TreeModel_->appendRow ({ trashItem, trashModifyItem });

		Ui_.FilesView_->header ()->resizeSection (Columns::Name,
				XmlSettingsManager::Instance ().Property ("ViewSectionSize",
						Ui_.FilesView_->header ()->sectionSize (Columns::Name)).toInt ());
	}

	void ManagerTab::FillListModel (IStorageAccount *acc)
	{
		if (acc != GetCurrentAccount ())
			return;

		ShowListItemsWithParent ();

		Ui_.FilesView_->header ()->resizeSection (Columns::Name,
				XmlSettingsManager::Instance ().Property ("ViewSectionSize",
						Ui_.FilesView_->header ()->sectionSize (Columns::Name)).toInt ());
	}

	void ManagerTab::requestFileListings (IStorageAccount *acc)
	{
		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		if (!sfl)
		{
			qWarning () << Q_FUNC_INFO
					<< acc
					<< "doesn't support FileListings";
			return;
		}
		sfl->RefreshListing ();
	}

	void ManagerTab::requestFileChanges (IStorageAccount*)
	{
		//TODO
	}

	QList<QByteArray> ManagerTab::GetTrashedFiles () const
	{
		QList<QByteArray> result;

		switch (ViewMode_)
		{
		case VMTree:
			for (int i = TreeModel_->rowCount () - 1; i >= 0; --i)
			{
				QStandardItem *item = TreeModel_->item (i);
				if (item->data (ListingRole::ID).toString () == "netstoremanager.item_trash")
				{
					for (int j = 0, cnt = item->rowCount (); j < cnt; ++j)
						result << item->child (j)->data (ListingRole::ID).toByteArray ();
					break;
				}
			}
			break;
		case VMList:
			break;
		default:
			break;
		}

		return result;
	}

	void ManagerTab::CallOnSelection (std::function<void (ISupportFileListings*, const QList<QByteArray>&)> func)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		QList<QByteArray> ids;
		Q_FOREACH (const auto& idx, Ui_.FilesView_->selectionModel ()->selectedRows ())
			ids << idx.data (ListingRole::ID).toByteArray ();

		if (ids.isEmpty ())
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		func (sfl, ids);
	}

	void ManagerTab::SaveExpandState (const QModelIndex& parent)
	{
		if (!TreeModel_->rowCount ())
			return;

		auto currentAcc = GetCurrentAccount ();

		auto parentItem = parent.isValid () ?
			TreeModel_->itemFromIndex (parent) :
			TreeModel_->invisibleRootItem ();

		for (int i = 0; i < parentItem->rowCount (); ++i)
		{
			auto item = parentItem->child (i);
			if (!item)
				continue;

			const bool isDir = item->data (ListingRole::Directory).toBool ();
			const auto& id = item->data (ListingRole::ID).toByteArray ();
			if (!isDir &&
					id != "netstoremanager.item_trash")
				continue;

			const auto& index = TreeModel_->indexFromItem (item);
			const bool exp = Ui_.FilesView_->isExpanded (ProxyModel_->mapFromSource (index));
			Account2ItemExpandState_ [currentAcc] [id] = exp;

			if (exp &&
					item->hasChildren ())
				SaveExpandState (index);
		}
	}

	void ManagerTab::RestoreExpandState ()
	{
		if (Account2ItemExpandState_ [GetCurrentAccount ()].isEmpty ())
			return;

		ExpandModelItems ();
		Account2ItemExpandState_.clear ();
	}

	void ManagerTab::ExpandModelItems (const QModelIndex& parent)
	{
		for (int i = 0; i < TreeModel_->rowCount (parent); ++i)
		{
			QStandardItem *item = !parent.isValid () ?
				TreeModel_->item (i) :
				TreeModel_->itemFromIndex (parent)->child (i);
			const auto& id = item->data (ListingRole::ID).toByteArray ();

			if (item->hasChildren () &&
					Account2ItemExpandState_ [GetCurrentAccount ()].value (id))
			{
				const auto& index = TreeModel_->indexFromItem (item);
				Ui_.FilesView_->expand (ProxyModel_->mapFromSource (index));
				ExpandModelItems (index);
			}
		}
	}

	void ManagerTab::ShowListItemsWithParent (const QByteArray& parentId)
	{
		ClearModel ();
		if (Id2Item_.contains (parentId))
		{
			QStandardItem *upLevel = new QStandardItem (Proxy_->GetIcon ("go-up"), "..");
			upLevel->setData ("netstoremanager.item_uplevel", ListingRole::ID);
			upLevel->setData (parentId);
			upLevel->setFlags (Qt::ItemIsEnabled);
			QStandardItem *upLevelModify = new QStandardItem;
			upLevelModify->setFlags (Qt::ItemIsEnabled);
			TreeModel_->appendRow ({ upLevel, upLevelModify });
		}

		const auto& items = Id2Item_.values ();
		for (const auto& item : items)
			if (!item->IsTrashed_)
			{
				if (parentId.isNull () &&
						!Id2Item_.contains (item->ParentID_))
					TreeModel_->appendRow (CreateItems (item, Proxy_));
				else if (!parentId.isNull () &&
						item->ParentID_ == parentId)
					TreeModel_->appendRow (CreateItems (item, Proxy_));
			}
	}

	void ManagerTab::changeViewMode (bool set)
	{
		if (set)
		{
			ViewModeAction_->setText (tr ("List view mode"));
			ViewModeAction_->setIcon (Proxy_->GetIcon ("view-list-details"));
			ToolBar_->addAction (Trash_);
			ViewMode_ = VMList;
		}
		else
		{
			ViewModeAction_->setText (tr ("Tree view mode"));
			ViewModeAction_->setIcon (Proxy_->GetIcon ("view-list-tree"));
			ToolBar_->removeAction (Trash_);
			ViewMode_ = VMTree;
		}
		ViewModeAction_->setChecked (set);
		XmlSettingsManager::Instance ().setProperty ("ViewMode", ViewMode_);

		FillModel (GetCurrentAccount ());
	}

	void ManagerTab::handleRefresh ()
	{
		auto acc = GetCurrentAccount ();
		if (!acc)
			return;

		switch (ViewMode_)
		{
			case VMTree:
				SaveExpandState ();
				requestFileListings (acc);
				break;
			case VMList:
				requestFileListings (acc);
				break;
			default:
				break;
		}
	}

	void ManagerTab::handleUpload ()
	{
		auto acc = GetCurrentAccount ();
		if (!acc)
		{
			QMessageBox::critical (this,
					tr ("Error"),
					tr ("You first need to add an account."));
			return;
		}

		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file for upload"),
				QDir::homePath ());
		if (filename.isEmpty ())
			return;

		QByteArray parentId;
		if (ViewMode_ == VMList)
			parentId = ProxyModel_->index (0, Columns::Name)
					.data (Qt::UserRole + 1).toByteArray ();

		emit uploadRequested (acc, filename, parentId);
	}

	void ManagerTab::handleDoubleClicked (const QModelIndex& idx)
	{
		if (ViewMode_ == VMTree)
			return;

		if (idx.data (ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
		{
			ShowListItemsWithParent (Id2Item_ [idx.data (Qt::UserRole + 1).toByteArray ()]->ParentID_);
			return;
		}

		if (!idx.data (ListingRole::Directory).toBool ())
			return;

		ShowListItemsWithParent (idx.data (ListingRole::ID).toByteArray ());
	}

	void ManagerTab::handleAccountAdded (QObject *accObj)
	{
		auto acc = qobject_cast<IStorageAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "added account is not an IStorageAccount";
			return;
		}

		auto stP = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
		AccountsBox_->addItem (stP->GetStorageIcon (),
				acc->GetAccountName (),
				QVariant::fromValue<IStorageAccount*> (acc));

		if (AccountsBox_->count () == 1)
			ShowAccountActions (true);
	}

	void ManagerTab::handleAccountRemoved (QObject *accObj)
	{
		auto acc = qobject_cast<IStorageAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "removed account is not an IStorageAccount";
			return;
		}

		for (int i = AccountsBox_->count () - 1; i >= 0; --i)
			if (AccountsBox_->itemData (i).value<IStorageAccount*> () == acc)
				AccountsBox_->removeItem (i);

		if (!AccountsBox_->count ())
			ShowAccountActions (false);
	}

	void ManagerTab::handleGotListing (const QList<StorageItem*>& items)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc ||
				sender () != acc->GetQObject ())
			return;

		qDeleteAll (Id2Item_);
		Id2Item_.clear ();
		for (auto item : items)
			Id2Item_ [item->ID_] = item;

		FillModel (acc);

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("NetStoreManager",
				tr ("File list updated"), PInfo_));

		RestoreExpandState ();
	}

	void ManagerTab::handleFilesViewSectionResized (int index,
			int oldSize, int newSize)
	{
		if (index == Columns::Name)
			XmlSettingsManager::Instance ().setProperty ("ViewSectionSize", newSize);
	}

	void ManagerTab::handleItemsAboutToBeCopied (const QList<QByteArray>& ids,
			const QByteArray& newParentId)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->Copy (ids, newParentId);
	}

	void ManagerTab::handleItemsAboutToBeMoved (const QList<QByteArray>& ids,
			const QByteArray& newParentId)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->Move (ids, newParentId);
	}

	void ManagerTab::handleItemsAboutToBeRestoredFromTrash (const QList<QByteArray>& ids)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->RestoreFromTrash (ids);
	}

	void ManagerTab::handleItemsAboutToBeTrashed (const QList<QByteArray>& ids)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->MoveToTrash (ids);
	}

	void ManagerTab::flCopy ()
	{

	}

	void ManagerTab::flMove ()
	{

	}

	void ManagerTab::flRename ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		const QString& oldName = Ui_.FilesView_->currentIndex ().data ().toString ();
		const auto& id = Ui_.FilesView_->currentIndex ()
				.data (ListingRole::ID).toByteArray ();
		QString name = QInputDialog::getText (this,
				"Rename",
				tr ("New name:"),
				QLineEdit::Normal,
				oldName);
		if (name.isEmpty () ||
				name == oldName)
			return;

		sfl->Rename (id, name);
	}

	void ManagerTab::flPaste ()
	{

	}

	void ManagerTab::flDelete ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QByteArray>& ids)
			{ sfl->Delete (ids); });
	}

	void ManagerTab::flMoveToTrash ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QByteArray>& ids)
			{ sfl->MoveToTrash (ids); });
	}

	void ManagerTab::flRestoreFromTrash ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QByteArray>& ids)
			{ sfl->RestoreFromTrash (ids); });
	}

	void ManagerTab::flEmptyTrash ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		if (sfl)
			sfl->Delete (GetTrashedFiles (), false);
		else
			qWarning () << Q_FUNC_INFO
					<< acc->GetQObject ()
					<< "is not an ISupportFileListings object";
	}

	void ManagerTab::flCreateDir ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
			return;

		QString name = QInputDialog::getText (this,
				"Create directory",
				tr ("New directory name:"));
		if (name.isEmpty ())
			return;

		QByteArray id;

		switch (ViewMode_)
		{
		case VMTree:
		{
			const QModelIndex& idx = Ui_.FilesView_->currentIndex ();
			QModelIndex index = idx.sibling (idx.row (), Columns::Name);
			index = index.data (ListingRole::Directory).toBool () ?
				index :
				index.parent ();
			id = index.isValid () ?
				index.data (ListingRole::ID).toByteArray () :
				QByteArray ();
		}
		break;
		case VMList:
			id = ProxyModel_->index (0, Columns::Name)
					.data (Qt::UserRole + 1).toByteArray ();
		break;
		};

		sfl->CreateDirectory (name, id);
	}

	void ManagerTab::flUploadInCurrentDir ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
			return;

		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file for upload"),
				QDir::homePath ());
		if (filename.isEmpty ())
			return;

		QByteArray parentId;
		switch (ViewMode_)
		{
		case VMTree:
		{
			QModelIndex idx = Ui_.FilesView_->currentIndex ();
			idx = idx.sibling (idx.row (), Columns::Name);
			parentId = idx.data (ListingRole::ID).toByteArray ();
		}
		break;
		case VMList:
			parentId = ProxyModel_->index (0, Columns::Name)
					.data (Qt::UserRole + 1).toByteArray ();
		break;
		default:
		break;
		}

		emit uploadRequested (acc, filename, parentId);
	}

	void ManagerTab::flDownload ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		QModelIndex idx = Ui_.FilesView_->currentIndex ();
		idx = idx.sibling (idx.row (), Columns::Name);

		acc->Download (idx.data (ListingRole::ID).toByteArray (), "");
	}

	void ManagerTab::flCopyUrl ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		const QModelIndex& idx = Ui_.FilesView_->currentIndex ();
		QModelIndex index = idx.sibling (idx.row (), Columns::Name);
		const QByteArray id = index.data (ListingRole::ID).toByteArray ();
		if (!Id2Item_ [id]->Url_.isEmpty () &&
				Id2Item_ [id]->Url_.isValid ())
			handleGotFileUrl (Id2Item_ [id]->Url_);
		else
			qobject_cast<ISupportFileListings*> (acc->GetQObject ())->RequestUrl (id);
	}

	void ManagerTab::showTrashContent ()
	{
		ClearModel ();
		QStandardItem *upLevel = new QStandardItem (Proxy_->GetIcon ("go-up"), "..");
		upLevel->setData ("netstoremanager.item_uplevel", ListingRole::ID);
		upLevel->setFlags (Qt::ItemIsEnabled);
		QStandardItem *upLevelModify = new QStandardItem;
		upLevelModify->setFlags (Qt::ItemIsEnabled);
		TreeModel_->appendRow ({ upLevel, upLevelModify });
	}

	void ManagerTab::handleContextMenuRequested (const QPoint& point)
	{
		QList<QModelIndex> idxs = Ui_.FilesView_->selectionModel ()->selectedRows ();
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		const bool dirSupport = sfl->GetListingOps () & ListingOp::DirectorySupport;
		const bool trashSupport = sfl->GetListingOps () & ListingOp::TrashSupporting;

		QMenu *menu = new QMenu;
		switch (ViewMode_)
		{
		case VMTree:
			UploadInCurrentDir_->setText (tr ("Upload in this directory"));
			break;
		case VMList:
			UploadInCurrentDir_->setText (tr ("Upload..."));
			break;
		default:
			break;
		}

		if (!idxs.isEmpty ())
		{
			QList<QAction*> editActions = { Copy_, Move_, Rename_, MoveToTrash_, DeleteFile_ };

			menu->addAction (CopyURL_);
			menu->addSeparator ();

			switch (ViewMode_)
			{
			case VMTree:
			break;
			case VMList:
			{
				menu->addActions ({ UploadInCurrentDir_, Download_ });
				menu->addSeparator ();
				menu->addActions (editActions);
			}
			break;
			}
		}
		else
			menu->addAction (UploadInCurrentDir_);

		menu->addSeparator ();
		menu->addAction (CreateDir_);

/*
			QList<QModelIndex> trashedIndexes;
			QList<QModelIndex> untrashedIndexes;
			bool trashItem = false;
			for (int i = idxs.count () - 1; i >= 0; --i)
			{
				const auto& index = idxs.at (i);
				if (index.data (ListingRole::InTrash).toBool ())
					trashedIndexes << index;
				else if (index.data (ListingRole::ID).toByteArray () == "netstoremanager.item_trash")
					trashItem = true;
				else if (index.data (ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
					idxs.removeAt (i);
				else
					untrashedIndexes << index;
			}

			const bool onlyUntrashed = (untrashedIndexes.count () == idxs.count ());
			const bool onlyTrashed = (trashedIndexes.count () == idxs.count ());

			QList<QAction*> actionsList;
			QList<QAction*> trashedActionsList;
			QList<QAction*> unTrashedActionsList;
			QList<QAction*> trashActionsList;

			trashActionsList << EmptyTrash_;
			trashedActionsList << UntrashFile_
					<< DeleteFile_;
			unTrashedActionsList << (trashSupport ? MoveToTrash_ : DeleteFile_);

			if (trashSupport &&
					trashItem)
				actionsList << trashActionsList;

			if (trashSupport &&
					onlyTrashed)
				actionsList << trashedActionsList;

			if (onlyUntrashed)
			{
				bool dirExists = false;
				for (const auto& index : idxs)
					if (idxs.at (0).data (ListingRole::Directory).toBool ())
					{
						dirExists = true;
						break;
					}
				if (!dirExists)
					unTrashedActionsList << Download_;

				if (dirSupport &&
						idxs.count () == 1 &&
						idxs.at (0).data (ListingRole::Directory).toBool ())
					unTrashedActionsList << Util::CreateSeparator (this)
						<< UploadInCurrentDir_;

				QModelIndex parentIndex = idxs.at (0).parent ();
				bool equals = true;
				for (int i = 1; i < idxs.count (); ++i)
					if (parentIndex != idxs [i].parent ())
					{
						equals = false;
						break;
					}
				if (dirSupport &&
						equals)
					unTrashedActionsList << Util::CreateSeparator (this)
							<< CreateDir_;

				actionsList << unTrashedActionsList;
			}

			if (!onlyTrashed &&
					!onlyUntrashed)
			{
				actionsList << unTrashedActionsList
						<< Util::CreateSeparator (this)
						<< trashedActionsList
						<< Util::CreateSeparator (this);
				if (trashItem)
					actionsList << trashActionsList;
			}

			EmptyTrash_->setEnabled (idxs.count () == 1);
			UntrashFile_->setEnabled (onlyTrashed);
			DeleteFile_->setEnabled (onlyTrashed);
			MoveToTrash_->setEnabled (onlyUntrashed);
			Download_->setEnabled (onlyUntrashed);

			if (idxs.count () == 1 &&
					!idxs.at (0).data (ListingRole::Directory).toBool () &&
					!idxs.at (0).data (ListingRole::InTrash).toBool () &&
					idxs.at (0).data (ListingRole::ID).toByteArray () != "netstoremanager.item_trash")
			{
				menu->addAction (CopyURL_);
				auto item = Id2Item_ [idxs.at (0).data (ListingRole::ID).toByteArray ()];
				if (!item->ExportLinks.isEmpty ())
				{
					QMenu *exportMenu = new QMenu (tr ("Export to..."));
					auto exportAct = menu->addMenu (exportMenu);
					exportAct->setIcon (Proxy_->GetIcon ("document-export"));
					for (const auto& key : item->ExportLinks.keys ())
					{
						const auto& pair = item->ExportLinks [key];
						QAction *action = new QAction (Proxy_->GetIcon (pair.first),
								pair.second, exportMenu);
						action->setProperty ("url", key);
						exportMenu->addAction (action);
						connect (exportMenu,
								SIGNAL (triggered (QAction*)),
								this,
								SLOT (handleExportMenuTriggered (QAction*)),
								Qt::UniqueConnection);
					}
				}
			}

			menu->addActions (actionsList);
		}*/

		if (!menu->isEmpty ())
			menu->exec (Ui_.FilesView_->viewport ()->
					mapToGlobal (QPoint (point.x (), point.y ())));
		menu->deleteLater ();
	}

	void ManagerTab::handleExportMenuTriggered (QAction *action)
	{
		if (!action ||
				action->property ("url").isNull ())
			return;

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeEntity (action->property ("url").toUrl (),
				QString (),
				OnlyHandle | FromUserInitiated));
	}

	void ManagerTab::handleCurrentIndexChanged (int index)
	{
		ClearModel ();
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< acc
					<< "is not an IStorageAccount object";
			return;
		}
		qDeleteAll (Id2Item_);
		Id2Item_.clear ();
		requestFileListings (acc);

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		DeleteFile_->setVisible (sfl->GetListingOps () & ListingOp::Delete);
		MoveToTrash_->setVisible (sfl->GetListingOps () & ListingOp::TrashSupporting);
	}

	void ManagerTab::handleGotFileUrl (const QUrl& url, const QByteArray&)
	{
		if (url.isEmpty () ||
				!url.isValid ())
			return;

		const QString& str = url.toString ();
		qApp->clipboard ()->setText (str, QClipboard::Clipboard);
		qApp->clipboard ()->setText (str, QClipboard::Selection);

		QString text = tr ("File URL has been copied to the clipboard.");
		Proxy_->GetEntityManager ()->
				HandleEntity (Util::MakeNotification ("NetStoreManager", text, PInfo_));
	}

}
}
