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
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <util/util.h>
#include <util/tags/tagscompleter.h>
#include "core.h"
#include "todomanager.h"
#include "addtododialog.h"
#include "todostorage.h"
#include "todolistdelegate.h"
#include "storagemodel.h"
#include "todosfproxymodel.h"
#include "icalgenerator.h"

namespace LeechCraft
{
namespace Otlozhu
{
	TodoTab::TodoTab (const TabClassInfo& tc, QObject *parent)
	: TC_ (tc)
	, Plugin_ (parent)
	, ProxyModel_ (new TodoSFProxyModel (this))
	, ProgressMenu_ (new QMenu (tr ("Set progress")))
	, DueDateMenu_ (new QMenu (tr ("Set due date")))
	, Bar_ (new QToolBar (tc.VisibleName_))
	{
		Ui_.setupUi (this);

		new Util::TagsCompleter (Ui_.FilterLine_, Ui_.FilterLine_);
		Ui_.FilterLine_->AddSelector ();

		Ui_.TodoTree_->setItemDelegate (new TodoListDelegate (Ui_.TodoTree_));

		ProxyModel_->setDynamicSortFilter (true);
		ProxyModel_->setSourceModel (Core::Instance ().GetTodoManager ()->GetTodoModel ());
		ProxyModel_->setFilterKeyColumn (0);
		ProxyModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				ProxyModel_,
				SLOT (setFilterFixedString (QString)));
		connect (Ui_.FilterLine_,
				SIGNAL (textChanged (QString)),
				ProxyModel_,
				SLOT (disableTagsMode ()));
		connect (Ui_.FilterLine_,
				SIGNAL (tagsChosen ()),
				ProxyModel_,
				SLOT (enableTagsMode ()));
		Ui_.TodoTree_->setModel (ProxyModel_);

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
		Ui_.TodoTree_->addAction (removeTodo);
		Ui_.TodoTree_->addAction (Util::CreateSeparator (Ui_.TodoTree_));

		for (int i = 0; i <= 100; i += 10)
		{
			QAction *action = ProgressMenu_->addAction (QString::number (i) + "%");
			action->setProperty ("Otlozhu/Progress", i);
			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (handleQuickProgress ()));

			const QString& sc = i == 100 ?
					QString ("Ctrl+S,D") :
					QString ("Ctrl+S,%1").arg (i / 10);
			action->setShortcut (sc);
		}
		Ui_.TodoTree_->addAction (ProgressMenu_->menuAction ());

		QAction *editComment = new QAction (tr ("Edit comment..."), this);
		editComment->setProperty ("ActionIcon", "document-edit");
		connect (editComment,
				SIGNAL (triggered ()),
				this,
				SLOT (handleEditCommentRequested ()));
		Ui_.TodoTree_->addAction (editComment);

		DueDateMenu_->setProperty ("ActionIcon", "view-calendar");
		const QList<int> delays = { 0, 1, 3, 6, 12, 24, 48, 168 };
		const QStringList labels = { tr ("Clear"), tr ("Hour"), tr ("3 hours"), tr ("6 hours"),
				tr ("12 hours"), tr ("Day"), tr ("2 days"), tr ("Week") };
		for (int i = 0; i < delays.size (); ++i)
		{
			QAction *delay = new QAction (labels.at (i), this);
			connect (delay,
					SIGNAL (triggered ()),
					this,
					SLOT (handleSetDueDateRequested ()));
			delay->setProperty ("Otlozhu/Delay", delays.at (i));
			DueDateMenu_->addAction (delay);
		}

		QAction *customDueDate = new QAction (tr ("Custom..."), this);
		customDueDate->setProperty ("ActionIcon", "view-calendar");
		connect (customDueDate,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSetCustomDueDateRequested ()));
		DueDateMenu_->addSeparator ();
		DueDateMenu_->addAction (customDueDate);
		Ui_.TodoTree_->addAction (DueDateMenu_->menuAction ());

		Bar_->addSeparator ();

		QAction *exportTodos = new QAction (tr ("Export"), this);
		exportTodos->setProperty ("ActionIcon", "document-export");
		connect (exportTodos,
				SIGNAL (triggered ()),
				this,
				SLOT (handleExport ()));
		Bar_->addAction (exportTodos);
	}

	TodoTab::~TodoTab ()
	{
		delete ProgressMenu_;
		delete DueDateMenu_;
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
		if (!index.isValid ())
			return;

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

	void TodoTab::handleEditCommentRequested ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& title = ProxyModel_->data (index, StorageModel::Roles::ItemTitle).toString ();
		auto comment = ProxyModel_->data (index, StorageModel::Roles::ItemComment).toString ();
		comment = QInputDialog::getText (this,
				"Otlozhu",
				tr ("Enter new comment for item %1:")
					.arg (title),
				QLineEdit::Normal,
				comment);
		ProxyModel_->setData (index, comment, StorageModel::Roles::ItemComment);
	}

	void TodoTab::handleSetDueDateRequested ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const int delay = sender ()->property ("Otlozhu/Delay").toInt ();
		QDateTime dt;
		if (delay)
			dt = QDateTime::currentDateTime ().addSecs (delay * 60 * 60);

		ProxyModel_->setData (index, dt, StorageModel::Roles::ItemDueDate);
	}

	void TodoTab::handleSetCustomDueDateRequested ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		QDateTime dt = index.data (StorageModel::Roles::ItemDueDate).toDateTime ();

		QDialog dia (this);
		dia.setWindowTitle (tr ("Select due date"));
		dia.setLayout (new QVBoxLayout);
		QCalendarWidget *w = new QCalendarWidget;
		QDialogButtonBox *box = new QDialogButtonBox;
		dia.layout ()->addWidget (w);
		dia.layout ()->addWidget (box);
		connect (box,
				SIGNAL (accepted ()),
				&dia,
				SLOT (accept ()));
		connect (box,
				SIGNAL (rejected ()),
				&dia,
				SLOT (reject ()));
		if (dia.exec () != QDialog::Accepted)
			return;

		dt.setDate (w->selectedDate ());
		if (QDateTime::currentDateTime ().daysTo (dt) > 1)
			dt.setTime (QTime ());

		ProxyModel_->setData (index, dt, StorageModel::Roles::ItemDueDate);
	}

	void TodoTab::handleQuickProgress ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const int perc = sender ()->property ("Otlozhu/Progress").toInt ();
		ProxyModel_->setData (index, perc, StorageModel::Roles::ItemProgress);
	}

	void TodoTab::handleExport ()
	{
		const QString& filename = QFileDialog::getSaveFileName (this,
				tr ("Todos export"),
				QDir::homePath (),
				tr ("iCalendar files (*.ics)"));

		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< filename
					<< file.errorString ();
			emit gotEntity (Util::MakeNotification ("Otlozhu",
						tr ("Unable to export to %1: %2.")
							.arg (filename)
							.arg (file.errorString ()),
						PCritical_));
			return;
		}

		ICalGenerator gen;
		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		for (int i = 0; i < storage->GetNumItems (); ++i)
			gen << storage->GetItemAt (i);

		file.write (gen ());

		emit gotEntity (Util::MakeNotification ("Otlozhu",
					tr ("Todo items were successfully exported to %1.")
						.arg (QFileInfo (filename).fileName ()),
					PInfo_));
	}
}
}
