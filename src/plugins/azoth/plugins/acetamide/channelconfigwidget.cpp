/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelconfigwidget.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "sortfilterproxymodel.h"
#include "channelclentry.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	ChannelConfigWidget::ChannelConfigWidget (ChannelCLEntry *clentry, QWidget *parent)
	: QWidget { parent }
	, ChannelEntry_ { clentry }
	, BanModel_ { new QStandardItemModel (this) }
	, ExceptModel_ { new QStandardItemModel (this) }
	, InviteModel_ { new QStandardItemModel (this) }
	, BanFilterModel_ { new SortFilterProxyModel (this) }
	, ExceptFilterModel_ { new SortFilterProxyModel (this) }
	, InviteFilterModel_ { new SortFilterProxyModel (this) }
	{
		Ui_.setupUi (this);

		auto handleSearch = [] (QLineEdit *edit, QSortFilterProxyModel *model)
		{
			connect (edit,
					&QLineEdit::textChanged,
					[model] (const QString& text)
					{
						model->setFilterRegExp (QRegExp { text, Qt::CaseInsensitive, QRegExp::FixedString });
						model->setFilterKeyColumn (1);
					});
		};
		handleSearch (Ui_.BanSearch_, BanFilterModel_);
		handleSearch (Ui_.ExceptSearch_, ExceptFilterModel_);
		handleSearch (Ui_.InviteSearch_, InviteFilterModel_);

		BanModel_->setColumnCount (3);
		BanModel_->setHorizontalHeaderLabels (QStringList () << tr ("Ban mask")
				<< tr ("Set by")
				<< tr ("Date"));

		ExceptModel_->setColumnCount (3);
		ExceptModel_->setHorizontalHeaderLabels (QStringList () << tr ("Except mask")
				<< tr ("Set by")
				<< tr ("Date"));

		InviteModel_->setColumnCount (3);
		InviteModel_->setHorizontalHeaderLabels (QStringList () << tr ("Invite mask")
				<< tr ("Set by")
				<< tr ("Date"));

		Ui_.BanList_->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
		Ui_.ExceptList_->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
		Ui_.InviteList_->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
		Ui_.BanList_->setModel (BanFilterModel_);
		Ui_.ExceptList_->setModel (ExceptFilterModel_);
		Ui_.InviteList_->setModel (InviteFilterModel_);
		BanFilterModel_->setSourceModel (BanModel_);
		ExceptFilterModel_->setSourceModel (ExceptModel_);
		InviteFilterModel_->setSourceModel (InviteModel_);

		ChannelMode_ = ChannelEntry_->GetChannelModes ();
		handleNewChannelModes (ChannelMode_);

		const auto appendRow = [] (QStandardItemModel *model)
		{
			return [model] (const QString& mask, const QString& nick, const QDateTime& date)
			{
				QList<QStandardItem*> items
				{
					new QStandardItem { mask },
					new QStandardItem { nick },
					new QStandardItem { date.toString ("dd.MM.yyyy hh:mm:ss") }
				};
				for (const auto item : items)
					item->setEditable (false);
				model->appendRow (items);
			};
		};

		connect (ChannelEntry_,
				&ChannelCLEntry::gotBanListItem,
				this,
				appendRow (BanModel_));
		connect (ChannelEntry_,
				&ChannelCLEntry::gotExceptListItem,
				this,
				appendRow (ExceptModel_));
		connect (ChannelEntry_,
				&ChannelCLEntry::gotInviteListItem,
				this,
				appendRow (InviteModel_));

		connect (ChannelEntry_,
				&ChannelCLEntry::gotNewChannelModes,
				this,
				&ChannelConfigWidget::handleNewChannelModes);

		Ui_.tabWidget->setCurrentIndex (0);
	}

	void ChannelConfigWidget::accept ()
	{
		ChannelMode_.BlockOutsideMessageMode_ = Ui_.BlockOutMessage_->isChecked ();
		ChannelMode_.ChannelKey_.first = Ui_.Password_->isChecked ();
		ChannelMode_.ChannelKey_.second = Ui_.Key_->text ();
		ChannelMode_.InviteMode_ = Ui_.InvitesOnly_->isChecked ();
		ChannelMode_.ModerateMode_ = Ui_.ModerateChannel_->isChecked ();
		ChannelMode_.OnlyOpChangeTopicMode_ = Ui_.OpTopic_->isChecked ();
		ChannelMode_.PrivateMode_ = Ui_.PrivateChannel_->isChecked ();
		ChannelMode_.ReOpMode_ = Ui_.ReOp_->isChecked ();
		ChannelMode_.SecretMode_ = Ui_.SecretChannel_->isChecked ();
		ChannelMode_.UserLimit_.first = Ui_.UserLimit_->isChecked ();
		ChannelMode_.UserLimit_.second = Ui_.Limit_->value ();
		ChannelEntry_->SetNewChannelModes (ChannelMode_);
	}

	void ChannelConfigWidget::on_tabWidget_currentChanged (int index)
	{
		switch (index)
		{
		case 1:
			BanModel_->clear ();
			ChannelEntry_->RequestBanList ();
			IsWidgetRequest_ = true;
			break;
		case 2:
			ExceptModel_->clear ();
			ChannelEntry_->RequestExceptList ();
			IsWidgetRequest_ = true;
			break;
		case 3:
			InviteModel_->clear ();
			ChannelEntry_->RequestInviteList ();
			IsWidgetRequest_ = true;
			break;
		default:
			IsWidgetRequest_ = false;
		}
		ChannelEntry_->SetIsWidgetRequest (IsWidgetRequest_);
	}

	void ChannelConfigWidget::on_UpdateBan__clicked ()
	{
		if (Ui_.BanHostMask_->text ().isEmpty ())
			return;
		on_RemoveBan__clicked ();
		on_AddBan__clicked ();
	}

	void ChannelConfigWidget::on_AddBan__clicked ()
	{
		if (Ui_.BanHostMask_->text ().isEmpty ())
			return;
		ChannelEntry_->AddBanListItem (Ui_.BanHostMask_->text ());
	}

	void ChannelConfigWidget::on_RemoveBan__clicked ()
	{
		const QModelIndex currentIndex = Ui_.BanList_->currentIndex ();
		if (!currentIndex.isValid ())
			return;
		ChannelEntry_->RemoveBanListItem (Ui_.BanHostMask_->text ());
	}

	void ChannelConfigWidget::on_UpdateExcept__clicked ()
	{
		if (Ui_.ExceptHostMask_->text ().isEmpty ())
			return;
		on_RemoveExcept__clicked ();
		on_AddExcept__clicked ();
	}

	void ChannelConfigWidget::on_AddExcept__clicked ()
	{
		if (Ui_.ExceptHostMask_->text ().isEmpty ())
			return;
		ChannelEntry_->AddExceptListItem (Ui_.ExceptHostMask_->text ());
	}

	void ChannelConfigWidget::on_RemoveExcept__clicked ()
	{
		const QModelIndex currentIndex = Ui_.ExceptList_->currentIndex ();
		if (!currentIndex.isValid ())
			return;
		ChannelEntry_->RemoveExceptListItem (Ui_.ExceptHostMask_->text ());
	}

	void ChannelConfigWidget::on_UpdateInvite__clicked ()
	{
		if (Ui_.InviteHostMask_->text ().isEmpty ())
			return;
		on_RemoveInvite__clicked ();
		on_AddInvite__clicked ();
	}

	void ChannelConfigWidget::on_AddInvite__clicked ()
	{
		if (Ui_.InviteHostMask_->text ().isEmpty ())
			return;
		ChannelEntry_->AddInviteListItem (Ui_.InviteHostMask_->text ());
	}

	void ChannelConfigWidget::on_RemoveInvite__clicked ()
	{
		const QModelIndex currentIndex = Ui_.InviteList_->currentIndex ();
		if (!currentIndex.isValid ())
			return;
		ChannelEntry_->RemoveInviteListItem (Ui_.InviteHostMask_->text ());
	}

	void ChannelConfigWidget::handleNewChannelModes (const ChannelModes& modes)
	{
		ChannelMode_ = modes;
		Ui_.OpTopic_->setChecked (ChannelMode_.OnlyOpChangeTopicMode_);
		Ui_.BlockOutMessage_->setChecked (ChannelMode_.BlockOutsideMessageMode_);
		Ui_.SecretChannel_->setChecked (ChannelMode_.SecretMode_);
		Ui_.PrivateChannel_->setChecked (ChannelMode_.PrivateMode_);
		Ui_.InvitesOnly_->setChecked (ChannelMode_.InviteMode_);
		Ui_.ModerateChannel_->setChecked (ChannelMode_.ModerateMode_);
		Ui_.ReOp_->setChecked (ChannelMode_.ReOpMode_);
		Ui_.UserLimit_->setChecked (ChannelMode_.UserLimit_.first);
		Ui_.Limit_->setValue (ChannelMode_.UserLimit_.second);
		Ui_.Password_->setChecked (ChannelMode_.ChannelKey_.first);
		Ui_.Key_->setText (ChannelMode_.ChannelKey_.second);
	}
}
}
}
