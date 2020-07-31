/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "syncwidget.h"
#include <unordered_set>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QtDebug>
#include <QDir>
#include "accountsmanager.h"
#include "syncitemdelegate.h"
#include "xmlsettingsmanager.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "utils.h"
#include "syncer.h"

namespace LC
{
namespace NetStoreManager
{
	SyncWidget::SyncWidget (AccountsManager *am, QWidget *parent)
	: QWidget (parent)
	, AM_ (am)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Account"),
				tr ("Local directory"), tr ("Remote directory") });
		Ui_.SyncView_->horizontalHeader ()->setStretchLastSection (true);
		Ui_.SyncView_->setItemDelegate (new SyncItemDelegate (AM_, Model_, this));
		Ui_.SyncView_->setModel (Model_);
	}

	void SyncWidget::RestoreData ()
	{
		const auto& infos = XmlSettingsManager::Instance ().property ("Synchronization").value<QList<SyncerInfo>> ();
		for (const auto& info : infos)
		{
			auto isa = AM_->GetAccountFromUniqueID (info.AccountId_);
			if (!isa)
			{
				qWarning () << Q_FUNC_INFO
						<< "there is no account with ID"
						<< info.AccountId_;
				continue;
			}
			auto isp = qobject_cast<IStoragePlugin*> (isa->GetParentPlugin ());
			if (!isp)
				continue;

			QStandardItem *accItem = new QStandardItem;
			accItem->setData (isp->GetStorageName () + ": " + isa->GetAccountName (),
					Qt::EditRole);
			accItem->setData (info.AccountId_, SyncItemDelegate::AccountId);
			QStandardItem *localDirItem = new QStandardItem;
			localDirItem->setData (info.LocalDirectory_, Qt::EditRole);
			QStandardItem *remoteDirItem = new QStandardItem;
			remoteDirItem->setData (info.RemoteDirectory_, Qt::EditRole);
			Model_->appendRow ({ accItem, localDirItem, remoteDirItem });

			Ui_.SyncView_->openPersistentEditor (Model_->indexFromItem (accItem));
			Ui_.SyncView_->resizeColumnToContents (SyncItemDelegate::Account);
		}

		RemoveInvalidRows ();
		RemoveDuplicateRows ();

		emit directoriesToSyncUpdated (GetInfos ());
	}

	void SyncWidget::RemoveInvalidRows ()
	{
		for (int i = Model_->rowCount () - 1; i >= 0; --i)
		{
			QStandardItem *accItem = Model_->item (i, SyncItemDelegate::Account);
			QStandardItem *localDirItem = Model_->item (i, SyncItemDelegate::LocalDirectory);
			QStandardItem *remoteDirItem = Model_->item (i, SyncItemDelegate::RemoteDirectory);

			if (!accItem ||  accItem->data (SyncItemDelegate::AccountId).toByteArray ().isEmpty () ||
					!localDirItem || localDirItem->text ().isEmpty () ||
					!remoteDirItem || remoteDirItem->text ().isEmpty ())
				Model_->removeRow (i);
		}
	}

	void SyncWidget::RemoveDuplicateRows ()
	{
		auto hasher = [] (const SyncerInfo& item)
			{ return qHash (item.AccountId_) + qHash (item.LocalDirectory_) +
					qHash (item.RemoteDirectory_); };
		std::unordered_set<SyncerInfo, decltype (hasher)> hash (0, hasher);

		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			const auto& accId = Model_->item (i, SyncItemDelegate::Account)->
					data (SyncItemDelegate::AccountId).toByteArray ();
			const auto& localDir = Model_->item (i,
					SyncItemDelegate::LocalDirectory)->text ();
			const auto& remoteDir = Model_->item (i,
					SyncItemDelegate::RemoteDirectory)->text ();
			SyncerInfo info { accId, localDir, remoteDir };
			if (hash.find (info) == hash.end ())
				hash.insert (info);
			else
				Model_->removeRow (i);
		}
	}

	QList<SyncerInfo> SyncWidget::GetInfos () const
	{
		QList<SyncerInfo> infos;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			SyncerInfo info;
			info.AccountId_ = Model_->item (i, SyncItemDelegate::Account)->
					data (SyncItemDelegate::AccountId).toByteArray ();
			info.LocalDirectory_ = Model_->item (i, SyncItemDelegate::LocalDirectory)->text ();
			info.RemoteDirectory_ = Model_->item (i, SyncItemDelegate::RemoteDirectory)->text ();

			infos << info;
		}

		return infos;
	}

	void SyncWidget::accept ()
	{
		RemoveInvalidRows ();
		RemoveDuplicateRows ();

		auto infos = GetInfos ();
		auto oldInfos = XmlSettingsManager::Instance ().property ("Synchronization")
				.value<QList<SyncerInfo>> ();
		if (oldInfos.count () == infos.count ())
		{
			bool found = false;
			for (auto i1 = oldInfos.begin (), i2 = infos.begin (), e1 = oldInfos.end ();
					i1 != e1; ++i1, ++i2)
				if (i1 != i2)
					found = true;

			if (!found)
				return;
		}
		XmlSettingsManager::Instance ().setProperty ("Synchronization", QVariant::fromValue (infos));

		emit directoriesToSyncUpdated (infos);
	}

	void SyncWidget::on_Add__released ()
	{
		Model_->appendRow ({ new QStandardItem, new QStandardItem });
		Ui_.SyncView_->openPersistentEditor (Model_->index (Model_->rowCount () - 1,
				SyncItemDelegate::Account));
		Ui_.SyncView_->resizeColumnToContents (SyncItemDelegate::Account);
	}

	void SyncWidget::on_Remove__released ()
	{
		const auto& idxList = Ui_.SyncView_->selectionModel ()->selectedIndexes ();
		for (auto idx : idxList)
			Model_->removeRow (idx.row ());
	}
}
}

