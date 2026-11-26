/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountslistwidget.h"
#include <QMenu>
#include <QWizard>
#include <QMessageBox>
#include <QComboBox>
#include <QSettings>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iprotocol.h"
#include "components/managers/accountsmanager.h"
#include "components/util/misc.h"
#include "customchatstylemanager.h"
#include "chatstyleoptionmanager.h"

Q_DECLARE_METATYPE (LC::Azoth::ChatStyleOptionManager*)

namespace LC
{
namespace Azoth
{
	namespace
	{
		class AccountListDelegate : public QStyledItemDelegate
		{
		public:
			QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const;
			void setEditorData (QWidget*, const QModelIndex&) const;
			void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const;
		};

		QWidget* AccountListDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			auto getStyler = [&index] (AccountsListWidget::Role role)
				{ return index.data (role).value<ChatStyleOptionManager*> (); };

			switch (index.column ())
			{
			case AccountsListWidget::Column::ChatStyle:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::ChatStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetStyleModel ());
				connect (box,
						&QComboBox::currentTextChanged,
						chatStyler,
						&ChatStyleOptionManager::StyleSelected);
				return box;
			}
			case AccountsListWidget::Column::ChatVariant:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::ChatStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetVariantModel ());
				return box;
			}
			case AccountsListWidget::Column::MUCStyle:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::MUCStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetStyleModel ());
				connect (box,
						&QComboBox::currentTextChanged,
						chatStyler,
						&ChatStyleOptionManager::StyleSelected);
				return box;
			}
			case AccountsListWidget::Column::MUCVariant:
			{
				auto chatStyler = getStyler (AccountsListWidget::Role::MUCStyleManager);

				auto box = new QComboBox (parent);
				box->setModel (chatStyler->GetVariantModel ());
				return box;
			}
			default:
				return QStyledItemDelegate::createEditor (parent, option, index);
			}
		}

		void AccountListDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
		{
			auto box = qobject_cast<QComboBox*> (editor);
			const auto& text = index.data ().toString ();

			switch (index.column ())
			{
			case AccountsListWidget::Column::ChatStyle:
			case AccountsListWidget::Column::ChatVariant:
			case AccountsListWidget::Column::MUCStyle:
			case AccountsListWidget::Column::MUCVariant:
				box->setCurrentIndex (box->findText (text));
				break;
			default:
				QStyledItemDelegate::setEditorData (editor, index);
				return;
			}
		}

		void AccountListDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
		{
			auto box = qobject_cast<QComboBox*> (editor);

			switch (index.column ())
			{
			case AccountsListWidget::Column::ChatStyle:
			case AccountsListWidget::Column::ChatVariant:
			case AccountsListWidget::Column::MUCStyle:
			case AccountsListWidget::Column::MUCVariant:
				model->setData (index, box->currentText ());
				break;
			default:
				QStyledItemDelegate::setModelData (editor, model, index);
				return;
			}
		}
	}

	AccountsListWidget::AccountsListWidget (const AccountsManager& accounts, CustomChatStyleManager& styles, QWidget* parent)
	: QWidget { parent }
	, CustomChatStyleManager_ { styles }
	, AccModel_ { new QStandardItemModel { this } }
	{
		Ui_.setupUi (this);

		Ui_.Accounts_->setItemDelegate (new AccountListDelegate);

		AccModel_->setHorizontalHeaderLabels ({ tr ("Name"),
				tr ("Chat style"), tr ("Variant"), tr ("MUC style"), tr ("MUC variant") });
		connect (AccModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));

		Ui_.Accounts_->setModel (AccModel_);

		const auto& fm = fontMetrics ();
		Ui_.Accounts_->setColumnWidth (Column::Name, fm.horizontalAdvance ("Some typical very long account name"));
		Ui_.Accounts_->setColumnWidth (Column::ChatStyle, fm.horizontalAdvance ("Some typical style"));
		Ui_.Accounts_->setColumnWidth (Column::ChatVariant, fm.horizontalAdvance ("Some typical style variant (alternate)"));
		Ui_.Accounts_->setColumnWidth (Column::MUCStyle, fm.horizontalAdvance ("Some typical style"));
		Ui_.Accounts_->setColumnWidth (Column::MUCVariant, fm.horizontalAdvance ("Some typical style variant (alternate)"));

		connect (&accounts,
				&AccountsManager::accountRemoved,
				this,
				[this] (IAccount& acc)
				{
					const auto item = Account2Item_.take (&acc);
					if (!item)
					{
						qWarning () << "account not found";
						return;
					}
					AccModel_->removeRow (item->row ());
				});
		accounts.OnEachAccount (*this, std::bind_front (&AccountsListWidget::AddAccount, this));

		connect (Ui_.Accounts_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleAccountSelected (QModelIndex)));
		handleAccountSelected ({});
	}

	void AccountsListWidget::AddAccount (IAccount& acc)
	{
		auto proto = qobject_cast<IProtocol*> (acc.GetParentProtocol ());

		auto name = new QStandardItem (acc.GetAccountName ());
		name->setIcon (proto ? proto->GetProtocolIcon () : QIcon ());
		name->setEditable (false);

		const auto& stylePair = CustomChatStyleManager_.GetStyleForAccount (&acc);
		auto style = new QStandardItem ();
		style->setText (stylePair.first);
		auto variant = new QStandardItem ();
		variant->setText (stylePair.second);

		const auto& mucPair = CustomChatStyleManager_.GetMUCStyleForAccount (&acc);
		auto mucStyle = new QStandardItem ();
		mucStyle->setText (mucPair.first);
		auto mucVariant = new QStandardItem ();
		mucVariant->setText (mucPair.second);

		auto chatStyler = new ChatStyleOptionManager (QByteArray (), acc.GetQObject ());
		chatStyler->AddEmptyVariant ();
		auto mucStyler = new ChatStyleOptionManager (QByteArray (), acc.GetQObject ());
		mucStyler->AddEmptyVariant ();

		const QList row { name, style, variant, mucStyle, mucVariant };
		for (auto item : row)
		{
			item->setData (QVariant::fromValue<IAccount*> (&acc), Role::AccObj);
			item->setData (QVariant::fromValue<ChatStyleOptionManager*> (chatStyler), Role::ChatStyleManager);
			item->setData (QVariant::fromValue<ChatStyleOptionManager*> (mucStyler), Role::MUCStyleManager);
		}
		AccModel_->appendRow (row);

		for (auto item : { style, variant, mucStyle, mucVariant })
			Ui_.Accounts_->openPersistentEditor (item->index ());

		Account2Item_ [&acc] = name;
	}

	void AccountsListWidget::on_Add__released ()
	{
		InitiateAccountAddition (this);
	}

	void AccountsListWidget::on_Modify__released ()
	{
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		index.data (Role::AccObj).value<IAccount*> ()->OpenConfigurationDialog ();
	}

	void AccountsListWidget::on_Delete__released()
	{
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		RemoveAccount (index.data (Role::AccObj).value<IAccount*> ());
	}

	void AccountsListWidget::on_ResetStyles__released ()
	{
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		const auto row = index.row ();
		for (auto col : { Column::ChatStyle, Column::ChatVariant, Column::MUCStyle, Column::MUCVariant })
			AccModel_->item (row, col)->setText ({});
	}

	void AccountsListWidget::handleAccountSelected (const QModelIndex& index)
	{
		const auto isValid = index.isValid ();

		const auto items =
		{
			Ui_.Delete_,
			Ui_.Modify_,
			Ui_.ResetStyles_,
		};
		for (const auto item : items)
			item->setEnabled (isValid);
	}

	void AccountsListWidget::handleItemChanged (QStandardItem *item)
	{
		auto acc = item->data (Role::AccObj).value<IAccount*> ();
		const auto& text = item->text ();
		switch (item->column ())
		{
		case Column::ChatStyle:
			CustomChatStyleManager_.Set (acc, CustomChatStyleManager::Settable::ChatStyle, text);
			break;
		case Column::ChatVariant:
			CustomChatStyleManager_.Set (acc, CustomChatStyleManager::Settable::ChatVariant, text);
			break;
		case Column::MUCStyle:
			CustomChatStyleManager_.Set (acc, CustomChatStyleManager::Settable::MUCStyle, text);
			break;
		case Column::MUCVariant:
			CustomChatStyleManager_.Set (acc, CustomChatStyleManager::Settable::MUCVariant, text);
			break;
		default:
			break;
		}
	}
}
}
