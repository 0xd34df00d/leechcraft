/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "syncwidget.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "accountsmanager.h"
#include "syncitemdelegate.h"
#include "xmlsettingsmanager.h"
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	SyncWidget::SyncWidget (AccountsManager *am, QWidget *parent)
	: QWidget (parent)
	, AM_ (am)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Model_->setHorizontalHeaderLabels ({ tr ("Account"), tr ("Directory") });
		Ui_.SyncView_->horizontalHeader ()->setStretchLastSection (true);
		Ui_.SyncView_->setItemDelegate (new SyncItemDelegate (AM_, this));
		Ui_.SyncView_->setModel (Model_);
	}

	void SyncWidget::RestoreData ()
	{
		QVariantMap map = XmlSettingsManager::Instance ().property ("Synchronization").toMap ();
		for (auto key : map.keys ())
		{
			auto isa = AM_->GetAccountFromUniqueID (key);
			if (!isa)
			{
				qWarning () << Q_FUNC_INFO
						<< "there is no account with ID"
						<< key;
				continue;
			}
			auto isp = qobject_cast<IStoragePlugin*> (isa->GetParentPlugin ());
			if (!isp)
				continue;

			QStandardItem *accItem = new QStandardItem;
			accItem->setData (isp->GetStorageName () + ": " + isa->GetAccountName (),
					Qt::EditRole);
			accItem->setData (key, SyncItemDelegate::AccountId);
			QStandardItem *dirItem = new QStandardItem;
			dirItem->setData (map [key].toString (), Qt::EditRole);
			Model_->appendRow ({ accItem, dirItem });

			Ui_.SyncView_->openPersistentEditor (Model_->indexFromItem (accItem));
			Ui_.SyncView_->resizeColumnToContents (SyncItemDelegate::Account);
		}

		emit directoryAdded (map);
	}

	void SyncWidget::accept ()
	{
		QVariantMap map;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			QStandardItem *accItem = Model_->item (i, SyncItemDelegate::Account);
			QStandardItem *dirItem = Model_->item (i, SyncItemDelegate::Directory);
			if (dirItem->text ().isEmpty () ||
					accItem->text ().isEmpty ())
				continue;

			map [accItem->data (SyncItemDelegate::AccountId).toString ()] = dirItem->text ();
		}

		emit directoryAdded (map);
		XmlSettingsManager::Instance ().setProperty ("Synchronization", map);
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

