/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "todotab.h"
#include <QToolBar>
#include <QMessageBox>
#include "core.h"
#include "todomanager.h"
#include "addtododialog.h"
#include "todostorage.h"
#include "todolistdelegate.h"
#include "storagemodel.h"

namespace LeechCraft
{
namespace Otlozhu
{
	TodoTab::TodoTab (const TabClassInfo& tc, QObject *parent)
	: TC_ (tc)
	, Plugin_ (parent)
	, Bar_ (new QToolBar (tc.VisibleName_))
	{
		Ui_.setupUi (this);
		Ui_.TodoTree_->setItemDelegate (new TodoListDelegate (Ui_.TodoTree_));
		Ui_.TodoTree_->setModel (Core::Instance ().GetTodoManager ()->GetTodoModel ());

		QAction *addTodo = new QAction (tr ("Add todo..."), this);
		addTodo->setProperty ("ActionIcon", "list-add");
		addTodo->setShortcut (Qt::Key_Insert);
		connect (addTodo,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddTodoRequested ()));
		Bar_->addAction (addTodo);

		QAction *removeTodo = new QAction (tr ("Remove"), this);
		removeTodo->setProperty ("ActionIcon", "list-remove");
		removeTodo->setShortcut (Qt::Key_Delete);
		connect (removeTodo,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRemoveTodoRequested ()));
		Bar_->addAction (removeTodo);
	}

	TodoTab::~TodoTab ()
	{
		delete Bar_;
	}

	TabClassInfo TodoTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* TodoTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void TodoTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* TodoTab::GetToolBar () const
	{
		return Bar_;
	}

	void TodoTab::handleAddTodoRequested ()
	{
		AddTodoDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		auto item = dia.GetItem ();
		Core::Instance ().GetTodoManager ()->GetTodoStorage ()->AddItem (item);
	}

	void TodoTab::handleRemoveTodoRequested ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		const QString& title = index.data (StorageModel::Roles::ItemTitle).toString ();
		if (QMessageBox::question (this,
					"Otlozhu",
					tr ("Are you sure you want to remove <em>%1</em>?")
						.arg (title),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const QString& id = index.data (StorageModel::Roles::ItemID).toString ();
		Core::Instance ().GetTodoManager ()->GetTodoStorage ()->RemoveItem (id);
	}
}
}
