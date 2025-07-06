/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "todotab.h"
#include <QToolBar>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "todomanager.h"
#include "addtododialog.h"
#include "todostorage.h"
#include "todolistdelegate.h"
#include "storagemodel.h"
#include "todosfproxymodel.h"
#include "icalgenerator.h"
#include "icalparser.h"
#include "itemsmergedialog.h"
#include "editcommentdialog.h"
#include "editdatedialog.h"

namespace LC
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

		new Util::TagsCompleter (Ui_.FilterLine_);
		Ui_.FilterLine_->AddSelector ();

		Ui_.TodoTree_->setItemDelegate (new TodoListDelegate (Ui_.TodoTree_));

		ProxyModel_->setSourceModel (Core::Instance ().GetTodoManager ()->GetTodoModel ());
		ProxyModel_->SetFilterColumns ({ 0 });
		connect (Ui_.FilterLine_,
				&QLineEdit::textChanged,
				ProxyModel_,
				&TodoSFProxyModel::SetFilterString);
		connect (Ui_.FilterLine_,
				&QLineEdit::textChanged,
				ProxyModel_,
				[this] { ProxyModel_->SetTagsMode (false); });
		connect (Ui_.FilterLine_,
				&Util::TagsLineEdit::tagsChosen,
				ProxyModel_,
				[this] { ProxyModel_->SetTagsMode (true); });
		Ui_.TodoTree_->setModel (ProxyModel_);

		QAction *addTodo = new QAction (tr ("Add task..."), this);
		addTodo->setProperty ("ActionIcon", "list-add");
		addTodo->setShortcut (Qt::Key_Insert);
		connect (addTodo,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddTodoRequested ()));
		Bar_->addAction (addTodo);
		Ui_.TodoTree_->addAction (addTodo);

		QAction *addChildTodo = new QAction (tr ("Add child task..."), this);
		connect (addChildTodo,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddChildTodoRequested ()));
		Ui_.TodoTree_->addAction (addChildTodo);

		QAction *removeTodo = new QAction (tr ("Remove task"), this);
		removeTodo->setProperty ("ActionIcon", "list-remove");
		removeTodo->setShortcut (Qt::Key_Delete);
		connect (removeTodo,
				SIGNAL (triggered ()),
				this,
				SLOT (handleRemoveTodoRequested ()));
		Bar_->addAction (removeTodo);
		Ui_.TodoTree_->addAction (removeTodo);

		QAction *cloneTodo = new QAction (tr ("Clone task"), this);
		cloneTodo->setProperty ("ActionIcon", "edit-copy");
		connect (cloneTodo,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCloneTodoRequested ()));
		Ui_.TodoTree_->addAction (cloneTodo);

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

			if (!delays.at (i))
				DueDateMenu_->addSeparator ();
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

		QAction *importTodos = new QAction (tr ("Import"), this);
		importTodos->setProperty ("ActionIcon", "document-import");
		connect (importTodos,
				SIGNAL (triggered ()),
				this,
				SLOT (handleImport ()));
		Bar_->addAction (importTodos);

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
		emit removeTab ();
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

		const auto& item = dia.GetItem ();
		Core::Instance ().GetTodoManager ()->GetTodoStorage ()->AddItem (item);
	}

	void TodoTab::handleAddChildTodoRequested ()
	{
		const auto& curIdx = Ui_.TodoTree_->currentIndex ();
		if (!curIdx.isValid () || curIdx.parent ().isValid ())
			return;

		AddTodoDialog dia;
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& selectedId = curIdx.data (StorageModel::Roles::ItemID).toString ();
		const auto& item = dia.GetItem ();

		const auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		storage->AddItem (item);
		storage->AddDependency (selectedId, item->GetID ());
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

	void TodoTab::handleCloneTodoRequested ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& itemId = index.data (StorageModel::Roles::ItemID).toString ();
		const auto& item = Core::Instance ().GetTodoManager ()->
				GetTodoStorage ()->GetItemByID (itemId);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get item with"
					<< itemId;
			return;
		}

		TodoItem_ptr clone { new TodoItem };
		clone->CopyFrom (item);
		Core::Instance ().GetTodoManager ()->GetTodoStorage ()->AddItem (clone);
	}

	void TodoTab::handleEditCommentRequested ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& title = ProxyModel_->data (index, StorageModel::Roles::ItemTitle).toString ();
		const auto& comment = ProxyModel_->data (index, StorageModel::Roles::ItemComment).toString ();

		EditCommentDialog dia { title, comment, this };
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& newComment = dia.GetComment ();
		if (newComment == comment)
			return;

		ProxyModel_->setData (index, newComment, StorageModel::Roles::ItemComment);
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

		const auto& dt = index.data (StorageModel::Roles::ItemDueDate).toDateTime ();

		EditDateDialog dia { dt, this };
		dia.setWindowTitle (tr ("Select due date"));
		if (dia.exec () != QDialog::Accepted)
			return;

		if (dt == dia.GetDateTime ())
			return;

		ProxyModel_->setData (index, dia.GetDateTime (), StorageModel::Roles::ItemDueDate);
	}

	void TodoTab::handleQuickProgress ()
	{
		const QModelIndex& index = Ui_.TodoTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const int perc = sender ()->property ("Otlozhu/Progress").toInt ();
		ProxyModel_->setData (index, perc, StorageModel::Roles::ItemProgress);
	}

	void TodoTab::handleImport ()
	{
		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Import tasks"),
				QDir::homePath (),
				tr ("iCalendar files (*.ics)"));

		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			QMessageBox::critical (this,
					tr ("Tasks import"),
					tr ("Unable to open %1: %2.")
						.arg ("<em>" + filename + "</em>")
						.arg (file.errorString ()));
			return;
		}

		const auto& items = ICalParser ().Parse (file.readAll ());
		if (items.isEmpty ())
			return;

		ItemsMergeDialog dia (items.size (), this);
		if (dia.exec () != QDialog::Accepted)
			return;

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		auto ourItems = storage->GetAllItems ();
		for (const auto& item : items)
		{
			const auto& itemId = item->GetID ();
			auto pos = std::find_if (ourItems.begin (), ourItems.end (),
					[&itemId] (const auto& ourItem) { return ourItem->GetID () == itemId; });
			if (pos != ourItems.end ())
			{
				if (dia.GetPriority () == ItemsMergeDialog::Priority::Imported)
				{
					(*pos)->CopyFrom (item);
					storage->HandleUpdated (*pos);
				}
				continue;
			}

			const auto& itemTitle = item->GetTitle ();
			pos = std::find_if (ourItems.begin (), ourItems.end (),
					[itemTitle] (const auto& ourItem) { return ourItem->GetTitle () == itemTitle; });
			if (pos != ourItems.end ())
			{
				if (dia.GetPriority () == ItemsMergeDialog::Priority::Imported &&
						dia.GetSameTitle () == ItemsMergeDialog::SameTitle::Merge)
				{
					(*pos)->CopyFrom (item);
					storage->HandleUpdated (*pos);
				}
				continue;
			}

			storage->AddItem (item);
		}
	}

	void TodoTab::handleExport ()
	{
		const QString& filename = QFileDialog::getSaveFileName (this,
				tr ("Todos export"),
				QDir::homePath (),
				tr ("iCalendar files (*.ics)"));

		const auto iem = GetProxyHolder ()->GetEntityManager ();

		QFile file (filename);
		if (!file.open (QIODevice::WriteOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< filename
					<< file.errorString ();
			iem->HandleEntity (Util::MakeNotification ("Otlozhu",
						tr ("Unable to export to %1: %2.")
							.arg (filename)
							.arg (file.errorString ()),
						Priority::Critical));
			return;
		}

		ICalGenerator gen;
		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		for (int i = 0; i < storage->GetNumItems (); ++i)
			gen << storage->GetItemAt (i);

		file.write (gen ());

		iem->HandleEntity (Util::MakeNotification ("Otlozhu",
					tr ("Todo items were successfully exported to %1.")
						.arg (QFileInfo (filename).fileName ()),
					Priority::Info));
	}
}
}
