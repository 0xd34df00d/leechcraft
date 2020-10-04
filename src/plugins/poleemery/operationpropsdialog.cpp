/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "operationpropsdialog.h"
#include <limits>
#include <algorithm>
#include <numeric>
#include <QStringListModel>
#include <QMessageBox>
#include <QtDebug>
#include <util/tags/tagscompleter.h>
#include <util/models/modeliterator.h>
#include <interfaces/core/itagsmanager.h>
#include "structures.h"
#include "core.h"
#include "accountsmanager.h"
#include "operationsmanager.h"
#include "currenciesmanager.h"
#include "entriesmodel.h"
#include "entriesdelegate.h"

namespace LC
{
namespace Poleemery
{
	OperationPropsDialog::OperationPropsDialog (QWidget *parent)
	: QDialog (parent)
	, Accounts_ (Core::Instance ().GetAccsManager ()->GetAccounts ())
	, ItemsModel_ (new EntriesModel (this))
	{
		ItemsModel_->SetModifiesStorage (false);
		Ui_.setupUi (this);
		Ui_.ItemsView_->setModel (ItemsModel_);
		for (auto col : { EntriesModel::Shop, EntriesModel::Account, EntriesModel::AccBalance, EntriesModel::SumBalance })
			Ui_.ItemsView_->setColumnHidden (col, true);
		Ui_.ItemsView_->setItemDelegate (new EntriesDelegate);

		Ui_.DateEdit_->setDateTime (QDateTime::currentDateTime ());

		for (const auto& acc : Accounts_)
			Ui_.AccsBox_->addItem (acc.Name_);

		auto completer = new Util::TagsCompleter (Ui_.Categories_);
		const auto& cats = Core::Instance ().GetOpsManager ()->GetKnownCategories ().values ();
		completer->OverrideModel (new QStringListModel (cats, completer));
		Ui_.Categories_->AddSelector ();

		const auto& entries = Core::Instance ().GetOpsManager ()->GetAllEntries ();
		for (const auto& entry : entries)
			switch (entry->GetType ())
			{
			case EntryType::Receipt:
				ReceiptNames_ << entry->Name_;
				break;
			case EntryType::Expense:
				ExpenseNames_ << entry->Name_;
				ShopNames_ << std::dynamic_pointer_cast<ExpenseEntry> (entry)->Shop_;
				break;
			}

		Ui_.AmountCurrency_->addItems (Core::Instance ()
				.GetCurrenciesManager ()->GetEnabledCurrencies ());

		ReceiptNames_.removeDuplicates ();
		std::sort (ReceiptNames_.begin (), ReceiptNames_.end ());
		ExpenseNames_.removeDuplicates ();
		std::sort (ExpenseNames_.begin (), ExpenseNames_.end ());
		ShopNames_.removeDuplicates ();
		std::sort (ShopNames_.begin (), ShopNames_.end ());

		Ui_.Shop_->addItems (QStringList (QString ()) + ShopNames_);

		on_ExpenseEntry__released ();

		if (!Accounts_.isEmpty ())
			on_AccsBox__currentIndexChanged (0);

		connect (Ui_.AmountBilled_,
				SIGNAL (valueChanged (double)),
				this,
				SLOT (recalcNatives ()));
	}

	EntryType OperationPropsDialog::GetEntryType () const
	{
		return Ui_.ExpenseEntry_->isChecked () ?
				EntryType::Expense :
				EntryType::Receipt;
	}

	QList<EntryBase_ptr> OperationPropsDialog::GetEntries () const
	{
		const auto& acc = Accounts_.at (Ui_.AccsBox_->currentIndex ());
		const auto accId = acc.ID_;

		auto curMgr = Core::Instance ().GetCurrenciesManager ();

		const auto currency = Ui_.AmountCurrency_->currentText ();

		if (GetEntryType () == EntryType::Receipt)
		{
			const auto& name = Ui_.Name_->currentText ();
			const auto& descr = Ui_.Description_->text ();
			const auto amount = curMgr->Convert (currency, acc.Currency_, Ui_.Amount_->value ());

			return { std::make_shared<ReceiptEntry> (accId, amount, name, descr, GetDateTime ()) };
		}
		else
			return ItemsModel_->GetEntries ();
	}

	QDateTime OperationPropsDialog::GetDateTime () const
	{
		auto dt = Ui_.DateEdit_->dateTime ();
		auto time = dt.time ();
		dt.setTime ({ time.hour (), time.minute () });
		return dt;
	}

	void OperationPropsDialog::on_AccsBox__currentIndexChanged (int index)
	{
		const auto& accCur = Accounts_.at (index).Currency_;
		const auto pos = Ui_.AmountCurrency_->findText (accCur);
		if (pos >= 0)
			Ui_.AmountCurrency_->setCurrentIndex (pos);

		const auto accId = Accounts_.at (index).ID_.Val_;
		for (int i = 0; i < ItemsModel_->rowCount (); ++i)
		{
			const auto& idx = ItemsModel_->index (i, EntriesModel::Columns::Account);
			ItemsModel_->setData (idx, accId);
		}

		Ui_.AmountBilled_->setSuffix (" " + accCur);

		recalcNatives ();
	}

