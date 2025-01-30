/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "remotedirectoryselectdialog.h"
#include <QMessageBox>
#include <QStandardItemModel>
#include <QPushButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QtDebug>
#include <util/threads/futures.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/iiconthememanager.h>
#include "interfaces/netstoremanager/istorageaccount.h"
#include "accountsmanager.h"
#include "filesproxymodel.h"
#include "utils.h"

namespace LC
{
namespace NetStoreManager
{
	RemoteDirectorySelectDialog::RemoteDirectorySelectDialog (const QByteArray& accId,
			AccountsManager *am, QWidget *parent)
	: QDialog (parent)
	, AccountId_ (accId)
	, Model_ (new QStandardItemModel (this))
	, ProxyModel_ (new FilesProxyModel (this))
	, AM_ (am)
	{
		Ui_.setupUi (this);

		const auto iconMgr = AM_->GetProxy ()->GetIconThemeManager ();
		auto createDir = new QPushButton (iconMgr->GetIcon ("folder-new"),
				tr ("New directory..."));
		Ui_.ButtonBox_->addButton (createDir, QDialogButtonBox::ActionRole);
		connect (createDir,
				SIGNAL (clicked ()),
				this,
				SLOT (createNewDir ()));

		Model_->setHorizontalHeaderLabels ({ tr ("Directory") });
		ProxyModel_->setSourceModel (Model_);
		Ui_.DirectoriesView_->setModel (ProxyModel_);
		if (auto account = am->GetAccountFromUniqueID (accId))
			if (auto isfl = qobject_cast<ISupportFileListings*> (account->GetQObject ()))
				Util::Sequence (this, isfl->RefreshListing ())
						.MultipleResults ([this] (const ISupportFileListings::RefreshResult_t& result)
								{ HandleGotListing (result); });
	}

	QStringList RemoteDirectorySelectDialog::GetDirectoryPath () const
	{
		auto index = Ui_.DirectoriesView_->currentIndex ();
		if (!index.isValid ())
			return QStringList ();

		QStringList path;
		do
		{
			path.prepend (index.data ().toString ());
			index = index.parent ();
		} while (index.isValid ());

		return path;
	}

	void RemoteDirectorySelectDialog::HandleGotListing (const ISupportFileListings::RefreshResult_t& items)
	{
		Util::Visit (items,
				[this] (const QString& error)
				{
					QMessageBox::critical (this,
							"LeechCraft",
							tr ("Unable to get file listing for the account.") + " " + error);
				},
				[this] (const QList<StorageItem>& items)
				{
					QHash<QByteArray, StorageItem> id2Item;
					QHash<QByteArray, QStandardItem*> id2StandardItem;
					for (const auto& item : items)
					{
						if (!item.IsDirectory_ ||
								item.IsTrashed_)
							continue;

						id2Item [item.ID_] = item;
						QStandardItem *dir = new QStandardItem (AM_->GetProxy ()->
								GetIconThemeManager ()->GetIcon ("inode-directory"), item.Name_);
						dir->setData (item.ID_, ListingRole::ID);
						dir->setEditable (false);
						id2StandardItem [item.ID_] = dir;
					}

					for (const auto& pair : Util::Stlize (id2StandardItem))
					{
						const auto& key = pair.first;
						const auto& id = id2Item [key].ParentID_;
						if (!id2Item.contains (id))
							Model_->appendRow (pair.second);
						else
							id2StandardItem [id]->appendRow (pair.second);
					}
				});
	}

	void RemoteDirectorySelectDialog::createNewDir ()
	{
		const QString& path = QInputDialog::getText (this,
				"LeechCraft",
				tr ("Enter new directory name:"));

		if (path.isEmpty ())
			return;

		const auto& index = Ui_.DirectoriesView_->currentIndex ();
		QStandardItem *item = new QStandardItem (AM_->GetProxy ()->
				GetIconThemeManager ()->GetIcon ("inode-directory"), path);
		item->setEditable (false);
		if (index.isValid ())
			Model_->itemFromIndex (ProxyModel_->mapToSource (index))->appendRow (item);
		else
			Model_->appendRow (item);
	}

}
}
