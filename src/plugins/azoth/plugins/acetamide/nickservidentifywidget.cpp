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
	}

	void NickServIdentifyWidget::accept ()
	{
		emit saveSettings ();
	}

	void NickServIdentifyWidget::on_Add__clicked ()
	{
		NewNickServIdentifyDialog nns { this };
		if (nns.exec () == QDialog::Rejected)
			return;

		emit identifyAdded (nns.GetIdentify ());
	}

	void NickServIdentifyWidget::on_Edit__clicked ()
	{
		const QModelIndex& index = Ui_.NickServIdentifyView_->currentIndex ();
		if (!index.isValid ())
			return;

		const auto& identify = index.data (Util::FlatItemsModelBase::DataRole).value<NickServIdentify> ();

		NewNickServIdentifyDialog nns { this };
		nns.SetIdentify (identify);

		if (nns.exec () == QDialog::Rejected)
			return;

		emit identifyEdited (index.row (), nns.GetIdentify ());
	}

	void NickServIdentifyWidget::on_Delete__clicked ()
	{
		const QModelIndex& index = Ui_.NickServIdentifyView_->currentIndex ();
		if (!index.isValid ())
			return;

		emit identifyRemoved (index.row ());
	}

}
