/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountpropsdialog.h"
#include <QtDebug>
#include "structures.h"
#include "core.h"
#include "currenciesmanager.h"

namespace LC
{
namespace Poleemery
{
	AccountPropsDialog::AccountPropsDialog (QWidget *parent)
	: QDialog (parent)
	, CurrentAccID_ (-1)
	{
		Ui_.setupUi (this);

		Ui_.AccType_->addItem (ToHumanReadable (AccType::Cash));
		Ui_.AccType_->addItem (ToHumanReadable (AccType::BankAccount));

		const auto& currencies = Core::Instance ()
				.GetCurrenciesManager ()->GetEnabledCurrencies ();
		Ui_.Currency_->addItems (currencies);
	}

	void AccountPropsDialog::SetAccount (const Account& account)
	{
		CurrentAccID_ = account.ID_;
		Ui_.AccType_->setCurrentIndex (static_cast<int> (account.Type_));
		Ui_.AccName_->setText (account.Name_);

		const auto pos = Ui_.Currency_->findText (account.Currency_);
		if (pos >= 0)
			Ui_.Currency_->setCurrentIndex (pos);
	}

	Account AccountPropsDialog::GetAccount () const
	{
		return
		{
			CurrentAccID_,
			static_cast<AccType> (Ui_.AccType_->currentIndex ()),
			Ui_.AccName_->text (),
			Ui_.Currency_->currentText ()
		};
	}
}
}
