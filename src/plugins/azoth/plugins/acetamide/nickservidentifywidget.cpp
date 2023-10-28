/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nickservidentifywidget.h"
#include <util/models/flatitemsmodelbase.h>
#include "localtypes.h"
#include "newnickservidentifydialog.h"

namespace LC::Azoth::Acetamide
{
	NickServIdentifyWidget::NickServIdentifyWidget (Util::FlatItemsModelBase& model, QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		Ui_.NickServIdentifyView_->setModel (&model);
		Ui_.NickServIdentifyView_->horizontalHeader ()->setStretchLastSection (true);

		connect (Ui_.Add_,
				&QPushButton::clicked,
				[this]
				{
					auto nns = new NewNickServIdentifyDialog { this };
					nns->show ();
					connect (nns,
							&QDialog::accepted,
							this,
							[=, this] { emit identifyAdded (nns->GetIdentify ()); });
					connect (nns,
							&QDialog::finished,
							this,
							[=, this] { DeleteInvalidatedDialogs_.removeOne (nns); });
					DeleteInvalidatedDialogs_ << nns;
				});
		connect (Ui_.Delete_,
				&QPushButton::clicked,
				[this]
				{
					const auto& index = Ui_.NickServIdentifyView_->currentIndex ();
					if (index.isValid ())
						emit identifyRemoved (index.row ());

					// Important to copy the dialogs list,
					// since rejecting the dialogs might get them removed from this list in the rejection handler.
					auto dias = DeleteInvalidatedDialogs_;
					for (const auto dia : dias)
						delete dia;
					DeleteInvalidatedDialogs_.clear ();
				});
		connect (Ui_.Edit_,
				&QPushButton::clicked,
				[this]
				{
					const auto& index = Ui_.NickServIdentifyView_->currentIndex ();
					if (!index.isValid ())
						return;

					const auto& identify = index.data (Util::FlatItemsModelBase::DataRole).value<NickServIdentify> ();

					auto nns = new NewNickServIdentifyDialog { this };
					nns->SetIdentify (identify);
					nns->show ();
					connect (nns,
							&QDialog::accepted,
							this,
							[=, this, row = index.row ()] { emit identifyEdited (row, nns->GetIdentify ()); });
					connect (nns,
							&QDialog::finished,
							this,
							[=, this] { DeleteInvalidatedDialogs_.removeOne (nns); });
					DeleteInvalidatedDialogs_ << nns;
				});
	}

	void NickServIdentifyWidget::accept ()
	{
		emit saveSettings ();
	}
}
