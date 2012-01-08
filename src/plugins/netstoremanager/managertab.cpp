/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include "interfaces/netstoremanager/istorageaccount.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "accountsmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	ManagerTab::ManagerTab (const TabClassInfo& tc, AccountsManager *am, QObject *obj)
	: Parent_ (obj)
	, Info_ (tc)
	, AM_ (am)
	, Model_ (new QStandardItemModel (this))
	{
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
			}
		}
		if (Ui_.AccountsBox_->count ())
			on_AccountsBox__activated (0);

		QAction *copyURL = new QAction (tr ("Copy URL..."), this);
		connect (copyURL,
				SIGNAL (triggered ()),
				this,
				SLOT (flCopyURL ()));
		Ui_.FilesTree_->addAction (copyURL);
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

	void ManagerTab::handleGotListing (const QList<QList<QStandardItem*>>& items)
	{
		const int idx = Ui_.AccountsBox_->currentIndex ();
		if (idx < 0 || items.isEmpty ())
			return;

		IStorageAccount *acc = Ui_.AccountsBox_->
				itemData (idx).value<IStorageAccount*> ();
		if (sender () != acc->GetObject ())
			return;

		QFontMetrics fm = Ui_.FilesTree_->fontMetrics ();

		QMap<int, int> sizes;
		Q_FOREACH (auto row, items)
		{
			Model_->appendRow (row);

			for (int i = 0; i < row.size (); ++i)
				sizes [i] = std::max (sizes [i], fm.width (row.at (i)->text ()));
		}

		Q_FOREACH (auto idx, sizes.keys ())
		{
			auto hdr = Ui_.FilesTree_->header ();
			const int size = hdr->sectionSize (idx);
			if (size < sizes [idx])
				hdr->resizeSection (idx, sizes [idx]);
		}
	}

	void ManagerTab::flCopyURL ()
	{
		auto item = Model_->itemFromIndex (Ui_.FilesTree_->currentIndex ());
		if (!item)
			return;

		const QUrl& url = item->data (ListingRole::URL).toUrl ();
		if (url.isEmpty () || !url.isValid ())
			return;

		const QString& str = url.toString ();
		qApp->clipboard ()->setText (str, QClipboard::Clipboard);
		qApp->clipboard ()->setText (str, QClipboard::Selection);
	}

	void ManagerTab::on_AccountsBox__activated (int index)
	{
		if (index < 0)
			return;

		IStorageAccount *acc = Ui_.AccountsBox_->
				itemData (index).value<IStorageAccount*> ();
		const bool hasListings = acc->GetAccountFeatures () & AccountFeature::FileListings;
		Ui_.Update_->setEnabled (hasListings);
		if (!hasListings)
			return;

		on_Update__released ();
	}

	void ManagerTab::on_Update__released ()
	{
		const int idx = Ui_.AccountsBox_->currentIndex ();
		if (idx < 0)
			return;

		Model_->clear ();

		IStorageAccount *acc = Ui_.AccountsBox_->
				itemData (idx).value<IStorageAccount*> ();
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
}
}
