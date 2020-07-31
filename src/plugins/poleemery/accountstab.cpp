/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountstab.h"
#include <QStandardItemModel>
#include "accountsmanager.h"
#include "accountpropsdialog.h"
#include "core.h"

namespace LC
{
namespace Poleemery
{
	AccountsTab::AccountsTab (const TabClassInfo& tc, QObject *plugin)
	: AccsManager_ (Core::Instance ().GetAccsManager ())
	, TC_ (tc)
	, ParentPlugin_ (plugin)
	, AccsModel_ (new QStandardItemModel (this))
	{
		AccsModel_->setHorizontalHeaderLabels ({ tr ("Account"), tr ("Type"), tr ("Currency") });

		Ui_.setupUi (this);
		Ui_.AccountsView_->setModel (AccsModel_);

		for (const auto& acc : AccsManager_->GetAccounts ())
			AddAccount (acc);
	}

	TabClassInfo AccountsTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* AccountsTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void AccountsTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* AccountsTab::GetToolBar () const
	{
		return 0;
	}

	void AccountsTab::AddAccount (const Account& acc)
	{
		QList<QStandardItem*> row
		{
			new QStandardItem (acc.Name_),
			new QStandardItem (ToHumanReadable (acc.Type_)),
			new QStandardItem (acc.Currency_)
		};
		row.first ()->setData (QVariant::fromValue (acc), Roles::Acc);

		for (auto item : row)
			item->setEditable (false);
		AccsModel_->appendRow (row);
	}

	void AccountsTab::on_Add__released ()
	{
		AccountPropsDialog dia (this);
		if (dia.exec () != QDialog::Accepted)
			return;

		auto acc = dia.GetAccount ();
		AccsManager_->AddAccount (acc);
		AddAccount (acc);
	}

	void AccountsTab::on_Modify__released ()
	{
		const auto& current = Ui_.AccountsView_->currentIndex ();
		if (!current.isValid ())
			return;

		const auto srcAccount = AccsModel_->item (current.row ())->data (Roles::Acc).value<Account> ();

		AccountPropsDialog dia (this);
		dia.SetAccount (srcAccount);
		if (dia.exec () != QDialog::Accepted)
			return;

		auto acc = dia.GetAccount ();
		if (acc == srcAccount)
			return;

		AccsManager_->UpdateAccount (acc);
		AccsModel_->item (current.row (), 0)->setText (acc.Name_);
		AccsModel_->item (current.row (), 1)->setText (ToHumanReadable (acc.Type_));
		AccsModel_->item (current.row (), 2)->setText (acc.Currency_);
	}

	void AccountsTab::on_Remove__released ()
	{
		const auto& current = Ui_.AccountsView_->currentIndex ();
		if (!current.isValid ())
			return;

		const auto& sibling = current.sibling (current.row (), 0);
		AccsManager_->DeleteAccount (sibling.data (Roles::Acc).value<Account> ());
		AccsModel_->removeRow (current.row ());
	}
}
}
