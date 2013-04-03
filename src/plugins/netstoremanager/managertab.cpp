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
#include "fileslistmodel.h"
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
	, ListModel_ (new FilesListModel (this))
	, ViewMode_ (static_cast<ViewMode> (XmlSettingsManager::Instance ()
			.Property ("ViewMode", VMTree).toInt ()))
	, ViewModeAction_ (0)
	, AccountsBox_ (0)

	{
		FillToolbar ();

// 		CopyURL_ = new QAction (tr ("Copy URL..."), this);
// 		connect (CopyURL_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flCopyURL ()));
// 		DeleteFile_ = new QAction (tr ("Delete selected"), this);
// 		connect (DeleteFile_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flDelete ()));
// 		MoveToTrash_ = new QAction (tr ("Move to trash"), this);
// 		connect (MoveToTrash_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flMoveToTrash ()));
// 		UntrashFile_ = new QAction (tr ("Restore from trash"), this);
// 		connect (UntrashFile_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flRestoreFromTrash ()));
// 		EmptyTrash_ = new QAction (tr ("Empty trash"), this);
// 		connect (EmptyTrash_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flEmptyTrash ()));
// 		CreateDir_ = new QAction (tr ("Create directory"), this);
// 		connect (CreateDir_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flCreateDir ()));
// 		UploadInCurrentDir_ = new QAction (tr ("Upload in this directory"), this);
// 		connect (UploadInCurrentDir_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flUploadInCurrentDir ()));
// 		Download_ = new QAction (tr ("Download"), this);
// 		connect (Download_,
// 				SIGNAL (triggered ()),
// 				this,
// 				SLOT (flDownload ()));

		Ui_.setupUi (this);

		connect (AM_,
				SIGNAL(accountAdded (QObject*)),
				this,
				SLOT (handleAccountAdded (QObject*)));
		connect (AM_,
				SIGNAL(accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)));

		Ui_.FilesView_->setModel (ProxyModel_);
		Ui_.FilesView_->setModel (ProxyModel_);
		if (ViewMode_ == VMTree)
		{
			ProxyModel_->setSourceModel (TreeModel_);
			TreeModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Modify") });
		}
		else
		{
			ProxyModel_->setSourceModel (ListModel_);
			ListModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Modify") });
		}
		Ui_.FilesView_->header ()->setResizeMode (Columns::Name, QHeaderView::Interactive);
		connect (Ui_.FilesView_->header (),
				SIGNAL (sectionResized (int, int, int)),
				this,
				SLOT (handleFilesViewSectionResized (int, int, int)));
		Ui_.FilesView_->setContextMenuPolicy (Qt::CustomContextMenu);

		connect (Ui_.FilesView_,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleContextMenuRequested (const QPoint&)));

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

		changeViewMode (ViewMode_ == VMList);

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
//
// 				connect (acc->GetQObject (),
// 						SIGNAL (gotFileUrl (const QUrl&, const QStringList&)),
// 						this,
// 						SLOT (handleGotFileUrl (const QUrl&, const QStringList&)));
//
// 				connect (acc->GetQObject (),
// 						SIGNAL (gotNewItem (QList<QStandardItem*>, QStringList)),
// 						this,
// 						SLOT (handleGotNewItem (QList<QStandardItem*>, QStringList)));
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

		if (!AccountsBox_->count ())
			ShowAccountActions (false);
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
		switch (ViewMode_)
		{
			case VMTree:
				TreeModel_->removeRows (0, TreeModel_->rowCount ());
				break;
			case VMList:
				ListModel_->removeRows (0, ListModel_->rowCount ());
				break;
			default:
				break;
		}
	}

	void ManagerTab::FillModel (IStorageAccount *acc)
	{
		switch (ViewMode_)
		{
			case VMTree:
				FillTreeModel (acc);
				break;
			case VMList:
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
		if (trashSupport)
		{
			trashItem = new QStandardItem (Proxy_->GetIcon ("user-trash"), tr ("Trash"));
			trashItem->setData ("netstoremanager.item_trash", ListingRole::ID);
			trashItem->setEditable (false);
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

		for (int i = 0; i < items.count (); ++i)
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
				for (int i = 0; i < res.at (0)->rowCount (); ++i)
					TreeModel_->appendRow (res.at (0)->takeRow (i));
			}
			else
				TreeModel_->appendRow (resultItems [itemKey]);
		}

		for (auto item : trashedItems.values ())
			trashItem->appendRow (item);

		if (trashSupport)
			TreeModel_->appendRow (trashItem);

		Ui_.FilesView_->header ()->resizeSection (Columns::Name,
				XmlSettingsManager::Instance ().Property ("ViewSectionSize",
						Ui_.FilesView_->header ()->sectionSize (Columns::Name)).toInt ());
	}

	void ManagerTab::FillListModel (IStorageAccount *acc)
	{
		//TODO
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

	void ManagerTab::ClearFilesModel ()
	{
// 		Model_->clear ();
//
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		Model_->setHorizontalHeaderLabels (sfl->GetListingHeaders ());
	}

	void ManagerTab::SaveModelState (const QModelIndex& parent)
	{
// 		auto currentAcc = GetCurrentAccount ();
//
// 		auto parentItem = parent.isValid () ?
// 				TreeModel_->itemFromIndex (parent) :
// 				TreeModel_->invisibleRootItem ();
//
// 		for (int i = 0; i < parentItem->rowCount (); ++i)
// 		{
// 			auto item = parentItem->child (i);
// 			if (!item)
// 				continue;
// 			const auto& index = Model_->indexFromItem (item);
//
// 			const auto& id = item->data (ListingRole::ID).toString ();
// 			Account2ItemExpandState_ [currentAcc] [id] = Ui_.FilesTree_->isExpanded (index);
//
// 			if (item->hasChildren ())
// 				SaveModelState (index);
// 		}
	}

	void ManagerTab::RestoreModelState ()
	{
// 		if (Account2ItemExpandState_ [GetCurrentAccount ()].isEmpty ())
// 			return;
//
// 		ExpandModelItems ();
// 		Account2ItemExpandState_.clear ();
	}

	void ManagerTab::ExpandModelItems (const QModelIndex& parent)
	{
// 		for (int i = 0; i < Model_->rowCount (parent); ++i)
// 		{
// 			QStandardItem *item = !parent.isValid () ?
// 				Model_->item (i) :
// 				Model_->itemFromIndex (parent)->child (i);
// 			const auto& id = item->data (ListingRole::ID).toString ();
//
// 			if (item->hasChildren () &&
// 					Account2ItemExpandState_ [GetCurrentAccount ()].value (id))
// 			{
// 				const auto& index = Model_->indexFromItem (item);
// 				Ui_.FilesTree_->expand (index);
// 				Ui_.FilesTree_->resizeColumnToContents (index.column ());
// 				ExpandModelItems (index);
// 			}
// 		}
	}

	QStandardItem* ManagerTab::GetItemFromId (const QStringList& id) const
	{
// 		QList<QStandardItem*> parents;
// 		QList<QStandardItem*> children;
//
// 		for (int i = 0; i < Model_->rowCount (); ++i)
// 			parents << Model_->item (i);
//
// 		while (!parents.isEmpty ())
// 		{
// 			for (auto parentItem : parents)
// 			{
// 				if (parentItem->data (ListingRole::ID) == id)
// 					return parentItem;
//
// 				for (int i = 0; i < parentItem->rowCount (); ++i)
// 					children << parentItem->child (i);
// 			}
//
// 			std::swap (parents, children);
// 			children.clear ();
// 		}

		return 0;
	}

	void ManagerTab::changeViewMode (bool set)
	{
		if (set)
		{
			ViewModeAction_->setText (tr ("List view mode"));
			ViewModeAction_->setIcon (Proxy_->GetIcon ("view-list-details"));
			ViewMode_ = VMList;
		}
		else
		{
			ViewModeAction_->setText (tr ("Tree view mode"));
			ViewModeAction_->setIcon (Proxy_->GetIcon ("view-list-tree"));
			ViewMode_ = VMTree;
		}
		ViewModeAction_->setChecked (set);
		XmlSettingsManager::Instance ().setProperty ("ViewMode", ViewMode_);
	}

	void ManagerTab::handleRefresh ()
	{
		auto acc = GetCurrentAccount ();
		if (!acc)
			return;

		switch (ViewMode_)
		{
			case VMTree:
				if (!TreeModel_->rowCount ())
					requestFileListings (acc);
				else
					requestFileChanges (acc);
				break;
			case VMList:
				//TODO
				break;
			default:
				break;
		}
// 		SaveModelState ();
	}

	void ManagerTab::handleUpload ()
	{
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

		for (auto item : items)
			Id2Item_ [item->ID_] = item;

		FillModel (acc);
// 		if (items.isEmpty ())
// 		{
// 			SaveModelState ();
// 			ClearFilesModel ();
// 			return;
// 		}

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		const bool trashSupporting = sfl &&
// 				sfl->GetListingOps () & ListingOp::TrashSupporting;
//
// 		QStandardItem *trashItem = new QStandardItem (Proxy_->GetIcon ("user-trash"),
// 				tr ("Trash"));
// 		trashItem->setEditable (false);
// 		trashItem->setData ("netstoremanager.item_trash", ListingRole::ID);
//
// 		Q_FOREACH (auto row, items)
// 			row [0]->data (ListingRole::InTrash).toBool () ?
// 				trashItem->appendRow (row) :
// 				Model_->appendRow (row);
//
// 		if (trashSupporting)
// 			Model_->appendRow (trashItem);
// 		RestoreModelState ();

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("NetStoreManager",
				tr ("File list updated"), PInfo_));
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

	void ManagerTab::handleGotFileUrl (const QUrl& url, const QStringList&)
	{
// 		if (url.isEmpty () || !url.isValid ())
// 			return;
//
// 		const QString& str = url.toString ();
// 		qApp->clipboard ()->setText (str, QClipboard::Clipboard);
// 		qApp->clipboard ()->setText (str, QClipboard::Selection);
//
// 		QString text = tr ("File URL %1 has been copied to the clipboard.")
// 				.arg (str);
// 		emit gotEntity (Util::MakeNotification ("NetStoreManager", text, PInfo_));
	}

	void ManagerTab::handleGotNewItem (const QList<QStandardItem*>& item,
			const QStringList& parentId)
	{
// 		if (item.isEmpty ())
// 			return;
//
// 		QStandardItem *thisItem = GetItemFromId (item [0]->data (ListingRole::ID).toStringList ());
// 		if (thisItem)
// 		{
// 			const QModelIndex index = Model_->indexFromItem (thisItem);
// 			const int columnCount = index.parent ().isValid () ?
// 				thisItem->parent ()->columnCount () :
// 				Model_->columnCount ();
//
// 			for (int i = 0; i < columnCount; ++i)
// 				Model_->setData (index.sibling (index.row (), i), item.value (i)->text ());
// 		}
// 		else
// 		{
// 			QStandardItem *parentItem = GetItemFromId (parentId);
// 			if (!parentItem)
// 				Model_->appendRow (item);
// 			else
// 				parentItem->appendRow (item);
// 		}
	}

	void ManagerTab::flCopyURL ()
	{
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		const auto& id = Ui_.FilesTree_->currentIndex ().data (ListingRole::ID).toStringList ();
// 		if (id.isEmpty ())
// 			return;
//
// 		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		sfl->RequestUrl (QList<QStringList> () << id);
	}

	void ManagerTab::flDelete ()
	{
// 		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->Delete (ids); });
	}

	void ManagerTab::flMoveToTrash ()
	{
// 		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->MoveToTrash (ids); });
	}

	void ManagerTab::flRestoreFromTrash ()
	{
// 		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->RestoreFromTrash (ids); });
	}

	void ManagerTab::flEmptyTrash ()
	{
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		if (sfl)
// 			sfl->EmptyTrash (GetTrashedFiles ());
// 		else
// 			qWarning () << Q_FUNC_INFO
// 					<< acc->GetQObject ()
// 					<< "is not an ISupportFileListings object";
	}

	void ManagerTab::flCreateDir ()
	{
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
// 			return;
//
// 		QString name = QInputDialog::getText (this,
// 				"Create directory",
// 				tr ("New directory name:"));
// 		if (name.isEmpty ())
// 			return;
//
// 		const QModelIndex& idx = Ui_.FilesTree_->currentIndex ();
// 		QModelIndex index = idx.sibling (idx.row (), Columns::FirstColumnNumber);
// 		index = index.data (ListingRole::Directory).toBool () ?
// 			index :
// 			index.parent ();
// 		QStringList id = index.isValid () ?
// 			index.data (ListingRole::ID).toStringList () :
// 			QStringList ();
// 		sfl->CreateDirectory (name, id);
	}

	void ManagerTab::flUploadInCurrentDir ()
	{
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
// 			return;
//
// 		const QString& filename = QFileDialog::getOpenFileName (this,
// 				tr ("Select file for upload"),
// 				QDir::homePath ());
// 		if (filename.isEmpty ())
// 			return;
//
// 		QModelIndex idx = Ui_.FilesTree_->currentIndex ();
// 		idx = idx.sibling (idx.row (), Columns::FirstColumnNumber);
// 		QStringList id = idx.data (ListingRole::ID).toStringList ();
//
// 		emit uploadRequested (acc, filename, id);
	}

	void ManagerTab::flDownload ()
	{
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		QModelIndex idx = Ui_.FilesTree_->currentIndex ();
// 		idx = idx.sibling (idx.row (), Columns::FirstColumnNumber);
//
// 		acc->Download (idx.data (ListingRole::ID).toStringList (), "");
	}

	void ManagerTab::on_AccountsBox__currentIndexChanged (int)
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
		requestFileListings (acc);
//
// 		const bool hasListings = acc->GetAccountFeatures () & AccountFeature::FileListings;
// 		Ui_.Update_->setEnabled (hasListings);
// 		if (!hasListings)
// 			return;
//
// 		on_Update__released ();
//
// 		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		DeleteFile_->setEnabled (sfl->GetListingOps () & ListingOp::Delete);
// 		MoveToTrash_->setEnabled (sfl->GetListingOps () & ListingOp::TrashSupporting);
	}

	void ManagerTab::on_Update__released ()
	{
// 		IStorageAccount *acc = GetCurrentAccount ();
// 		if (!acc)
// 			return;
//
// 		SaveModelState ();
//
// 		Model_->clear ();
//
// 		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
// 		sfl->RefreshListing ();
// 		Model_->setHorizontalHeaderLabels (sfl->GetListingHeaders ());
	}

	void ManagerTab::on_Upload__released ()
	{
// 		const int accIdx = Ui_.AccountsBox_->currentIndex ();
// 		if (accIdx < 0)
// 		{
// 			QMessageBox::critical (this,
// 					tr ("Error"),
// 					tr ("You first need to add an account."));
// 			return;
// 		}
//
// 		const QString& filename = QFileDialog::getOpenFileName (this,
// 				tr ("Select file for upload"),
// 				QDir::homePath ());
// 		if (filename.isEmpty ())
// 			return;
//
// 		IStorageAccount *acc = Ui_.AccountsBox_->
// 				itemData (accIdx).value<IStorageAccount*> ();
// 		emit uploadRequested (acc, filename);
	}

	void ManagerTab::handleContextMenuRequested (const QPoint& point)
	{
// 		const auto& index = Ui_.FilesTree_->indexAt (point);
//
// 		QMenu *menu = new QMenu;
// 		if (index.isValid ())
// 		{
// 			const bool inTrash = index.data (ListingRole::InTrash).toBool ();
// 			MoveToTrash_->setEnabled (!inTrash);
// 			UntrashFile_->setEnabled (inTrash);
// 			const bool isTrashItem = index.data (ListingRole::ID).toString () == "netstoremanager.item_trash";
// 			isTrashItem ?
// 				menu->addAction (EmptyTrash_) :
// 				menu->addActions ({ CopyURL_, MoveToTrash_, UntrashFile_,  DeleteFile_ });
//
// 			if (!inTrash &&
// 					!isTrashItem)
// 			{
// 				menu->insertAction (MoveToTrash_, CreateDir_);
// 				if (index.data (ListingRole::Directory).toBool ())
// 					menu->addActions ({ menu->addSeparator (), UploadInCurrentDir_ });
// 				else
// 					menu->addActions ({ menu->addSeparator (), Download_ });
// 			}
// 		}
// 		else
// 			menu->addAction (CreateDir_);
//
// 		menu->exec (Ui_.FilesTree_->viewport ()->
// 				mapToGlobal (QPoint (point.x (), point.y ())));
// 		menu->deleteLater ();
	}

}
}
