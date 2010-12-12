/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "mainwidget.h"
#include <QToolBar>
#include <QMenu>
#include <QVBoxLayout>
#include "interfaces/iclentry.h"
#include "interfaces/iaccount.h"
#include "core.h"
#include "sortfilterproxymodel.h"
#include "accountslistdialog.h"
#include "setstatusdialog.h"
#include "contactlistdelegate.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
	MainWidget::MainWidget (QWidget *parent)
	: QWidget (parent)
	, UpperBar_ (new QToolBar)
	, MenuGeneral_ (new QMenu (tr ("General")))
	, ProxyModel_ (new SortFilterProxyModel ())
	{
		Ui_.setupUi (this);
		Ui_.CLTree_->setItemDelegate (new ContactListDelegate (this));
		ProxyModel_->setSourceModel (Core::Instance ().GetCLModel ());
		Ui_.CLTree_->setModel (ProxyModel_);

		QVBoxLayout *lay = qobject_cast<QVBoxLayout*> (layout ());
		lay->insertWidget (0, UpperBar_);

		QAction *accountsList = new QAction (tr ("Accounts..."),
				this);
		connect (accountsList,
				SIGNAL (triggered ()),
				this,
				SLOT (showAccountsList ()));
		MenuGeneral_->addAction (accountsList);

		UpperBar_->addAction (MenuGeneral_->menuAction ());

		ActionChangeStatus_ = new QAction (tr ("Change status..."), this);
		connect (ActionChangeStatus_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleChangeStatusRequested ()));
	}

	void MainWidget::AddMUCJoiners (const QList<QAction*>& actions)
	{
		MenuGeneral_->addActions (actions);
	}

	void MainWidget::on_CLTree__activated (const QModelIndex& index)
	{
		if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return;

		Core::Instance ().OpenChat (ProxyModel_->mapToSource (index));
	}

	void MainWidget::on_CLTree__customContextMenuRequested (const QPoint& pos)
	{
		QModelIndex index = Ui_.CLTree_->indexAt (pos);
		if (!index.isValid ())
			return;

		QList<QAction*> actions;
		switch (index.data (Core::CLREntryType).value<Core::CLEntryType> ())
		{
		case Core::CLETContact:
		{
			QObject *obj = index.data (Core::CLREntryObject).value<QObject*> ();
			Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (obj);
			actions << entry->GetActions ();
			break;
		}
		case Core::CLETAccount:
		{
			QVariant objVar = index.data (Core::CLRAccountObject);
			ActionChangeStatus_->setData (objVar);
			actions << ActionChangeStatus_;
			break;
		}
		default:
			break;
		}
		if (!actions.size ())
			return;

		QMenu *menu = new QMenu (tr ("Entry context menu"));
		menu->addActions (actions);
		menu->exec (Ui_.CLTree_->mapToGlobal (pos));
	}

	void MainWidget::handleChangeStatusRequested ()
	{
		QObject *obj = ActionChangeStatus_->data ().value<QObject*> ();
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "no object is set";
			return;
		}

		Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (obj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IAccount";
			return;
		}

		SetStatusDialog *ssd = new SetStatusDialog (this);
		if (ssd->exec () != QDialog::Accepted)
			return;

		acc->ChangeState (ssd->GetState (), ssd->GetStatusText ());
	}

	void MainWidget::showAccountsList ()
	{
		AccountsListDialog *dia = new AccountsListDialog (this);
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		dia->exec ();
	}
}
}
}