	void OperationPropsDialog::on_AmountCurrency__currentIndexChanged (int)
	{
		if (!ItemsModel_->rowCount ())
			return;

		const bool recalc = QMessageBox::question (this,
				tr ("Update prices"),
				tr ("The selected currency has been changed. "
					"Should prices be recalculated using the new currency?"),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;

		const auto& newCurrency = Ui_.AmountCurrency_->currentText ();
		if (!recalc)
		{
			for (int i = 0; i < ItemsModel_->rowCount (); ++i)
			{
				const auto& idx = ItemsModel_->index (i, EntriesModel::Columns::EntryCurrency);
				ItemsModel_->setData (idx, newCurrency);
			}
		}
		else
		{
			const auto& sourceCurrency = ItemsModel_->
					index (0, EntriesModel::Columns::EntryCurrency).data ().toString ();
			const auto rate = Core::Instance ().GetCurrenciesManager ()->
					GetRate (sourceCurrency, newCurrency);

			for (int i = 0; i < ItemsModel_->rowCount (); ++i)
			{
				const auto& curIdx = ItemsModel_->index (i, EntriesModel::Columns::EntryCurrency);
				ItemsModel_->setData (curIdx, newCurrency);

				const auto& curPriceIdx = ItemsModel_->index (i, EntriesModel::Columns::Price);
				ItemsModel_->setData (curPriceIdx,
						curPriceIdx.data (Qt::EditRole).toDouble () * rate);

				const auto& curRateIdx = ItemsModel_->index (i, EntriesModel::Columns::EntryRate);
				ItemsModel_->setData (curRateIdx,
						curRateIdx.data (Qt::EditRole).toDouble () / rate);
			}
		}
	}

	void OperationPropsDialog::on_DateEdit__dateTimeChanged (const QDateTime& datetime)
	{
		for (int i = 0; i < ItemsModel_->rowCount (); ++i)
		{
			const auto& idx = ItemsModel_->index (i, EntriesModel::Columns::Date);
			ItemsModel_->setData (idx, datetime);
		}
	}

	void OperationPropsDialog::on_Shop__editTextChanged (const QString& text)
	{
		for (int i = 0; i < ItemsModel_->rowCount (); ++i)
		{
			const auto& idx = ItemsModel_->index (i, EntriesModel::Columns::Shop);
			ItemsModel_->setData (idx, text);
		}
	}

	void OperationPropsDialog::on_ExpenseEntry__released ()
	{
		Ui_.Name_->clear ();
		Ui_.Name_->addItems (QStringList (QString ()) + ExpenseNames_);
		Ui_.PagesStack_->setCurrentWidget (Ui_.ExpensePage_);
	}

	void OperationPropsDialog::on_ReceiptEntry__released ()
	{
		Ui_.Name_->clear ();
		Ui_.Name_->addItems (QStringList (QString ()) + ReceiptNames_);
		Ui_.PagesStack_->setCurrentWidget (Ui_.ReceiptPage_);
	}

	void OperationPropsDialog::on_AddEntry__released ()
	{
		const auto& acc = Accounts_.at (Ui_.AccsBox_->currentIndex ());

		auto entry = std::make_shared<ExpenseEntry> ();
		entry->AccountID_ = acc.ID_;
		entry->Date_ = GetDateTime ();
		entry->Shop_ = Ui_.Shop_->currentText ();
		entry->EntryCurrency_ = Ui_.AmountCurrency_->currentText ();

		auto curMgr = Core::Instance ().GetCurrenciesManager ();
		entry->Rate_ = curMgr->GetRate (entry->EntryCurrency_, acc.Currency_);

		ItemsModel_->AddEntry (entry);

		recalcNatives ();
	}

	void OperationPropsDialog::on_RemoveEntry__released ()
	{
		const auto& selected = Ui_.ItemsView_->currentIndex ();
		if (!selected.isValid ())
			return;

		ItemsModel_->RemoveEntry (selected);
	}

	void OperationPropsDialog::recalcNatives ()
	{
		const auto val = Ui_.AmountBilled_->value ();

		double rate = 1;
		if (val >= std::numeric_limits<double>::epsilon ())
		{
			const auto sum = std::accumulate (Util::ModelIterator (ItemsModel_, 0, EntriesModel::Columns::Price),
					Util::ModelIterator (ItemsModel_, ItemsModel_->rowCount (), EntriesModel::Columns::Price),
					0.0,
					[] (double sum, const QModelIndex& idx)
						{ return sum + idx.data (Qt::EditRole).toDouble (); });
			rate = val / sum;
		}

		for (int i = 0; i < ItemsModel_->rowCount (); ++i)
		{
			const auto& curRateIdx = ItemsModel_->index (i, EntriesModel::Columns::EntryRate);
			ItemsModel_->setData (curRateIdx, rate);

			const auto& curPriceIdx = ItemsModel_->index (i, EntriesModel::Columns::Price);
			const auto& curNativePriceIdx = ItemsModel_->index (i, EntriesModel::Columns::NativePrice);
			ItemsModel_->setData (curNativePriceIdx,
					curPriceIdx.data (Qt::EditRole).toDouble () * rate);
		}
	}
}
}
