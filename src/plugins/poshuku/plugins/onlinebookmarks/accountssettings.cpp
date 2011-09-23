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
#include "core.h"

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

		Ui_.Edit_->setEnabled (false);
		Ui_.Delete_->setEnabled (false);

		Ui_.LoginFrame_->hide ();
		Ui_.Register_->hide ();
		
		Q_FOREACH (QObject *plugin, Core::Instance ().GetPlugins ())
		{
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);
			if (!ibs)
			{
				qWarning () << Q_FUNC_INFO
						<< plugin
						<< "doesn't implement IBookmarksService";
				continue;
			}

			Ui_.Services_->addItem (ibs->GetServiceIcon (), ibs->GetServiceName ());
			Ui_.Services_->setItemData (Ui_.Services_->count () - 1,
					QVariant::fromValue<QObject*> (plugin), RServiceObject);

			if (!Service2AuthWidget_.contains (ibs))
			{
				QWidget *widget = ibs->GetAuthWidget ();
				if (!qobject_cast<IAuthWidget*> (widget))
				{
					qWarning () << Q_FUNC_INFO
							<< "auth widget for service"
							<< ibs->GetServiceName ()
							<< "is not a IAuthWidget"
							<< widget;
					return;
				}

				Service2AuthWidget_ [ibs] = widget;
			}
		}
	}

	AccountsSettings::~AccountsSettings ()
	{
		qDeleteAll (Service2AuthWidget_);
	}

	void AccountsSettings::accept ()
	{
	}

	void AccountsSettings::on_Add__toggled (bool checked)
	{
		QObject *plugin = Ui_.Services_->itemData (Ui_.Services_->currentIndex (),
				RServiceObject).value<QObject*> ();
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);
		if (!ibs)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "doesn't implement IBookmarksService";
			return;
		}

		if (!Service2AuthWidget_.contains (ibs))
			return;

		if (checked)
		{
			if (Ui_.Edit_->isChecked ())
				Ui_.Edit_->toggle ();

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

	void AccountsSettings::on_Edit__toggled (bool checked)
	{
		Ui_.Register_->hide ();

		if (checked)
		{
			if (Ui_.Add_->isChecked ())
				Ui_.Add_->toggle ();
			Ui_.ControlLayout_->insertWidget (2, Ui_.LoginFrame_);
			Ui_.LoginFrame_->show ();
		}
		else
		{
			Ui_.ControlLayout_->removeWidget (Ui_.LoginFrame_);
			Ui_.LoginFrame_->hide ();
		}
	}

	void AccountsSettings::on_Delete__clicked ()
	{
		if (Ui_.AccountsView_->currentIndex ().parent () == QModelIndex ())
			return;

		AccountsModel_->removeRow (Ui_.AccountsView_->currentIndex ().row (),
				Ui_.AccountsView_->currentIndex ().parent ());

		if (Ui_.Add_->isChecked ())
			Ui_.Add_->toggle ();
		else if (Ui_.Edit_->isChecked ())
			Ui_.Edit_->toggle ();
	}

	void AccountsSettings::on_AccountsView__clicked (const QModelIndex& index)
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
		}
	}

}
}
}
