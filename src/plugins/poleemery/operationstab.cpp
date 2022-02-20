/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "operationstab.h"
#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QStringListModel>
#include <QToolBar>
#include "core.h"
#include "operationsmanager.h"
#include "operationpropsdialog.h"
#include "entriesmodel.h"
#include "accountsmanager.h"
#include "entriesdelegate.h"

namespace LC
{
namespace Poleemery
{
	OperationsTab::OperationsTab (const TabClassInfo& tc, QObject *plugin)
	: OpsManager_ (Core::Instance ().GetOpsManager ())
	, TC_ (tc)
	, ParentPlugin_ (plugin)
	, Toolbar_ (new QToolBar (tr ("Poleemery")))
	{
		Ui_.setupUi (this);
		Ui_.OpsView_->setItemDelegate (new EntriesDelegate);
		Ui_.OpsView_->setModel (OpsManager_->GetModel ());

		const auto& fm = fontMetrics ();
		auto setColWidth = [&fm, this] (EntriesModel::Columns col, const QString& str)
			{ Ui_.OpsView_->setColumnWidth (col, fm.horizontalAdvance (str) * 1.1); };

		setColWidth (EntriesModel::Columns::Date,
				QDateTime::currentDateTime ().toString ());
		setColWidth (EntriesModel::Columns::Account, "some account name");
		setColWidth (EntriesModel::Columns::Name, "some typical very long product name");
		setColWidth (EntriesModel::Columns::Price, " 9999.00 USD ");
		setColWidth (EntriesModel::Columns::Count, " 0.999 ");
		setColWidth (EntriesModel::Columns::Shop, "some typical shop name");
		setColWidth (EntriesModel::Columns::AccBalance, " 99999.00 USD ");
		setColWidth (EntriesModel::Columns::SumBalance, " 99999.00 USD ");

		auto addAction = Toolbar_->addAction (tr ("Add..."),
				this, SLOT (add ()));
		addAction->setShortcut (Qt::Key_Insert);
		addAction->setProperty ("ActionIcon", "list-add");

		auto removeAction = Toolbar_->addAction (tr ("Remove"),
				this, SLOT (remove ()));
		removeAction->setShortcut (Qt::Key_Delete);
		removeAction->setProperty ("ActionIcon", "list-remove");
	}

	TabClassInfo OperationsTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* OperationsTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void OperationsTab::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* OperationsTab::GetToolBar () const
	{
		return Toolbar_;
	}

	void OperationsTab::add ()
	{
		OperationPropsDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		for (const auto& entry : dia.GetEntries ())
			OpsManager_->AddEntry (entry);
	}

	void OperationsTab::remove ()
	{
		const auto& item = Ui_.OpsView_->currentIndex ();
		if (!item.isValid ())
			return;

		const auto& name = item.sibling (item.row (), EntriesModel::Columns::Name)
				.data ().toString ();
		if (QMessageBox::question (this,
					"Poleemery",
					tr ("Are you sure you want to delete entry %1?")
						.arg ("<em>" + name + "</em>"),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		OpsManager_->RemoveEntry (item);
	}
}
}
