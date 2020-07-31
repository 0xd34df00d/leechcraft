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
#include <util/sll/curry.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iprotocol.h"
#ifdef ENABLE_CRYPT
#include "interfaces/azoth/isupportpgp.h"
#include "pgpkeyselectiondialog.h"
#include "cryptomanager.h"
#endif
#include "core.h"
#include "util.h"
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
						SIGNAL (currentIndexChanged (QString)),
						chatStyler,
						SLOT (handleChatStyleSelected (QString)));
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
						SIGNAL (currentIndexChanged (QString)),
						chatStyler,
						SLOT (handleChatStyleSelected (QString)));
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

	AccountsListWidget::AccountsListWidget (QWidget* parent)
	: QWidget (parent)
	, AccModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.Accounts_->setItemDelegate (new AccountListDelegate);

		AccModel_->setHorizontalHeaderLabels ({ tr ("Show"), tr ("Name"),
				tr ("Chat style"), tr ("Variant"), tr ("MUC style"), tr ("MUC variant") });
		connect (AccModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));


		connect (&Core::Instance (),
				SIGNAL (accountAdded (IAccount*)),
				this,
				SLOT (addAccount (IAccount*)));
		connect (&Core::Instance (),
				SIGNAL (accountRemoved (IAccount*)),
				this,
				SLOT (handleAccountRemoved (IAccount*)));

		for (const auto acc : Core::Instance ().GetAccounts ())
			addAccount (acc);

		Ui_.Accounts_->setModel (AccModel_);

		connect (Ui_.Accounts_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleAccountSelected (QModelIndex)));
		handleAccountSelected ({});

		Ui_.Accounts_->setColumnWidth (0, 32);
		const auto& fm = fontMetrics ();
		Ui_.Accounts_->setColumnWidth (1, fm.horizontalAdvance ("Some typical very long account name"));
		Ui_.Accounts_->setColumnWidth (2, fm.horizontalAdvance ("Some typical style"));
		Ui_.Accounts_->setColumnWidth (3, fm.horizontalAdvance ("Some typical style variant (alternate)"));
		Ui_.Accounts_->setColumnWidth (4, fm.horizontalAdvance ("Some typical style"));
		Ui_.Accounts_->setColumnWidth (5, fm.horizontalAdvance ("Some typical style variant (alternate)"));
	}

	void AccountsListWidget::addAccount (IAccount *acc)
	{
		auto proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());

		auto show = new QStandardItem ();
		show->setCheckable (true);
		show->setCheckState (acc->IsShownInRoster () ? Qt::Checked : Qt::Unchecked);
		show->setEditable (false);

		auto name = new QStandardItem (acc->GetAccountName ());
		name->setIcon (proto ? proto->GetProtocolIcon () : QIcon ());
		name->setEditable (false);

		const auto& stylePair = Core::Instance ()
				.GetCustomChatStyleManager ()->GetStyleForAccount (acc);
		auto style = new QStandardItem ();
		style->setText (stylePair.first);
		auto variant = new QStandardItem ();
		variant->setText (stylePair.second);

		const auto& mucPair = Core::Instance ()
				.GetCustomChatStyleManager ()->GetMUCStyleForAccount (acc);
		auto mucStyle = new QStandardItem ();
		mucStyle->setText (mucPair.first);
		auto mucVariant = new QStandardItem ();
		mucVariant->setText (mucPair.second);

		auto chatStyler = new ChatStyleOptionManager (QByteArray (), acc->GetQObject ());
		chatStyler->AddEmptyVariant ();
		auto mucStyler = new ChatStyleOptionManager (QByteArray (), acc->GetQObject ());
		mucStyler->AddEmptyVariant ();

		const QList<QStandardItem*> row { show, name, style, variant, mucStyle, mucVariant };
		for (auto item : row)
		{
			item->setData (QVariant::fromValue<IAccount*> (acc), Role::AccObj);
			item->setData (QVariant::fromValue<ChatStyleOptionManager*> (chatStyler), Role::ChatStyleManager);
			item->setData (QVariant::fromValue<ChatStyleOptionManager*> (mucStyler), Role::MUCStyleManager);
		}
		AccModel_->appendRow (row);

		for (auto item : { style, variant, mucStyle, mucVariant })
			Ui_.Accounts_->openPersistentEditor (item->index ());

		Account2Item_ [acc] = name;
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

	void AccountsListWidget::on_PGP__released ()
	{
#ifdef ENABLE_CRYPT
		const auto& index = Ui_.Accounts_->selectionModel ()->currentIndex ();
		if (!index.isValid ())
			return;

		auto acc = index.data (Role::AccObj).value<IAccount*> ();
		auto pgpAcc = qobject_cast<ISupportPGP*> (acc->GetQObject ());
		if (!pgpAcc)
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("The account %1 doesn't support encryption.")
						.arg (acc->GetAccountName ()));
			return;
		}

		const QString& str = tr ("Please select new PGP key for the account %1.")
				.arg (acc->GetAccountName ());
		PGPKeySelectionDialog dia (str,
				PGPKeySelectionDialog::TPrivate, pgpAcc->GetPrivateKey (), this);
		if (dia.exec () != QDialog::Accepted)
			return;

		pgpAcc->SetPrivateKey (dia.GetSelectedKey ());
		CryptoManager::Instance ().AssociatePrivateKey (acc, dia.GetSelectedKey ());
#endif
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
#ifdef ENABLE_CRYPT
			Ui_.PGP_
#endif
		};
		for (const auto item : items)
			item->setEnabled (isValid);
	}

	void AccountsListWidget::handleItemChanged (QStandardItem *item)
	{
		auto acc = item->data (Role::AccObj).value<IAccount*> ();

		const auto& styleMgr = Core::Instance ().GetCustomChatStyleManager ();
		const auto& text = item->text ();

		switch (item->column ())
		{
		case Column::ShowInRoster:
			acc->SetShownInRoster (item->checkState () == Qt::Checked);

			if (!acc->IsShownInRoster () && acc->GetState ().State_ != SOffline)
				acc->ChangeState ({ SOffline, {} });

			emit accountVisibilityChanged (acc);

			break;
		case Column::ChatStyle:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::ChatStyle, text);
			break;
		case Column::ChatVariant:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::ChatVariant, text);
			break;
		case Column::MUCStyle:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::MUCStyle, text);
			break;
		case Column::MUCVariant:
			styleMgr->Set (acc, CustomChatStyleManager::Settable::MUCVariant, text);
			break;
		default:
			break;
		}
	}

	void AccountsListWidget::handleAccountRemoved (IAccount *acc)
	{
		if (!Account2Item_.contains (acc))
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< acc->GetAccountName ()
					<< acc->GetQObject ()
					<< "from"
					<< sender ()
					<< "not found here";
			return;
		}

		AccModel_->removeRow (Account2Item_ [acc]->row ());
		Account2Item_.remove (acc);
	}
}
}
