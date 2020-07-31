/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "acceptedrejecteddialog.h"
#include <algorithm>
#include <QMessageBox>
#include <QTimer>
#include "exceptionsmodel.h"

namespace LC
{
namespace CertMgr
{
	AcceptedRejectedDialog::AcceptedRejectedDialog (ICoreProxy_ptr proxy)
	: Proxy_ { proxy }
	, CoreSettings_ { QCoreApplication::organizationName (),
			QCoreApplication::applicationName () }
	, Model_ { new ExceptionsModel { CoreSettings_, this } }
	{
		CoreSettings_.beginGroup ("SSL exceptions");

		Model_->setHorizontalHeaderLabels ({ tr ("Address"), tr ("State") });
		Model_->Populate ();

		Ui_.setupUi (this);
		Ui_.View_->setModel (Model_);

		connect (Ui_.View_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (toggleState (QModelIndex)));

		QTimer::singleShot (0,
				this,
				SLOT (adjustWidths ()));

		connect (Ui_.View_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (handleSelectionChanged ()));
		handleSelectionChanged ();
	}

	AcceptedRejectedDialog::~AcceptedRejectedDialog ()
	{
		CoreSettings_.endGroup ();
	}

	void AcceptedRejectedDialog::on_RemoveButton__released ()
	{
		auto selected = Ui_.View_->selectionModel ()->selectedRows ();
		if (selected.isEmpty ())
			return;

		if (QMessageBox::question (this,
					tr ("Remove exceptions"),
					tr ("Are you sure you want to remove %n exception(s)?",
						0, selected.size ()),
					QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
			return;

		std::sort (selected.begin (), selected.end (),
				[] (const QModelIndex& i1, const QModelIndex& i2)
					{ return i1.row () > i2.row (); });

		for (const auto& item : selected)
		{
			const auto& title = item.sibling (item.row (), 0).data ().toString ();

			CoreSettings_.remove (title);

			Model_->removeRow (item.row ());
		}
	}

	void AcceptedRejectedDialog::handleSelectionChanged ()
	{
		const auto& selected = Ui_.View_->selectionModel ()->selectedRows ();
		Ui_.RemoveButton_->setEnabled (!selected.isEmpty ());
	}

	void AcceptedRejectedDialog::toggleState (const QModelIndex& index)
	{
		Model_->ToggleState (index);
	}

	void AcceptedRejectedDialog::adjustWidths ()
	{
		const auto totalWidth = Ui_.View_->viewport ()->width ();
		const auto statusWidth = 150;
		Ui_.View_->setColumnWidth (ExceptionsModel::Name, totalWidth - statusWidth);
		Ui_.View_->setColumnWidth (ExceptionsModel::Status, statusWidth);
	}
}
}
