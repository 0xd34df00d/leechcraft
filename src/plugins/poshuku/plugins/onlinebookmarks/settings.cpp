/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "settings.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDialog>
#include <QDateTime>
#include <plugininterface/util.h>
#include "onlinebookmarks.h"
#include "readitlater/readitlaterbookmarksservice.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "abstractbookmarksservice.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	Settings::Settings (QStandardItemModel *model, Core *parent)
	: Model_ (model)
	, Core_ (parent)
	{
		Ui_.setupUi (this);

		Ui_.AccountsView_->setModel (model);
		Ui_.AccountsView_->expandAll ();

		Ui_.Edit_->setEnabled (false);

		Ui_.Services_->setCurrentIndex (0);
		Ui_.LoginFrame_->hide ();

		connect (Ui_.Login_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (handleLoginTextChanged (QString)));

		connect (Ui_.Password_,
				SIGNAL (textChanged (QString)),
				this,
				SLOT (handlePasswordTextChanged (QString)));

		connect (Ui_.Apply_,
				SIGNAL (released ()),
				this,
				SLOT (handleStuff ()));

		BookmarksServices_ << new ReadItLaterBookmarksService (this);
		SetupServices ();
		ReadSettings ();
	}

	void Settings::ClearFrameState ()
	{
		Ui_.Login_->setText (QString ());
		Ui_.Password_->setText (QString ());
	}

	void Settings::SetupServices ()
	{
		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Core::Instance ().GetServiceModel ());
		if (!model)
		{
			qWarning() << Q_FUNC_INFO
					<< "model is not QAbstractItemModel"
					<< Core::Instance ().GetServiceModel ();
			return;
		}
		
		Q_FOREACH (AbstractBookmarksService *as, BookmarksServices_)
		{
			Ui_.Services_->addItem (as->GetIcon (), as->GetName (),
					QVariant::fromValue<QObject*> (as));

			QStandardItem *item = new QStandardItem (as->GetIcon (), as->GetName ());
			item->setCheckable (true);
			model->appendRow (item);

			connect (as,
					SIGNAL (gotValidReply (bool)),
					this,
					SLOT (checkServiceAnswer (bool)));
		}
	}

	void Settings::ReadSettings ()
	{
		QStringList  activeServicesNames = XmlSettingsManager::Instance ()->
				Property ("Sync/ActiveServices", QString ("")).toStringList ();
		
		QList<AbstractBookmarksService*> activeServices;
		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Core::Instance ().GetServiceModel ());
		if (!model)
		{
			qWarning() << Q_FUNC_INFO
					<< "model is not QAbstractItemModel"
					<< Core::Instance ().GetServiceModel ();
			return;
		}
		
		if (activeServicesNames.size ())
			for (int i = 0, rows = model->rowCount (); i < rows; i++)
				if (activeServicesNames.contains (model->item (i)->text ()))
				{
					model->item (i)->setCheckState (Qt::Checked);
					activeServices << BookmarksServices_.at (i);
				}

		if (!activeServices.isEmpty ())
			Core::Instance ().SetActiveBookmarksServices (activeServices);
	}

	void Settings::SetApplyEnabled (const QString& firestString, const QString& secondString)
	{
		Ui_.Apply_->setEnabled (!(firestString.isEmpty () ||
				secondString.isEmpty () ||
				!Ui_.Edit_->isChecked () &&
				!Model_->findItems (Ui_.Login_->text (),
						Qt::MatchFixedString | Qt::MatchRecursive).isEmpty ()));
	}

	void Settings::SetConfirmSend (bool confirm)
	{
	}


	QString Settings::GetSelectedName () const
	{
		return Ui_.Services_->currentText ();
	}

	void Settings::accept ()
	{
		QList<AbstractBookmarksService*> activeServices;
		QStringList activeServicesNames;
		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Core::Instance ().GetServiceModel ());
		if (!model)
		{
			qWarning() << Q_FUNC_INFO
					<< "model is not QAbstractItemModel"
					<< Core::Instance ().GetServiceModel ();
			return;
		}
		int rows = model->rowCount ();
		for (int i = 0; i < rows; i++)
			if (model->item (i)->checkState () == Qt::Checked)
			{
				activeServices << BookmarksServices_.at (i);
				activeServicesNames << model->item (i)->text ();
			}

		XmlSettingsManager::Instance ()->
				setProperty ("Sync/ActiveServices", activeServicesNames);

		if (!activeServices.isEmpty ())
			Core::Instance ().SetActiveBookmarksServices (activeServices);
	}

	void Settings::on_Add__toggled (bool checked)
	{
		if (checked)
		{
			if (Ui_.Edit_->isChecked ())
				Ui_.Edit_->toggle ();
			Ui_.ControlLayout_->insertWidget (1, Ui_.LoginFrame_);
			Ui_.LoginFrame_->show ();
		}
		else
		{
			Ui_.ControlLayout_->removeWidget (Ui_.LoginFrame_);
			Ui_.LoginFrame_->hide ();
			ClearFrameState ();
		}
	}

	void Settings::on_Edit__toggled (bool checked)
	{
		if (checked)
		{
			if (Ui_.Add_->isChecked ())
				Ui_.Add_->toggle ();
			Ui_.ControlLayout_->insertWidget (2, Ui_.LoginFrame_);
			Ui_.LoginFrame_->show ();
			Ui_.Login_->setText (Ui_.AccountsView_->currentIndex ().data ().toString ());
			Ui_.Password_->setText (Core::Instance ().GetPassword (Ui_.Login_->text (),
					Ui_.AccountsView_->currentIndex ().parent ().data ().toString ()));
		}
		else
		{
			Ui_.ControlLayout_->removeWidget (Ui_.LoginFrame_);
			Ui_.LoginFrame_->hide ();
			ClearFrameState ();
		}
	}

	void Settings::on_Delete__released ()
	{
		if (Ui_.AccountsView_->currentIndex ().parent () == QModelIndex ())
			return;
		else
		{
			QList<QVariant> keys;
			keys << "org.LeechCraft.Poshuku.OnlineBookmarks." +
					Ui_.AccountsView_->currentIndex ().parent ().data ().toString () +
					"/" + Ui_.AccountsView_->currentIndex ().data ().toString ();

			Entity e = Util::MakeEntity (keys,
					QString (),
					Internal,
					"x-leechcraft/data-persistent-clear");

			Ui_.LoginFrame_->hide ();
			Model_->removeRow (Ui_.AccountsView_->currentIndex ().row (),
					Ui_.AccountsView_->currentIndex ().parent ());

			if (Ui_.Add_->isChecked ())
				Ui_.Add_->toggle ();
			else if (Ui_.Edit_->isChecked ())
				Ui_.Edit_->toggle ();

			Core::Instance ().SendEntity (e);
		}
	}

	void Settings::handleStuff ()
	{
		QStringList names;
		Q_FOREACH (AbstractBookmarksService *item, BookmarksServices_)
			names << item->GetName ();

		int indexService = -1;
		if (Ui_.Add_->isChecked ())
			indexService = names.indexOf (GetSelectedName ());
		else if (Ui_.Edit_->isChecked ())
			indexService = names.indexOf (Ui_.AccountsView_->currentIndex ().parent ().
					data ().toString ());

		BookmarksServices_.at (indexService)->
				CheckValidAccountData (Ui_.Login_->text (), Ui_.Password_->text ());
	}

	void Settings::handleLoginTextChanged (const QString& text)
	{
		SetApplyEnabled (text, Ui_.Password_->text ());
	}

	void Settings::handlePasswordTextChanged (const QString& text)
	{
		SetApplyEnabled (text, Ui_.Login_->text ());
	}

	void Settings::on_AccountsView__clicked (const QModelIndex& index)
	{
		if (index.parent() == QModelIndex ())
		{
			if (Ui_.Edit_->isChecked ())
				Ui_.Edit_->toggle ();

			Ui_.Edit_->setEnabled (false);
			Ui_.Delete_->setEnabled (false);
		}
		else
		{
			Ui_.Edit_->setEnabled (true);
			Ui_.Delete_->setEnabled (true);
			Ui_.Login_->setText (Ui_.AccountsView_->currentIndex ().data ().toString ());
		}
	}

	void Settings::checkServiceAnswer (bool valid)
	{
		if (!valid)
		{
			Entity e = Util::MakeNotification ("Poshuku",
					tr ("Invalid account data"),
					PCritical_);
			Core::Instance ().SendEntity (e);
		
			return;
		}
		
		QString service = "Account/" + Ui_.Services_->currentText ().toUtf8 ().toBase64 ();
		if (XmlSettingsManager::Instance ()->property (service.toLatin1 ()).isNull ())
			XmlSettingsManager::Instance ()->
					setProperty (service.toLatin1 (), Ui_.Login_->text ());
		else
		{
			QStringList loginList = XmlSettingsManager::Instance ()->
					property (service.toLatin1 ()).toStringList();
			loginList << Ui_.Login_->text ();
			XmlSettingsManager::Instance ()->
					setProperty (service.toLatin1 (), loginList);
		}

		QList<QStandardItem*> items = Model_->findItems (Ui_.Services_->currentText ());
		QStandardItem *serviceItem;
		if (!items.size ())
		{
			serviceItem = new QStandardItem (Ui_.Services_->currentText ());
			Model_->appendRow (serviceItem);
		}
		else
			serviceItem = items.at (0);

		serviceItem->appendRow (new QStandardItem (Ui_.Login_->text ()));

		Core::Instance ().SetPassword (Ui_.Password_->text (), Ui_.Login_->text (), GetSelectedName ());

		ClearFrameState ();

		if (Ui_.Add_->isChecked ())
			Ui_.Add_->toggle ();
		else if (Ui_.Edit_->isChecked ())
			Ui_.Edit_->toggle ();
	
		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Core::Instance ().GetServiceModel ());
		if (!model)
		{
			qWarning() << Q_FUNC_INFO
					<< "model is not QAbstractItemModel"
					<< Core::Instance ().GetServiceModel ();
			return;
		}
		
		for (int i = 0, rows = model->rowCount (); i < rows; i++)
			if (model->item (i)->text () == Ui_.Services_->currentText ())
				model->item (i)->setCheckState (Qt::Checked);
	}
}
}
}
}
}
