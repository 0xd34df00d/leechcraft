/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editlistsdialog.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include "scriptsmanager.h"
#include "urllistscript.h"

namespace LC
{
namespace XProxy
{
	EditListsDialog::EditListsDialog (const QList<UrlListScript*>& scripts,
			const ScriptsManager *manager, QWidget *parent)
	: QDialog { parent }
	, Model_ { new QStandardItemModel { this } }
	, Manager_ { manager }
	, Scripts_ { scripts }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Script name") });

		Ui_.setupUi (this);
		Ui_.ListsView_->setModel (Model_);

		for (const auto script : scripts)
		{
			const auto item = new QStandardItem { script->GetListName () };
			item->setEditable (false);
			Model_->appendRow (item);
		}
	}

	const QList<UrlListScript*>& EditListsDialog::GetScripts () const
	{
		return Scripts_;
	}

	void EditListsDialog::on_AddButton__released ()
	{
		auto unaddedScripts = Manager_->GetScripts ();
		for (auto script : Scripts_)
			unaddedScripts.removeOne (script);

		if (unaddedScripts.isEmpty ())
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("No new scripts can be added."));
			return;
		}

		QStringList names;
		for (const auto script : unaddedScripts)
			names << script->GetListName ();

		const auto& item = QInputDialog::getItem (this,
				tr ("Add a script"),
				tr ("Select a script or dynamic list to add:"),
				names,
				0,
				false);
		const auto idx = names.indexOf (item);
		if (idx < 0)
			return;

		const auto script = unaddedScripts.value (idx);
		Scripts_ << script;
		const auto modelItem = new QStandardItem { script->GetListName () };
		modelItem->setEditable (false);
		Model_->appendRow (modelItem);
	}

	void EditListsDialog::on_RemoveButton__released ()
	{
		const auto& row = Ui_.ListsView_->currentIndex ().row ();
		if (row < 0 || row >= Scripts_.size ())
			return;

		Scripts_.removeAt (row);
		Model_->removeRow (row);
	}
}
}
