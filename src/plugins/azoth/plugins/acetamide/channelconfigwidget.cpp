/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "channelconfigwidget.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QtDebug>
#include "channelclentry.h"

namespace LC::Azoth::Acetamide
{

	ChannelConfigWidget::ChannelConfigWidget (ChannelCLEntry *clentry, QWidget *parent)
	: QWidget { parent }
	, ChannelEntry_ { clentry }
	, BanModel_ { new QStandardItemModel (this) }
	, ExceptModel_ { new QStandardItemModel (this) }
	, InviteModel_ { new QStandardItemModel (this) }
	, BanFilterModel_ { new QSortFilterProxyModel (this) }
	, ExceptFilterModel_ { new QSortFilterProxyModel (this) }
	, InviteFilterModel_ { new QSortFilterProxyModel (this) }
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

		connect (Ui_.ConfigTabs_,
				&QTabWidget::currentChanged,
				[this] (int idx)
				{
					if (idx > 0)
						RerequestList (idx - 1);
				});

		SetupListButtons ();

		BanModel_->setColumnCount (3);
		BanModel_->setHorizontalHeaderLabels ({ tr ("Ban mask"), tr ("Set by"), tr ("Date") });

		ExceptModel_->setColumnCount (3);
		ExceptModel_->setHorizontalHeaderLabels ({ tr ("Except mask"), tr ("Set by"), tr ("Date") });

		InviteModel_->setColumnCount (3);
		InviteModel_->setHorizontalHeaderLabels ({ tr ("Invite mask"), tr ("Set by"), tr ("Date") });

		Ui_.BanList_->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
		Ui_.ExceptList_->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
		Ui_.InviteList_->horizontalHeader ()->setSectionResizeMode (QHeaderView::Stretch);
		Ui_.BanList_->setModel (BanFilterModel_);
		Ui_.ExceptList_->setModel (ExceptFilterModel_);
		Ui_.InviteList_->setModel (InviteFilterModel_);
		BanFilterModel_->setSourceModel (BanModel_);
		BanFilterModel_->setDynamicSortFilter (true);
		BanFilterModel_->setFilterKeyColumn (-1);
		ExceptFilterModel_->setSourceModel (ExceptModel_);
		ExceptFilterModel_->setDynamicSortFilter (true);
		ExceptFilterModel_->setFilterKeyColumn (-1);
		InviteFilterModel_->setSourceModel (InviteModel_);
		InviteFilterModel_->setDynamicSortFilter (true);
		InviteFilterModel_->setFilterKeyColumn (-1);

		ChannelMode_ = ChannelEntry_->GetChannelModes ();
		HandleNewChannelModes (ChannelMode_);

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
				&ChannelConfigWidget::HandleNewChannelModes);
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

	void ChannelConfigWidget::SetupListButtons ()
	{
		auto handleButtons = [this] (auto addButton, auto removeButton, auto updateButton,
				auto textEdit, auto list, auto adder, auto remover)
		{
			auto addHandler = [=, this]
			{
				const auto& text = textEdit->text ();
				if (!text.isEmpty ())
					(ChannelEntry_->*adder) (text);
			};
			auto removeHandler = [=, this]
			{
				const auto& idx = list->currentIndex ();
				if (idx.isValid ())
					(ChannelEntry_->*remover) (idx.data ().toString ());
			};

			connect (addButton,
					&QPushButton::clicked,
					addHandler);
			connect (removeButton,
					&QPushButton::clicked,
					removeHandler);
			connect (updateButton,
					&QPushButton::clicked,
					[=]
					{
						if (!textEdit->text ().isEmpty ())
						{
							removeHandler ();
							addHandler ();
						}
					});
		};

		handleButtons (Ui_.AddBan_, Ui_.RemoveBan_, Ui_.UpdateBan_,
				Ui_.BanHostMask_, Ui_.BanList_,
				&ChannelCLEntry::AddBanListItem, &ChannelCLEntry::RemoveBanListItem);
		handleButtons (Ui_.AddExcept_, Ui_.RemoveExcept_, Ui_.UpdateExcept_,
				Ui_.ExceptHostMask_, Ui_.ExceptList_,
				&ChannelCLEntry::AddExceptListItem, &ChannelCLEntry::RemoveExceptListItem);
		handleButtons (Ui_.AddInvite_, Ui_.RemoveInvite_, Ui_.UpdateInvite_,
				Ui_.InviteHostMask_, Ui_.InviteList_,
				&ChannelCLEntry::AddInviteListItem, &ChannelCLEntry::RemoveInviteListItem);
	}

	void ChannelConfigWidget::RerequestList (int index)
	{
		switch (index)
		{
		case 0:
			BanModel_->clear ();
			ChannelEntry_->RequestBanList ();
			break;
		case 1:
			ExceptModel_->clear ();
			ChannelEntry_->RequestExceptList ();
			break;
		case 2:
			InviteModel_->clear ();
			ChannelEntry_->RequestInviteList ();
			break;
		}
	}

	void ChannelConfigWidget::HandleNewChannelModes (const ChannelModes& modes)
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
