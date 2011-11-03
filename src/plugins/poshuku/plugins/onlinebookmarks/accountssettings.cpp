/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "accountssettings.h"
#include <QStandardItemModel>
#include <QDateTime>
#include <QTimer>
#include <QLayout>
#include <interfaces/iauthwidget.h>
#include <interfaces/iaccount.h>
#include "core.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	AccountsSettings::AccountsSettings ()
	: AccountsModel_ (new QStandardItemModel (this))
	, Scheduled_ (false)
	, LastWidget_ (0)
	{
		Ui_.setupUi (this);

		Ui_.AccountsView_->setModel (AccountsModel_);

		AccountsModel_->setHorizontalHeaderLabels (QStringList () << tr ("Account")
				<< tr ("Last upload date")
				<< tr ("Last download date"));
		Ui_.Delete_->setEnabled (false);
		Ui_.Register_->hide ();
		Ui_.LoginFrame_->hide ();
	}

	AccountsSettings::~AccountsSettings ()
	{
		qDeleteAll (Service2AuthWidget_);
	}

	void AccountsSettings::InitServices ()
	{
		Q_FOREACH (QObject *plugin, Core::Instance ().GetServicePlugins ())
		{
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);
			QAction *act = new QAction (ibs->GetServiceIcon (), ibs->GetServiceName (), this);
			Action2Service_ [act] = ibs;
			Ui_.AddAccount_->addAction (act);

			QWidget *widget = ibs->GetAuthWidget ();
			if (!qobject_cast<IAuthWidget*> (widget))
			{
				qWarning () << Q_FUNC_INFO
						<< "auth widget for plugin"
						<< plugin
						<< "is not a IAuthWidget"
						<< widget;
				continue;
			}

			Service2AuthWidget_ [ibs] = widget;

			connect (ibs->GetObject (),
					SIGNAL (accountAdded (QObjectList)),
					this,
					SLOT (addAccount (QObjectList)));

			connect (this,
					SIGNAL (accountRemoved (QObject*)),
					ibs->GetObject (),
					SLOT (removeAccount (QObject*)));
		}
	}

	QStandardItemModel* AccountsSettings::GetAccountsModel () const
	{
		return AccountsModel_;
	}

	void AccountsSettings::UpdateDates ()
	{
		Q_FOREACH (QStandardItem *item, Item2Account_.keys ())
		{
			int row = item->row ();
			QStandardItem * parentItem = item->parent ();
			parentItem->child ( row, 1)->
					setText (Item2Account_ [item]->GetLastUploadDateTime ()
						.toString (Qt::DefaultLocaleShortDate));
			parentItem->child ( row, 2)->
					setText (Item2Account_ [item]->GetLastDownloadDateTime ()
						.toString (Qt::DefaultLocaleShortDate));
		}
	}

	QModelIndex AccountsSettings::GetServiceIndex (QObject *serviceObj) const
	{
		Q_FOREACH (QStandardItem *item, Item2Service_.keys ())
			if (Item2Service_ [item] == qobject_cast<IBookmarksService*> (serviceObj))
				return item->index ();

		return QModelIndex ();
	}

	void AccountsSettings::ScheduleResize ()
	{
		if (Scheduled_)
			return;

		QTimer::singleShot (100, this, SLOT (resizeColumns ()));
		Scheduled_ = true;
	}

	void AccountsSettings::accept ()
	{
		QObjectList accounts;
		Q_FOREACH (QStandardItem *item,  Item2Account_.keys ())
			if (item->checkState () == Qt::Checked)
			{
				Item2Account_ [item]->SetSyncing (true);
				accounts << Item2Account_ [item]->GetObject ();
			}

		Q_FOREACH (IBookmarksService *service, Item2Service_.values ())
			service->saveAccounts ();

		Core::Instance ().SetActiveAccounts (accounts);

		Core::Instance ().SetQuickUploadButtons ();
	}

	void AccountsSettings::resizeColumns ()
	{
		for (int i = 0; i < 3; ++i)
			Ui_.AccountsView_->resizeColumnToContents (i);
	}

	void AccountsSettings::on_Delete__clicked ()
	{
		const QModelIndex& current = Ui_.AccountsView_->currentIndex ();
		const int row = current.row ();
		const QModelIndex& parentIndex = current.parent ();
		if (parentIndex == QModelIndex ())
			return;

		QStandardItem *item = AccountsModel_->itemFromIndex (parentIndex)->child (row, 0);

		Core::Instance ().DeletePassword (Item2Account_ [item]->GetObject ());
		emit accountRemoved (Item2Account_ [item]->GetObject ());
		Core::Instance ().removeAccount (Item2Account_ [item]->GetObject ());

		AccountsModel_->removeRow (current.row (), parentIndex);
		Item2Account_.remove (item);
		if (!AccountsModel_->rowCount (parentIndex))
		{
			Item2Service_.remove (AccountsModel_->itemFromIndex (parentIndex));
			AccountsModel_->removeRow (parentIndex.row ());
		}
	}

	void AccountsSettings::on_Auth__clicked ()
	{
		IBookmarksService *ibs = Service2AuthWidget_.key (LastWidget_);
		if (!ibs)
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (LastWidget_);
		if (!aw)
		{
			qWarning () << Q_FUNC_INFO
					<< "is not a IAuthWidget"
					<< LastWidget_;
			return;
		}

		ibs->CheckAuthData (aw->GetIdentifyingData ());
	}

	void AccountsSettings::on_Register__clicked ()
	{
		IBookmarksService *ibs = Service2AuthWidget_.key (LastWidget_);
		if (!ibs)
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (LastWidget_);
		if (!aw)
		{
			qWarning () << Q_FUNC_INFO
			<< "is not a IAuthWidget"
			<< LastWidget_;
			return;
		}

		ibs->RegisterAccount (aw->GetIdentifyingData ());
	}

	void AccountsSettings::on_AccountsView__clicked (const QModelIndex& index)
	{
		Ui_.Delete_->setEnabled (index.parent().isValid ());
	}

	void AccountsSettings::on_AddAccount__triggered (QAction *action)
	{
		if (!Action2Service_.contains (action))
			return;

		if (LastWidget_)
		{
			Ui_.AuthWidget_->layout ()->removeWidget (LastWidget_);
			LastWidget_->hide ();
		}

		IBookmarksService *ibs = Action2Service_ [action];

		Ui_.Register_->setVisible (ibs->GetFeatures () &
			IBookmarksService::FCanRegisterAccount);

		if (!Service2AuthWidget_.contains (ibs))
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (Service2AuthWidget_ [ibs]);
		aw->SetIdentifyingData (QVariantMap ());

		Ui_.AuthWidget_->layout ()->addWidget (Service2AuthWidget_ [ibs]);
		Service2AuthWidget_ [ibs]->show ();
		Ui_.ControlLayout_->insertWidget (1, Ui_.LoginFrame_);
		Ui_.LoginFrame_->show ();
		LastWidget_ = Service2AuthWidget_ [ibs];
	}

	void AccountsSettings::on_Close__clicked ()
	{
		Ui_.AuthWidget_->layout ()->removeWidget (LastWidget_);
		LastWidget_->hide ();
		Ui_.ControlLayout_->removeWidget (Ui_.LoginFrame_);
		Ui_.LoginFrame_->hide ();
	}

	void AccountsSettings::addAccount (QObjectList accObjects)
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "ins't IBookmarksService";
			return;
		}

		QObjectList accounts;
		Q_FOREACH (QObject *accObj, accObjects)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			if (Id2Account_.contains (account->GetAccountID ()))
				continue;

			if (!account->GetPassword ().isEmpty ())
				Core::Instance ().SavePassword (accObj);
			else
				account->SetPassword (Core::Instance ().GetPassword (accObj));

			Id2Account_ [account->GetAccountID ()] = accObj;

			accounts << accObj;

			const QModelIndex& index = GetServiceIndex (ibs->GetObject ());
			QStandardItem *parentItem = 0;
			if (!index.isValid ())
			{
				parentItem = new QStandardItem (ibs->GetServiceIcon (), ibs->GetServiceName ());
				parentItem->setEditable (false);
				Item2Service_ [parentItem] = ibs;
				AccountsModel_->appendRow (parentItem);
			}
			else
				parentItem = AccountsModel_->itemFromIndex (index);

			QList<QStandardItem*> record;
			QStandardItem *item = new QStandardItem (account->GetLogin ());
			item->setEditable (false);
			item->setCheckable (true);
			item->setCheckState (account->IsSyncing () ? Qt::Checked : Qt::Unchecked);
			Item2Account_ [item] = account;

			QStandardItem *uploaditem = new QStandardItem (account->GetLastUploadDateTime ()
					.toString (Qt::DefaultLocaleShortDate));
			uploaditem->setEditable (false);

			QStandardItem *downloaditem = new QStandardItem (account->GetLastDownloadDateTime ()
					.toString (Qt::DefaultLocaleShortDate));
			uploaditem->setEditable (false);

			record << item
					<< uploaditem
					<< downloaditem;
			parentItem->appendRow (record);

			if (account->IsSyncing ())
				Core::Instance ().AddActiveAccount (accObj);

			Ui_.AccountsView_->expandAll ();

			Scheduled_ = false;
			ScheduleResize ();
		}

		Core::Instance ().AddAccounts (accounts);
	}

}
}
}
