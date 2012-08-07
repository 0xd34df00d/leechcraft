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
#include <QStandardItemModel>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QtDebug>
#include <util/util.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"

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
	, Model_ (new QStandardItemModel (this))
	{
		CopyURL_ = new QAction (tr ("Copy URL..."), this);
		connect (CopyURL_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopyURL ()));
		ProlongateFile_ = new QAction (tr ("Prolongate selected"), this);
		connect (ProlongateFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flProlongate ()));
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
		EmptyTrash_  = new QAction (tr ("Empty trash"), this);
		connect (EmptyTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flEmptyTrash ()));

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
			}
		}
		if (Ui_.AccountsBox_->count ())
			on_AccountsBox__activated (0);

		Ui_.FilesTree_->setContextMenuPolicy (Qt::CustomContextMenu);
		connect (Ui_.FilesTree_,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleContextMenuRequested (const QPoint&)));
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
		for (int i = 0; i < Model_->rowCount (parent); ++i)
		{
			QStandardItem *item = !parent.isValid () ?
				Model_->item (i) :
				Model_->itemFromIndex (parent)->child (i);

			const auto& index = Model_->indexFromItem (item);
			Account2ItemExpandState_ [GetCurrentAccount ()]
					.insert (item->data (ListingRole::ID).toString (),
							Ui_.FilesTree_->isExpanded (index));

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
				sfl->GetListingOps () & ListingOp::TrashSupporing;

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

	void ManagerTab::handleGotFileUrl (const QUrl& url, const QList<QStringList>& id)
	{
		if (url.isEmpty () || !url.isValid ())
			return;

		const QString& str = url.toString ();
		qApp->clipboard ()->setText (str, QClipboard::Clipboard);
		qApp->clipboard ()->setText (str, QClipboard::Selection);

		QString text = tr ("File shared with URL: %1, the URL was copied to the clipboard")
				.arg (url.toString ());
		emit gotEntity (Util::MakeNotification ("NetStoreManager", text, PInfo_));
	}

	void ManagerTab::flCopyURL ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->RequestUrl (ids); });
	}

	void ManagerTab::flProlongate ()
	{
		CallOnSelection ([] (ISupportFileListings *sfl, const QList<QStringList>& ids) { sfl->Prolongate (ids); });
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
		ProlongateFile_->setEnabled (sfl->GetListingOps () & ListingOp::Prolongate);
		DeleteFile_->setEnabled (sfl->GetListingOps () & ListingOp::Delete);
		MoveToTrash_->setEnabled (sfl->GetListingOps () & ListingOp::TrashSupporing);
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
		if (!index.isValid ())
			return;

		QMenu *menu = new QMenu;
		MoveToTrash_->setEnabled (!index.data (ListingRole::InTrash).toBool ());
		UntrashFile_->setEnabled (index.data (ListingRole::InTrash).toBool ());

		index.data (ListingRole::ID).toString () == "netstoremanager.item_trash" ?
			menu->addAction (EmptyTrash_) :
			menu->addActions ({ CopyURL_, ProlongateFile_,
					MoveToTrash_, UntrashFile_,  DeleteFile_ });

		menu->exec (Ui_.FilesTree_->viewport ()->
				mapToGlobal (QPoint (point.x (), point.y ())));
		menu->deleteLater ();
	}

}
}
