/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "panelsettingsdialog.h"
#include <QStandardItemModel>
#include <QAbstractButton>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LC::SB2
{
	PanelSettingsDialog::PanelSettingsDialog (SettingsList_t settingsList, QWidget *parent)
	: QDialog { parent }
	, ItemsModel_ { new QStandardItemModel { this } }
	, Items_ { std::move (settingsList) }
	{
		Ui_.setupUi (this);

		for (const auto& item : Items_)
		{
			const auto rowItem = new QStandardItem { item.Icon_, item.Name_ };
			rowItem->setEditable (false);
			ItemsModel_->appendRow (rowItem);

			Ui_.SettingsStack_->addWidget (item.XSD_->GetWidget ());
		}

		Ui_.ItemsView_->setModel (ItemsModel_);

		connect (Ui_.ItemsView_->selectionModel (),
				&QItemSelectionModel::currentChanged,
				this,
				[this] (const QModelIndex& index)
				{
					if (index.isValid ())
						Ui_.SettingsStack_->setCurrentIndex (index.row ());
				});
		connect (Ui_.ButtonBox_,
				&QDialogButtonBox::clicked,
				this,
				&PanelSettingsDialog::HandleDialogButtonClicked);
	}

	PanelSettingsDialog::~PanelSettingsDialog()
	{
		for (const auto& item : Items_)
		{
			const auto w = item.XSD_->GetWidget ();
			Ui_.SettingsStack_->removeWidget (w);
			w->setParent (nullptr);
		}
	}

	void PanelSettingsDialog::HandleDialogButtonClicked (QAbstractButton *button)
	{
		switch (Ui_.ButtonBox_->buttonRole (button))
		{
		case QDialogButtonBox::AcceptRole:
		case QDialogButtonBox::ApplyRole:
			for (const auto& item : Items_)
				item.XSD_->accept ();
			break;
		case QDialogButtonBox::RejectRole:
		case QDialogButtonBox::ResetRole:
			for (const auto& item : Items_)
				item.XSD_->reject ();
			break;
		default:
			break;
		}
	}
}
