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
#include <interfaces/iauthwidget.h>
#include <interfaces/iaccount.h>
#include "core.h"
#include <QLayout>

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
			Ui_.Services_->addItem (ibs->GetServiceIcon (), ibs->GetServiceName ());
			Ui_.Services_->setItemData (Ui_.Services_->count () - 1,
					QVariant::fromValue<QObject*> (plugin), RServiceObject);

			if (!Service2AuthWidget_.contains (ibs))
			{
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
			}

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

	QModelIndex AccountsSettings::GetServiceIndex (QObject *serviceObj) const
	{
		for (int i = AccountsModel_->rowCount () - 1; i >= 0; --i)
			if (AccountsModel_->item (i)->
					data (RServiceObject).value<QObject*> () == serviceObj)
				return AccountsModel_->index (i, 0);

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
	}

	void AccountsSettings::resizeColumns ()
	{
		for (int i = 0; i < 3; ++i)
			Ui_.AccountsView_->resizeColumnToContents (i);
	}

	void AccountsSettings::on_Add__toggled (bool checked)
	{
		if (LastWidget_)
		{
			Ui_.AuthWidget_->layout ()->removeWidget (LastWidget_);
			LastWidget_->hide ();
		}

		QObject *plugin = Ui_.Services_->itemData (Ui_.Services_->currentIndex (),
				RServiceObject).value<QObject*> ();
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);

		if (ibs->GetFeatures () & IBookmarksService::FCanRegisterAccount)
			Ui_.Register_->show ();
		else
			Ui_.Register_->hide ();

		if (!Service2AuthWidget_.contains (ibs))
			return;

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (Service2AuthWidget_ [ibs]);
		aw->SetIdentifyingData (QVariantMap ());

		if (checked)
		{
			Ui_.AuthWidget_->layout ()->addWidget (Service2AuthWidget_ [ibs]);
			Service2AuthWidget_ [ibs]->show ();
			Ui_.ControlLayout_->insertWidget (1, Ui_.LoginFrame_);
			Ui_.LoginFrame_->show ();
			LastWidget_ = Service2AuthWidget_ [ibs];
		}
		else
		{
			Ui_.AuthWidget_->layout ()->removeWidget (Service2AuthWidget_ [ibs]);
			Service2AuthWidget_ [ibs]->hide ();
			Ui_.ControlLayout_->removeWidget (Ui_.LoginFrame_);
			Ui_.LoginFrame_->hide ();
		}
	}

	void AccountsSettings::on_Delete__clicked ()
	{
		const QModelIndex& current = Ui_.AccountsView_->currentIndex ();
		QStandardItem *item = AccountsModel_->itemFromIndex (current);
		const QModelIndex& parentIndex = current.parent ();
		if (parentIndex == QModelIndex ())
			return;

		if (Ui_.Add_->isChecked ())
			Ui_.Add_->toggle ();

		Core::Instance ().DeletePassword (Item2Account_ [item]->GetObject ());
		emit accountRemoved (Item2Account_ [item]->GetObject ());

		AccountsModel_->removeRow (current.row (), parentIndex);

		if (!AccountsModel_->rowCount (parentIndex))
			AccountsModel_->removeRow (parentIndex.row ());
	}

	void AccountsSettings::on_Auth__clicked ()
	{
		QObject *plugin = Ui_.Services_->itemData (Ui_.Services_->currentIndex (),
				RServiceObject).value<QObject*> ();
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (Service2AuthWidget_ [ibs]);
		if (!aw)
		{
			qWarning () << Q_FUNC_INFO
					<< "auth widget for plugin"
					<< plugin
					<< "is not a IAuthWidget"
					<< aw;
			return;
		}

		ibs->CheckAuthData (aw->GetIdentifyingData ());
	}

	void AccountsSettings::on_Register__clicked ()
	{
		QObject *plugin = Ui_.Services_->itemData (Ui_.Services_->currentIndex (),
				RServiceObject).value<QObject*> ();
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);

		IAuthWidget *aw = qobject_cast<IAuthWidget*> (Service2AuthWidget_ [ibs]);
		if (!aw)
		{
			qWarning () << Q_FUNC_INFO
					<< "auth widget for plugin"
					<< plugin
					<< "is not a IAuthWidget"
					<< aw;
			return;
		}

		ibs->RegisterAccount (aw->GetIdentifyingData ());
	}

	void AccountsSettings::on_AccountsView__clicked (const QModelIndex& index)
	{
		if (index.parent() == QModelIndex ())
			Ui_.Delete_->setEnabled (false);
		else
			Ui_.Delete_->setEnabled (true);
	}

	void AccountsSettings::on_Services__currentIndexChanged (int)
	{
		if (Ui_.Add_->isChecked ())
			Ui_.Add_->toggle ();
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
			QModelIndex index = GetServiceIndex (ibs->GetObject ());
			QStandardItem *parentItem;
			if (index == QModelIndex ())
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
	}

}
}
}
