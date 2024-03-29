/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "managertab.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QInputDialog>
#include <QComboBox>
#include <QToolButton>
#include <QToolBar>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "accountsmanager.h"
#include "filestreemodel.h"
#include "xmlsettingsmanager.h"
#include "filesproxymodel.h"
#include "downmanager.h"
#include "utils.h"

namespace LC
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
	, DownManager_ (new DownManager (proxy, this))
	{
		Ui_.setupUi (this);

		Ui_.FilesView_->setModel (ProxyModel_);
		ProxyModel_->setSourceModel (TreeModel_);
		TreeModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("Used space"), tr ("Modify") });

		const auto header = Ui_.FilesView_->header ();

		header->setSectionResizeMode (Columns::CName, QHeaderView::Interactive);

		connect (Ui_.FilesView_->header (),
				SIGNAL (sectionResized (int, int, int)),
				this,
				SLOT (handleFilesViewSectionResized (int, int, int)));
		Ui_.FilesView_->setContextMenuPolicy (Qt::CustomContextMenu);

		OpenFile_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("system-run"),
				tr ("Open file"), this);
		connect (OpenFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flOpenFile ()));
		CopyURL_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-copy"),
				tr ("Copy URL..."), this);
		connect (CopyURL_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopyUrl ()));
		Copy_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-copy"),
				tr ("Copy..."), this);
		connect (Copy_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopy ()));
		Move_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("transform-move"),
				tr ("Move..."), this);
		connect (Move_,
				SIGNAL (triggered ()),
				this,
				SLOT (flMove ()));
		Rename_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-rename"),
				tr ("Rename..."), this);
		connect (Rename_,
				SIGNAL (triggered ()),
				this,
				SLOT (flRename ()));
		Paste_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-paste"),
				tr ("Paste"), this);
		connect (Paste_,
				SIGNAL (triggered ()),
				this,
				SLOT (flPaste ()));
		DeleteFile_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-delete"),
				tr ("Delete..."), this);
		connect (DeleteFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flDelete ()));
		MoveToTrash_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-clear"),
				tr ("Move to trash"), this);
		connect (MoveToTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flMoveToTrash ()));
		UntrashFile_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("edit-undo"),
				tr ("Restore from trash"), this);
		connect (UntrashFile_,
				SIGNAL (triggered ()),
				this,
				SLOT (flRestoreFromTrash ()));
		EmptyTrash_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("trash-empty"),
				tr ("Empty trash"), this);
		connect (EmptyTrash_,
				SIGNAL (triggered ()),
				this,
				SLOT (flEmptyTrash ()));
		CreateDir_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("folder-new"),
				tr ("Create directory"), this);
		connect (CreateDir_,
				SIGNAL (triggered ()),
				this,
				SLOT (flCreateDir ()));
		UploadInCurrentDir_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("svn-commit"),
				tr ("Upload..."), this);
		connect (UploadInCurrentDir_,
				SIGNAL (triggered ()),
				this,
				SLOT (flUploadInCurrentDir ()));
		Download_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("download"),
				tr ("Download"), this);
		connect (Download_,
				SIGNAL (triggered ()),
				this,
				SLOT (flDownload ()));

		FillToolbar ();

		connect (AM_,
				SIGNAL (accountAdded (QObject*)),
				this,
				SLOT (handleAccountAdded (QObject*)));
		connect (AM_,
				SIGNAL (accountRemoved (QObject*)),
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
				SLOT (performCopy (QList<QByteArray>,QByteArray)));
		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeMoved (QList<QByteArray>, QByteArray)),
				this,
				SLOT (performMove (QList<QByteArray>, QByteArray)));
		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeRestoredFromTrash (QList<QByteArray>)),
				this,
				SLOT (performRestoreFromTrash (QList<QByteArray>)));
		connect (Ui_.FilesView_,
				SIGNAL (itemsAboutToBeTrashed (QList<QByteArray>)),
				this,
				SLOT (performMoveToTrash (QList<QByteArray>)));
		connect (Ui_.FilesView_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (handleReturnPressed ()));
		connect (Ui_.FilesView_,
				SIGNAL (backspacePressed ()),
				this,
				SLOT (handleBackspacePressed ()));
		connect (Ui_.FilesView_,
				SIGNAL (quoteLeftPressed ()),
				this,
				SLOT (handleQuoteLeftPressed ()));
		connect (Ui_.Filter_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (handleFilterTextChanged (QString)));
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
		emit removeTab ();
	}

	QToolBar* ManagerTab::GetToolBar () const
	{
		return ToolBar_;
	}

	void ManagerTab::FillToolbar ()
	{
		AccountsBox_ = new QComboBox (this);
		AccountsBox_->setSizeAdjustPolicy (QComboBox::AdjustToContents);
		for (auto acc : AM_->GetAccounts ())
			AppendAccount (acc);

		ToolBar_->addWidget (AccountsBox_);

		Refresh_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("view-refresh"),
				tr ("Refresh"), this);
		connect (Refresh_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRefresh ()));
		Upload_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("svn-commit"),
				tr ("Upload"), this);
		connect (Upload_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleUpload ()));

		ToolBar_->addActions ({ Refresh_, Util::CreateSeparator (ToolBar_), CreateDir_, Upload_ });
		ToolBar_->addSeparator ();

		OpenTrash_ = new QAction (Proxy_->GetIconThemeManager ()->GetIcon ("user-trash"),
				tr ("Open trash"), this);
		OpenTrash_->setCheckable (true);
		connect (OpenTrash_,
				SIGNAL (triggered (bool)),
				this,
				SLOT (showTrashContent (bool)));

		Trash_ = new QToolButton (this);
		Trash_->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("user-trash"));
		Trash_->setText (tr ("Trash"));
		Trash_->setPopupMode (QToolButton::InstantPopup);
		Trash_->addActions ({ OpenTrash_, EmptyTrash_ });
		ToolBar_->addWidget (Trash_);

		ShowAccountActions (AccountsBox_->count ());

		connect (AccountsBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (handleCurrentIndexChanged (int)));

		const auto& id = XmlSettingsManager::Instance ()
				.property ("LastActiveAccount").toByteArray ();

		int j = 0;
		for (int i = 0; i < AccountsBox_->count (); ++i)
			if (AccountsBox_->itemData (i)
					.value<IStorageAccount*> ()->GetUniqueID () == id)
			{
				j = i;
				break;
			}

		handleCurrentIndexChanged (j);
	}

	void ManagerTab::AppendAccount (IStorageAccount *acc)
	{
		auto stP = qobject_cast<IStoragePlugin*> (acc->GetParentPlugin ());
		AccountsBox_->addItem (Utils::GetStorageIcon (stP),
				acc->GetAccountName (),
				QVariant::fromValue<IStorageAccount*> (acc));
		connect (acc->GetQObject (),
				SIGNAL (downloadFile (QUrl, QString, TaskParameters, bool)),
				DownManager_,
				SLOT (handleDownloadRequest (QUrl, QString, TaskParameters, bool)));
		if (! (acc->GetAccountFeatures () & AccountFeature::FileListings))
			return;

		connect (acc->GetQObject (),
				SIGNAL (listingUpdated (QByteArray)),
				this,
				SLOT (handleListingUpdated (QByteArray)));
		connect (acc->GetQObject (),
				SIGNAL (gotNewItem (StorageItem, QByteArray)),
				this,
				SLOT (handleGotNewItem (StorageItem, QByteArray)));
		connect (acc->GetQObject (),
				SIGNAL (gotChanges (QList<Change>)),
				this,
				SLOT (handleGotChanges (QList<Change>)));
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

	namespace
	{
		QList<QStandardItem*> CreateItems (const StorageItem& storageItem, quint64 folderSize, ICoreProxy_ptr proxy)
		{
			QStandardItem *name = new QStandardItem (storageItem.Name_);
			name->setEditable (false);
			name->setData (storageItem.ID_, ListingRole::ID);
			name->setData (storageItem.Hash_, ListingRole::Hash);
			name->setData (static_cast<int> (storageItem.HashType_),
					ListingRole::HashType);
			name->setData (storageItem.IsDirectory_, ListingRole::IsDirectory);
			name->setData (storageItem.IsTrashed_, ListingRole::InTrash);
			name->setData (storageItem.Name_, SortRoles::SRName);
			QIcon icon = proxy->GetIconThemeManager ()->GetIcon (storageItem.IsDirectory_ ?
					"inode-directory" :
					storageItem.MimeType_);
			if (icon.isNull ())
			{
				qDebug () << "[NetStoreManager]"
						<< "Unknown mime type:"
						<< storageItem.MimeType_
						<< "for file"
						<< storageItem.Name_
						<< storageItem.ID_;
				icon = proxy->GetIconThemeManager ()->GetIcon ("unknown");
			}
			name->setIcon (icon);

			QStandardItem *size = new QStandardItem (Util::MakePrettySize (storageItem
					.IsDirectory_ ? folderSize : storageItem.Size_));
			size->setData (storageItem.IsDirectory_ ? folderSize : storageItem.Size_,
					SortRoles::SRSize);

			size->setEditable (false);

			QStandardItem *modify = new QStandardItem (storageItem.ModifyDate_
					.toString ("dd.MM.yy hh:mm"));
			modify->setEditable (false);
			modify->setData (storageItem.ModifyDate_, SortRoles::SRModifyDate);

			return { name, size, modify };
		}
	}

	void ManagerTab::FillListModel ()
	{
		ShowListItemsWithParent (LastParentID_, OpenTrash_->isChecked ());

		Ui_.FilesView_->header ()->resizeSection (Columns::CName,
				XmlSettingsManager::Instance ().Property ("ViewSectionSize",
						Ui_.FilesView_->header ()->sectionSize (Columns::CName)).toInt ());
	}

	void ManagerTab::RequestFileListings (IStorageAccount *acc)
	{
		const auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		if (!sfl)
		{
			qWarning () << Q_FUNC_INFO
					<< acc
					<< "doesn't support FileListings";
			return;
		}

		Util::Sequence (this, sfl->RefreshListing ())
				.MultipleResults (Util::Visitor
						{
							[this, acc] (const QString& error)
							{
								const auto& e = Util::MakeNotification ("LeechCraft",
										tr ("Unable to get file listing for the account %1: %2.")
												.arg ("<em>" + acc->GetAccountName () + "</em>")
												.arg (error),
										Priority::Critical);
								Proxy_->GetEntityManager ()->HandleEntity (e);
							},
							[this, acc] (const QList<StorageItem>& items)
							{
								if (acc != GetCurrentAccount ())
									return;

								for (auto item : items)
									Id2Item_ [item.ID_] = item;

								const auto iconName = GetTrashedFiles ().isEmpty () ?
									"user-trash-full" :
									"user-trash";
								Trash_->setIcon (Proxy_->GetIconThemeManager ()->GetIcon (iconName));
								ClearModel ();
								FillListModel ();
							}
						});
	}

	void ManagerTab::RequestFileChanges (IStorageAccount*)
	{
		//TODO
	}

	QList<QByteArray> ManagerTab::GetTrashedFiles () const
	{
		QList<QByteArray> result;
		for (const auto& item : Id2Item_)
			if (item.IsTrashed_)
				result << item.ID_;
		return result;
	}

	QList<QByteArray> ManagerTab::GetSelectedIDs () const
	{
		QList<QByteArray> ids;
		for (const auto& idx : Ui_.FilesView_->selectionModel ()->selectedRows ())
		{
			const auto& id = ProxyModel_->mapToSource (idx).data (ListingRole::ID).toByteArray ();
			if (id != "netstoremanager.item_uplevel")
				ids << id;
		}

		return ids;
	}

	QByteArray ManagerTab::GetParentIDInListViewMode () const
	{
		return ProxyModel_->index (0, Columns::CName).data (ListingRole::ParentID)
				.toByteArray ();
	}

	QByteArray ManagerTab::GetCurrentID () const
	{
		QModelIndex idx = Ui_.FilesView_->currentIndex ();
		idx = idx.sibling (idx.row (), Columns::CName);
		return ProxyModel_->mapToSource (idx).data (ListingRole::ID).toByteArray ();
	}

	void ManagerTab::CallOnSelection (std::function<void (ISupportFileListings*, QList<QByteArray>)> func)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		func (sfl, GetSelectedIDs ());
	}

	quint64 ManagerTab::GetFolderSize (const QByteArray& id) const
	{
		quint64 size = 0;
		for (const auto& item : Id2Item_)
		{
			if (item.ParentID_ != id)
				continue;

			if (item.IsTrashed_ != Id2Item_ [id].IsTrashed_)
				continue;

			if (item.IsDirectory_)
				size += GetFolderSize (item.ID_);
			else
				size += item.Size_;
		}
		return size;
	}

	void ManagerTab::ShowListItemsWithParent (const QByteArray& parentId, bool inTrash)
	{
		if (!parentId.isEmpty ())
		{
			QStandardItem *upLevel = new QStandardItem (Proxy_->GetIconThemeManager ()->GetIcon ("go-up"), "..");
			upLevel->setData ("netstoremanager.item_uplevel", ListingRole::ID);
			upLevel->setData (parentId, ListingRole::ParentID);
			upLevel->setEditable (false);
			TreeModel_->appendRow (upLevel);
		}

		for (const auto& item : Id2Item_)
		{
			const quint64 folderSize = item.IsDirectory_ ?
					GetFolderSize (item.ID_) :
					0;
			if (!inTrash &&
					!item.IsTrashed_)
			{
				if (parentId.isEmpty () &&
						!Id2Item_.contains (item.ParentID_))
					TreeModel_->appendRow (CreateItems (item, folderSize, Proxy_));
				else if (!parentId.isEmpty () &&
						item.ParentID_ == parentId)
					TreeModel_->appendRow (CreateItems (item, folderSize, Proxy_));
			}
			else if (inTrash &&
					item.IsTrashed_)
			{
				if (parentId.isEmpty () &&
						(!Id2Item_.contains (item.ParentID_) ||
						!Id2Item_ [item.ParentID_].IsTrashed_))
					TreeModel_->appendRow (CreateItems (item, folderSize, Proxy_));
				else if (!parentId.isEmpty () &&
						item.ParentID_ == parentId &&
						Id2Item_ [parentId].IsTrashed_)
					TreeModel_->appendRow (CreateItems (item, folderSize, Proxy_));
				else if (!parentId.isEmpty () &&
						!Id2Item_ [parentId].IsTrashed_)
					ShowListItemsWithParent (QByteArray (), true);
			}
		}

		Ui_.FilesView_->setCurrentIndex (ProxyModel_->index (0, 0));
	}

	void ManagerTab::handleRefresh ()
	{
		auto acc = GetCurrentAccount ();
		if (!acc)
			return;

		RequestFileListings (acc);
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

		QByteArray parentId = GetParentIDInListViewMode ();

		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file for upload"),
				XmlSettingsManager::Instance ()
						.Property ("DirUploadFrom", QDir::homePath ()).toString ());
		if (filename.isEmpty ())
			return;

		XmlSettingsManager::Instance ().setProperty ("DirUploadFrom",
				QFileInfo (filename).dir ().absolutePath ());

		emit uploadRequested (acc, filename, parentId);
	}

	void ManagerTab::handleDoubleClicked (const QModelIndex& idx)
	{
		auto isa = GetCurrentAccount ();
		if (!isa)
			return;

		if (idx.data (ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
		{
			if (auto isfl = qobject_cast<ISupportFileListings*> (isa->GetQObject ()))
			{
				LastParentID_ = Id2Item_ [idx.data (ListingRole::ParentID).toByteArray ()].ParentID_;
				isfl->RefreshChildren (LastParentID_);
			}
		}
		else if (!idx.data (ListingRole::IsDirectory).toBool ())
			flOpenFile ();
		else if (auto isfl = qobject_cast<ISupportFileListings*> (isa->GetQObject ()))
		{
			LastParentID_ = idx.data (ListingRole::ID).toByteArray ();
			isfl->RefreshChildren (LastParentID_);
		}
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

		AppendAccount (acc);

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

	void ManagerTab::handleListingUpdated (const QByteArray& parentId)
	{
		if (LastParentID_ == parentId || parentId.isEmpty ())
		{
			ClearModel ();
			FillListModel ();
		}
	}

	void ManagerTab::handleGotNewItem (const StorageItem& item, const QByteArray&)
	{
		Id2Item_ [item.ID_] = item;
		LastParentID_ = GetParentIDInListViewMode ();
	}

	void ManagerTab::handleFilesViewSectionResized (int index,
			int, int newSize)
	{
		if (index == Columns::CName)
			XmlSettingsManager::Instance ().setProperty ("ViewSectionSize", newSize);
	}

	void ManagerTab::performCopy (const QList<QByteArray>& ids,
			const QByteArray& newParentId)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->Copy (ids, newParentId);
	}

	void ManagerTab::performMove (const QList<QByteArray>& ids,
			const QByteArray& newParentId)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->Move (ids, newParentId);
	}

	void ManagerTab::performRestoreFromTrash (const QList<QByteArray>& ids)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->RestoreFromTrash (ids);
	}

	void ManagerTab::performMoveToTrash (const QList<QByteArray>& ids)
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		sfl->MoveToTrash (ids);
	}

	void ManagerTab::handleReturnPressed ()
	{
		handleDoubleClicked (Ui_.FilesView_->currentIndex ());
	}

	void ManagerTab::handleBackspacePressed ()
	{
		const auto& index = ProxyModel_->index (0, 0);
		if (index.data (ListingRole::ID).toByteArray () != "netstoremanager.item_uplevel")
			return;

		const auto& id = index.data (Qt::UserRole + 1).toByteArray ();
		ClearModel ();
		ShowListItemsWithParent (Id2Item_ [id].ParentID_, OpenTrash_->isChecked ());
	}

	void ManagerTab::handleQuoteLeftPressed ()
	{
		ClearModel ();
		ShowListItemsWithParent (QByteArray (), OpenTrash_->isChecked ());
	}

	void ManagerTab::flOpenFile ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		TaskParameters tp = OnlyDownload |
				AutoAccept |
				DoNotNotifyUser |
				DoNotSaveInHistory |
				FromUserInitiated;
		auto idx = Ui_.FilesView_->currentIndex ();
		idx = idx.sibling (idx.row (), Columns::CName);
		idx = ProxyModel_->mapToSource (idx);
		acc->Download (idx.data (ListingRole::ID).toByteArray (),
				idx.data ().toString (), tp, true);
	}

	void ManagerTab::flCopy ()
	{
		TransferedIDs_ = { TransferOperation::Copy, GetSelectedIDs () };
	}

	void ManagerTab::flMove ()
	{
		TransferedIDs_ = { TransferOperation::Move, GetSelectedIDs () };
	}

	void ManagerTab::flRename ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		const QString& oldName = Ui_.FilesView_->currentIndex ().data ().toString ();
		const auto& id = GetCurrentID ();
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
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());

		switch (TransferedIDs_.first)
		{
		case TransferOperation::Copy:
			sfl->Copy (TransferedIDs_.second, GetParentIDInListViewMode ());
			break;
		case TransferOperation::Move:
			sfl->Move (TransferedIDs_.second, GetParentIDInListViewMode ());
			for (const auto& id : TransferedIDs_.second)
				Id2Item_.remove (id);
			break;
		}
		TransferedIDs_.second.clear ();
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

		sfl->CreateDirectory (name, GetParentIDInListViewMode ());
	}

	void ManagerTab::flUploadInCurrentDir ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		if (!(sfl->GetListingOps () & ListingOp::DirectorySupport))
			return;

		QByteArray parentId = GetParentIDInListViewMode ();
		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file for upload"),
				XmlSettingsManager::Instance ().Property ("DirUploadFrom", QDir::homePath ()).toString ());
		if (filename.isEmpty ())
			return;
		XmlSettingsManager::Instance ().setProperty ("DirUploadFrom",
				QFileInfo (filename).dir ().absolutePath ());

		emit uploadRequested (acc, filename, parentId);
	}

	void ManagerTab::flDownload ()
	{
		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		const auto& rows = Ui_.FilesView_->selectionModel ()->selectedRows ();
		if (rows.size () < 1)
			return;

		if (rows.size () == 1)
		{
			auto idx = rows.at (0);
			idx = idx.sibling (idx.row (), Columns::CName);
			idx = ProxyModel_->mapToSource (idx);
			acc->Download (idx.data (ListingRole::ID).toByteArray (),
					idx.data ().toString (),
					OnlyDownload | FromUserInitiated,
					false);
			return;
		}

		const auto& defaultDir = XmlSettingsManager::Instance ()
				.Property ("DirMultiDownload", QDir::homePath ()).toString ();
		const auto& dir = QFileDialog::getExistingDirectory (this,
				tr ("Download %n file(s)", 0, rows.size ()),
				defaultDir);
		if (dir.isEmpty ())
			return;

		XmlSettingsManager::Instance ().setProperty ("DirMultiDownload", dir);

		for (auto row : rows)
		{
			row = row.sibling (row.row (), Columns::CName);
			row = ProxyModel_->mapToSource (row);
			acc->Download (row.data (ListingRole::ID).toByteArray (),
					dir + "/" + row.data ().toString (),
					OnlyDownload | FromUserInitiated | AutoAccept,
					false);
		}
	}

	void ManagerTab::flCopyUrl ()
	{
		const auto acc = GetCurrentAccount ();
		if (!acc)
			return;

		const auto urlHandler = [this] (const QUrl& url)
		{
			const auto& str = url.toString ();
			qApp->clipboard ()->setText (str, QClipboard::Clipboard);
			qApp->clipboard ()->setText (str, QClipboard::Selection);

			const auto& text = tr ("File URL has been copied to the clipboard.");
			Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("NetStoreManager", text, Priority::Info));
		};

		const auto& id = GetCurrentID ();
		if (Id2Item_ [id].Shared_)
		{
			urlHandler (Id2Item_ [id].ShareUrl_);
			return;
		}

		const auto isfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		Util::Sequence (this, isfl->RequestUrl (id)) >>
				Utils::HandleRequestFileUrlResult (Proxy_->GetEntityManager (),
						tr ("Unable to request file URL."),
						urlHandler);
	}

	void ManagerTab::showTrashContent (bool show)
	{
		OpenTrash_->setText (show ? tr ("Close trash") : tr ("Open trash"));
		ClearModel ();
		ShowListItemsWithParent (QByteArray (), show);
	}

	void ManagerTab::handleContextMenuRequested (const QPoint& point)
	{
		QList<QModelIndex> idxs = Ui_.FilesView_->selectionModel ()->selectedRows ();
		for (int i = idxs.count () - 1; i >= 0; --i)
			if (idxs.at (i).data (ListingRole::ID).toByteArray () == "netstoremanager.item_uplevel")
			{
				idxs.removeAt (i);
				break;
			}

		IStorageAccount *acc = GetCurrentAccount ();
		if (!acc)
			return;

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		const bool trashSupport = sfl->GetListingOps () & ListingOp::TrashSupporting;

		QMenu *menu = new QMenu;
		UploadInCurrentDir_->setText (tr ("Upload..."));

		if (!idxs.isEmpty ())
		{
			QList<QAction*> editActions = { Copy_, Move_, Rename_, DeleteFile_ };

			menu->addAction (CopyURL_);
			menu->addSeparator ();

			menu->addAction (Download_);
			menu->addSeparator ();
			QList<QAction*> actions;
			if (trashSupport &&
					idxs.at (0).data (ListingRole::InTrash).toBool ())
				actions << UntrashFile_;
			else
			{
				menu->insertAction (Download_, UploadInCurrentDir_);
				actions << editActions;
				if (trashSupport)
					actions << MoveToTrash_;
			}
			actions << DeleteFile_;
			menu->addActions (actions);

			const auto& id = GetCurrentID ();
			StorageItem item;
			if (Id2Item_.contains (id))
				item = Id2Item_ [id];

			if (item.IsValid () &&
					!item.IsDirectory_)
			{
				menu->insertAction (CopyURL_, OpenFile_);
				menu->insertSeparator (CopyURL_);
			}

			if (item.IsValid () &&
					!item.ExportLinks.isEmpty ())
			{
				QMenu *exportMenu = new QMenu (tr ("Export to..."), menu);
				auto exportAct = menu->insertMenu (Download_, exportMenu);
				exportAct->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("document-export"));
				for (const auto& key : item.ExportLinks.keys ())
				{
					const auto& pair = item.ExportLinks [key];
					QAction *action = new QAction (Proxy_->GetIconThemeManager ()->GetIcon (pair.first),
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
		else
			menu->addAction (UploadInCurrentDir_);

		if (!TransferedIDs_.second.isEmpty () &&
				!OpenTrash_->isChecked ())
		{
			auto sep = menu->insertSeparator (menu->actions ().at (0));
			menu->insertAction (sep, Paste_);
		}

		menu->addSeparator ();
		menu->addAction (CreateDir_);

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

	void ManagerTab::handleCurrentIndexChanged (int)
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

		Id2Item_.clear ();
		LastParentID_.clear ();
		RequestFileListings (acc);

		auto sfl = qobject_cast<ISupportFileListings*> (acc->GetQObject ());
		DeleteFile_->setVisible (sfl->GetListingOps () & ListingOp::Delete);
		MoveToTrash_->setVisible (sfl->GetListingOps () & ListingOp::TrashSupporting);
		UntrashFile_->setVisible (sfl->GetListingOps () & ListingOp::TrashSupporting);
		Trash_->setVisible (sfl->GetListingOps () & ListingOp::TrashSupporting);

		XmlSettingsManager::Instance ().setProperty ("LastActiveAccount",
				acc->GetUniqueID ());
	}

	void ManagerTab::handleGotChanges(const QList<Change>& changes)
	{
		for (const auto& change : changes)
		{
			if (change.Deleted_)
				Id2Item_.remove (change.ItemID_);
			else if (change.Item_.IsValid ())
				Id2Item_ [change.ItemID_] = change.Item_;
		}

		LastParentID_ = GetParentIDInListViewMode ();
	}

	void ManagerTab::handleFilterTextChanged (const QString& text)
	{
		ProxyModel_->setFilterFixedString (text);
	}

}
}
