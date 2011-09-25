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
#include <interfaces/iauthwidget.h>
#include "core.h"
#include "interfaces/iaccount.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	AccountsSettings::AccountsSettings ()
	: AccountsModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.AccountsView_->setModel (AccountsModel_);
		Ui_.AccountsView_->expandAll ();

		Ui_.Delete_->setEnabled (false);

		Ui_.LoginFrame_->hide ();
		Ui_.Register_->hide ();
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
					SIGNAL (accountAdded (QObject*)),
					this,
					SLOT (addAccount (QObject*)));
			connect (this,
					SIGNAL (accountRemoved (QObject*)),
					ibs->GetObject (),
					SLOT (removeAccount (QObject*)));
		}
	}

	QModelIndex AccountsSettings::GetServiceIndex (QObject *serviceObj) const
	{
		for (int i = 0; i < AccountsModel_->rowCount (); ++i)
			if (AccountsModel_->item (i)->
						data (RServiceObject).value<QObject*> () == serviceObj)
				return AccountsModel_->index (i, 0);

			return QModelIndex ();
	}

	void AccountsSettings::DeletePassword (QObject* accObj)
	{
		IAccount *account = qobject_cast<IAccount*> (accObj);
		QVariantList keys;
		keys << account->GetAccountID ();

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-clear");
		emit gotEntity (e);
	}

	void AccountsSettings::SavePassword (QObject *accObj)
	{
		QVariantList keys;
		IAccount *account = qobject_cast<IAccount*> (accObj);
		keys << account->GetAccountID ();

		QVariantList passwordVar;
		passwordVar << account->GetPassword ();
		QVariantList values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		emit gotEntity (e);
	}

	QString AccountsSettings::GetPassword (QObject *accObj)
	{
		QVariantList keys;
		IAccount *account = qobject_cast<IAccount*> (accObj);
		keys << account->GetAccountID ();
		
		const QVariantList& result = Util::GetPersistentData (keys, this);
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}

	void AccountsSettings::accept ()
	{
	}

	void AccountsSettings::on_Add__toggled (bool checked)
	{
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
		const QModelIndex& parentIndex = current.parent ();
		if (parentIndex == QModelIndex ())
			return;

		if (Ui_.Add_->isChecked ())
			Ui_.Add_->toggle ();

		QObject *accObj = AccountsModel_->itemFromIndex (current)->
				data (RAccountObject).value<QObject*> ();
		DeletePassword (accObj);
		emit accountRemoved (accObj);
		
		AccountsModel_->removeRow (current.row (), parentIndex);
		
		if (!AccountsModel_->rowCount (parentIndex))
			AccountsModel_->removeRows (parentIndex.row (), 0);
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

	void AccountsSettings::on_Services__currentIndexChanged (int index)
	{
		if (Ui_.Add_->isChecked ())
			Ui_.Add_->toggle ();
	}

	void AccountsSettings::addAccount (QObject* accObj)
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (sender ());
		if (!ibs)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "ins't IBookmarksService";
			return;
		}

		IAccount *account = qobject_cast<IAccount*> (accObj);
		if (Id2Account_.contains (account->GetAccountID ()))
			return;

		SavePassword (accObj);
		Id2Account_ [account->GetAccountID ()] = accObj;
		QModelIndex index = GetServiceIndex (ibs->GetObject ());
		QStandardItem *parentItem;
		if (index == QModelIndex ())
		{
			parentItem = new QStandardItem (ibs->GetServiceIcon (), ibs->GetServiceName ());
			parentItem->setEditable (false);
			parentItem->setData (QVariant::fromValue<QObject*> (sender ()), RServiceObject);
			AccountsModel_->appendRow (parentItem);
		}
		else
			parentItem = AccountsModel_->itemFromIndex (index);

		QStandardItem *item = new QStandardItem (account->GetLogin ());
		item->setData (QVariant::fromValue<QObject*> (accObj), RAccountObject);
		item->setEditable (false);
		parentItem->appendRow (item);
	}

}
}
}
