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
#include "xmlsettingsmanager.h"
#include "addcontactdialog.h"

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

		connect (Core::Instance ().GetCLModel (),
				SIGNAL (rowsInserted (const QModelIndex&, int, int)),
				this,
				SLOT (handleRowsInserted (const QModelIndex&, int, int)));
		connect (Core::Instance ().GetCLModel (),
				SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
				this,
				SLOT (rebuildTreeExpansions ()));
		connect (Core::Instance ().GetCLModel (),
				SIGNAL (modelReset ()),
				this,
				SLOT (rebuildTreeExpansions ()));

		Ui_.CLTree_->expandAll ();

		if (Core::Instance ().GetCLModel ()->rowCount ())
			handleRowsInserted (QModelIndex (),
					0, Core::Instance ().GetCLModel ()->rowCount () - 1);

		QVBoxLayout *lay = qobject_cast<QVBoxLayout*> (layout ());
		lay->insertWidget (0, UpperBar_);

		MenuGeneral_->addAction (tr ("Accounts..."),
				this,
				SLOT (showAccountsList ()));
		MenuGeneral_->addSeparator ();
		MenuGeneral_->addAction (tr ("Add contact..."),
				this,
				SLOT (handleAddContactRequested ()));

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
		const QModelIndex& index = Ui_.CLTree_->indexAt (pos);
		if (!index.isValid ())
			return;

		QList<QAction*> actions;
		switch (index.data (Core::CLREntryType).value<Core::CLEntryType> ())
		{
		case Core::CLETContact:
		{
			QObject *obj = index.data (Core::CLREntryObject).value<QObject*> ();
			Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (obj);
			const QList<QAction*>& allActions = Core::Instance ().GetEntryActions (entry);
			Q_FOREACH (QAction *action, allActions)
				if (Core::Instance ().GetAreasForAction (action)
						.contains (Core::CLEAAContactListCtxtMenu) ||
					action->isSeparator ())
					actions << action;
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

		acc->ChangeState (Plugins::EntryStatus (ssd->GetState (), ssd->GetStatusText ()));
	}

	void MainWidget::showAccountsList ()
	{
		AccountsListDialog *dia = new AccountsListDialog (this);
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		dia->exec ();
	}

	void MainWidget::handleAddContactRequested ()
	{
		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (this));
		if (dia->exec () != QDialog::Accepted)
			return;

		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}

	namespace
	{
		QString BuildPath (const QModelIndex& index)
		{
			QString path = index.data ().toString ();
			QModelIndex parent = index;
			while ((parent = parent.parent ()).isValid ())
				path.prepend (parent.data ().toString () + "/");
			path = path.toUtf8 ().toBase64 ();
			path.prepend ("CLTreeState/Expanded/");
			return path;
		}
	}

	void MainWidget::handleRowsInserted (const QModelIndex& parent, int begin, int end)
	{
		for (int i = begin; i <= end; ++i)
		{
			const QModelIndex& index = Core::Instance ().GetCLModel ()->index (i, 0, parent);
			if (index.data (Core::CLREntryType).value<Core::CLEntryType> () == Core::CLETContact)
				continue;

			QString path = BuildPath (index);

			bool expanded = XmlSettingsManager::Instance ().Property (path, true).toBool ();
			if (expanded)
				Ui_.CLTree_->setExpanded (index, true);

			if (index.model ()->rowCount (index))
				handleRowsInserted (index, 0, index.model ()->rowCount (index) - 1);
		}
	}

	void MainWidget::rebuildTreeExpansions ()
	{
		if (Core::Instance ().GetCLModel ()->rowCount ())
			handleRowsInserted (QModelIndex (),
					0, Core::Instance ().GetCLModel ()->rowCount () - 1);
	}

	namespace
	{
		void SetExpanded (const QModelIndex& idx, bool expanded)
		{
			XmlSettingsManager::Instance ().setProperty (BuildPath (idx).toUtf8 ().constData (), true);
		}
	}

	void MainWidget::on_CLTree__collapsed (const QModelIndex& idx)
	{
		SetExpanded (idx, false);
	}

	void MainWidget::on_CLTree__expanded (const QModelIndex& idx)
	{
		SetExpanded (idx, true);
	}
}
}
}
