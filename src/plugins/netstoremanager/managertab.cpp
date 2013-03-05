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

#include "managertab.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QInputDialog>
#include <QtDebug>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"
#include "filesmodel.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	ManagerTab::ManagerTab (const TabClassInfo& tc, AccountsManager *am,
			ICoreProxy_ptr proxy, QObject *obj)
	: Parent_ (obj)
	, Info_ (tc)
	, Proxy_ (proxy)
	, AM_ (am)
	, Model_ (new FilesModel (this))
	{
		CopyURL_ = new QAction (tr ("Copy URL..."), this);
		connect (CopyURL_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopyURL ()));
		DeleteFile_ = new QAction (tr ("Delete selected"), this);
		connect (DeleteFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flDelete ()));
		MoveToTrash_ = new QAction (tr ("Move to trash"), this);
		connect (MoveToTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flMoveToTrash ()));
		UntrashFile_ = new QAction (tr ("Restore from trash"), this);
		connect (UntrashFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flRestoreFromTrash ()));
		EmptyTrash_ = new QAction (tr ("Empty trash"), this);
		connect (EmptyTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flEmptyTrash ()));
		CreateDir_ = new QAction (tr ("Create directory"), this);
		connect (CreateDir_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCreateDir ()));
		UploadInCurrentDir_ = new QAction (tr ("Upload in this directory"), this);
		connect (UploadInCurrentDir_,
				SIGNAL (triggered ()),
				this,
				SLOT (flUploadInCurrentDir ()));
		Download_ = new QAction (tr ("Download"), this);
		connect (Download_,
				SIGNAL (triggered ()),
				this,
				SLOT (flDownload ()));

		Ui_.setupUi (this);
		Ui_.FilesTree_->setModel (Model_);
		Q_FOREACH (auto acc, AM_->GetAccounts ())
		{
			auto stP = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
			Ui_.AccountsBox_->addItem (stP->GetStorageIcon (),
					acc->GetAccountName (),
					QVariant::fromValue<IStorageAccount*> (acc));

			if (acc->GetAccountFeatures () & AccountFeature::FileListings)
			{
				connect (acc->GetObject (),
						SIGNAL (gotListing (const QList<QList<QStandardItem*>>&)),
						this,
						SLOT (handleGotListing (const QList<QList<QStandardItem*>>&)));

				connect (acc->GetObject (),
						SIGNAL (gotFileUrl (const QUrl&, const QStringList&)),
						this,
						SLOT (handleGotFileUrl (const QUrl&, const QStringList&)));

				connect (acc->GetObject (),
						SIGNAL (gotNewItem (QList<QStandardItem*>, QStringList)),
						this,
						SLOT (handleGotNewItem (QList<QStandardItem*>, QStringList)));
			}
		}
		if (Ui_.AccountsBox_->count ())
			on_AccountsBox__activated (0);

		Ui_.FilesTree_->setContextMenuPolicy (Qt::CustomContextMenu);
		connect (Ui_.FilesTree_,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleContextMenuRequested (const QPoint&)));
		connect (Ui_.FilesTree_,
				SIGNAL (copiedItem (QStringList, QStringList)),
				this,
				SLOT (handleCopiedItem (QStringList, QStringList)));
		connect (Ui_.FilesTree_,
				SIGNAL (movedItem (QStringList, QStringList)),
				this,
				SLOT (handleMovedItem (QStringList, QStringList)));
		connect (Ui_.FilesTree_,
				SIGNAL (restoredFromTrash (QStringList)),
				this,
				SLOT (handleRestoredFromTrash (QStringList)));
		connect (Ui_.FilesTree_,
				SIGNAL (trashedItem (QStringList)),
				this,
				SLOT (handleTrashedItem (QStringList)));
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
		return 0;
	}

	IStorageAccount* ManagerTab::GetCurrentAccount () const
	{
		const int idx = Ui_.AccountsBox_->currentIndex ();
		if (idx < 0)
			return 0;
		return Ui_.AccountsBox_->itemData (idx).value<IStorageAccount*> ();
	}

	void ManagerTab::CallOnSelection (std::function<void (ISupportFileListings*, const QList<QStringList>&)> func)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		QList<QStringList> ids;
		Q_FOREACH (const auto& idx, Ui_.FilesTree_->selectionModel ()->selectedRows ())
			ids << idx.data (ListingRole::ID).toStringList ();

		if (ids.isEmpty ())
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		func (sfl, ids);
	}

	void ManagerTab::ClearFilesModel ()
	{
		Model_->clear ();

		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		Model_->setHorizontalHeaderLabels (sfl->GetListingHeaders ());
	}

	void ManagerTab::SaveModelState (const QModelIndex& parent)
	{
		auto currentAcc = GetCurrentAccount ();

		auto parentItem = parent.isValid () ?
				Model_->itemFromIndex (parent) :
				Model_->invisibleRootItem ();

		for (int i = 0; i < parentItem->rowCount (); ++i)
		{
			auto item = parentItem->child (i);
			if (!item)
				continue;
			const auto& index = Model_->indexFromItem (item);

			const auto& id = item->data (ListingRole::ID).toString ();
			Account2ItemExpandState_ [currentAcc] [id] = Ui_.FilesTree_->isExpanded (index);

			if (item->hasChildren ())
				SaveModelState (index);
		}
	}

	void ManagerTab::RestoreModelState ()
	{
		if (Account2ItemExpandState_ [GetCurrentAccount ()].isEmpty ())
			return;

		ExpandModelItems ();
		Account2ItemExpandState_.clear ();
	}

	void ManagerTab::ExpandModelItems (const QModelIndex& parent)
	{
		for (int i = 0; i < Model_->rowCount (parent); ++i)
		{
			QStandardItem *item = !parent.isValid () ?
				Model_->item (i) :
				Model_->itemFromIndex (parent)->child (i);
			const auto& id = item->data (ListingRole::ID).toString ();

			if (item->hasChildren () &&
					Account2ItemExpandState_ [GetCurrentAccount ()].value (id))
			{
				const auto& index = Model_->indexFromItem (item);
				Ui_.FilesTree_->expand (index);
				Ui_.FilesTree_->resizeColumnToContents (index.column ());
				ExpandModelItems (index);
			}
		}
	}

	QList<QStringList> ManagerTab::GetTrashedFiles () const
	{
		QList<QStringList> result;
		for (int i = 0, count = Model_->rowCount (); i < count; ++i)
		{
			QStandardItem *item = Model_->item (i);
			if (item->data (ListingRole::ID).toString () == "netstoremanager.item_trash")
			{
				for (int j = 0, cnt = item->rowCount (); j < cnt; ++j)
					result << QStringList (item->child (j)->data (ListingRole::ID).toString ());
				break;
			}
		}

		return result;
	}

	QStandardItem* ManagerTab::GetItemFromId (const QStringList& id) const
	{
		QList<QStandardItem*> parents;
		QList<QStandardItem*> children;

		for (int i = 0; i < Model_->rowCount (); ++i)
			parents << Model_->item (i);

		while (!parents.isEmpty ())
		{
			for (auto parentItem : parents)
			{
				if (parentItem->data (ListingRole::ID) == id)
					return parentItem;

				for (int i = 0; i < parentItem->rowCount (); ++i)
					children << parentItem->child (i);
			}

			std::swap (parents, children);
			children.clear ();
		}

		return 0;
	}

	void ManagerTab::handleGotListing (const QList<QList<QStandardItem*>>& items)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc || sender () != acc->GetObject ())
			return;

		if (items.isEmpty ())
		{
			SaveModelState ();
			ClearFilesModel ();
			return;
		}
		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		const bool trashSupporting = sfl &&
				sfl->GetListingOps () & ListingOp::TrashSupporting;

		QStandardItem *trashItem = new QStandardItem (Proxy_->GetIcon ("user-trash"),
				tr ("Trash"));
		trashItem->setEditable (false);
		trashItem->setData ("netstoremanager.item_trash", ListingRole::ID);

		Q_FOREACH (auto row, items)
			row [0]->data (ListingRole::InTrash).toBool () ?
				trashItem->appendRow (row) :
				Model_->appendRow (row);

		if (trashSupporting)
			Model_->appendRow (trashItem);
		RestoreModelState ();
	}

	void ManagerTab::handleGotFileUrl (const QUrl& url, const QStringList&)
	{
		if (url.isEmpty () || !url.isValid ())
			return;

		const QString& str = url.toString ();
		qApp->clipboard ()->setText (str, QClipboard::Clipboard);
		qApp->clipboard ()->setText (str, QClipboard::Selection);

		QString text = tr ("File URL %1 has been copied to the clipboard.")
				.arg (str);
		emit gotEntity (Util::MakeNotification ("NetStoreManager", text, PInfo_));
	}

	void ManagerTab::handleGotNewItem (const QList<QStandardItem*>& item,
			const QStringList& parentId)
	{
		if (item.isEmpty ())
			return;

		QStandardItem *thisItem = GetItemFromId (item [0]->data (ListingRole::ID).toStringList ());
		if (thisItem)
		{
			const QModelIndex index = Model_->indexFromItem (thisItem);
			const int columnCount = index.parent ().isValid () ?
				thisItem->parent ()->columnCount () :
				Model_->columnCount ();

			for (int i = 0; i < columnCount; ++i)
				Model_->setData (index.sibling (index.row (), i), item.value (i)->text ());
		}
		else
		{
			QStandardItem *parentItem = GetItemFromId (parentId);
			if (!parentItem)
				Model_->appendRow (item);
			else
				parentItem->appendRow (item);
		}
	}

	void ManagerTab::flCopyURL ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		const auto& id = Ui_.FilesTree_->currentIndex ().data (ListingRole::ID).toStringList ();
		if (id.isEmpty ())
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		sfl->RequestUrl (QList<QStringList> () << id);
	}

	void ManagerTab::flDelete ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->Delete (ids); });
	}

	void ManagerTab::flMoveToTrash ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->MoveToTrash (ids); });
	}

	void ManagerTab::flRestoreFromTrash ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->RestoreFromTrash (ids); });
	}

	void ManagerTab::flEmptyTrash ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		if (sfl)
			sfl->EmptyTrash (GetTrashedFiles ());
		else
			qWarning () << Q_FUNC_INFO
					<< acc->GetObject ()
					<< "is not an ISupportFileListings object";
	}

	void ManagerTab::flCreateDir ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
			return;

		QString name = QInputDialog::getText (this,
				"Create directory",
				tr ("New directory name:"));
		if (name.isEmpty ())
			return;

		const QModelIndex& idx = Ui_.FilesTree_->currentIndex ();
		QModelIndex index = idx.sibling (idx.row (), Columns::FirstColumnNumber);
		index = index.data (ListingRole::Directory).toBool () ?
			index :
			index.parent ();
		QStringList id = index.isValid () ?
			index.data (ListingRole::ID).toStringList () :
			QStringList ();
		sfl->CreateDirectory (name, id);
	}

	void ManagerTab::flUploadInCurrentDir ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
			return;

		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file for upload"),
				QDir::homePath ());
		if (filename.isEmpty ())
			return;

		QModelIndex idx = Ui_.FilesTree_->currentIndex ();
		idx = idx.sibling (idx.row (), Columns::FirstColumnNumber);
		QStringList id = idx.data (ListingRole::ID).toStringList ();

		emit uploadRequested (acc, filename, id);
	}

	void ManagerTab::flDownload ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		QModelIndex idx = Ui_.FilesTree_->currentIndex ();
		idx = idx.sibling (idx.row (), Columns::FirstColumnNumber);

		acc->Download (idx.data (ListingRole::ID).toStringList (), "");
	}

	void ManagerTab::on_AccountsBox__activated (int)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		const bool hasListings = acc->GetAccountFeatures () & AccountFeature::FileListings;
		Ui_.Update_->setEnabled (hasListings);
		if (!hasListings)
			return;

		on_Update__released ();

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		DeleteFile_->setEnabled (sfl->GetListingOps () & ListingOp::Delete);
		MoveToTrash_->setEnabled (sfl->GetListingOps () & ListingOp::TrashSupporting);
	}

	void ManagerTab::on_Update__released ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		SaveModelState ();

		Model_->clear ();

		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		sfl->RefreshListing ();
		Model_->setHorizontalHeaderLabels (sfl->GetListingHeaders ());
	}

	void ManagerTab::on_Upload__released ()
	{
		const int accIdx = Ui_.AccountsBox_->currentIndex ();
		if (accIdx < 0)
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

		IStorageAccount *acc = Ui_.AccountsBox_->
				itemData (accIdx).value<IStorageAccount*> ();
		emit uploadRequested (acc, filename);
	}

	void ManagerTab::handleContextMenuRequested (const QPoint& point)
	{
		const auto& index = Ui_.FilesTree_->indexAt (point);

		QMenu *menu = new QMenu;
		if (index.isValid ())
		{
			const bool inTrash = index.data (ListingRole::InTrash).toBool ();
			MoveToTrash_->setEnabled (!inTrash);
			UntrashFile_->setEnabled (inTrash);
			const bool isTrashItem = index.data (ListingRole::ID).toString () == "netstoremanager.item_trash";
			isTrashItem ?
				menu->addAction (EmptyTrash_) :
				menu->addActions ({ CopyURL_, MoveToTrash_, UntrashFile_,  DeleteFile_ });

			if (!inTrash &&
					!isTrashItem)
			{
				menu->insertAction (MoveToTrash_, CreateDir_);
				if (index.data (ListingRole::Directory).toBool ())
					menu->addActions ({ menu->addSeparator (), UploadInCurrentDir_ });
				else
					menu->addActions ({ menu->addSeparator (), Download_ });
			}
		}
		else
			menu->addAction (CreateDir_);

		menu->exec (Ui_.FilesTree_->viewport ()->
				mapToGlobal (QPoint (point.x (), point.y ())));
		menu->deleteLater ();
	}

	void ManagerTab::handleCopiedItem (const QStringList& itemId,
			const QStringList& newParentId)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		sfl->Copy (itemId, newParentId);
	}

	void ManagerTab::handleMovedItem (const QStringList& itemId,
			const QStringList& newParentId)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		sfl->Move (itemId, newParentId);
	}

	void ManagerTab::handleRestoredFromTrash (const QStringList& id)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		sfl->RestoreFromTrash (QList<QStringList> () << id);
	}

	void ManagerTab::handleTrashedItem (const QStringList& id)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		ISupportFileListings *sfl = qobject_cast<ISupportFileListings*> (acc->GetObject ());
		sfl->MoveToTrash (QList<QStringList> () << id);
	}

}
}
