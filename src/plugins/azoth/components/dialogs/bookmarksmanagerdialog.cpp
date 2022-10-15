/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarksmanagerdialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include "interfaces/azoth/imucjoinwidget.h"
#include "interfaces/azoth/imucbookmarkeditorwidget.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/isupportbookmarks.h"
#include "interfaces/azoth/imucprotocol.h"
#include "core.h"
#include "bookmarkeditdialog.h"

namespace LC
{
namespace Azoth
{
	BookmarksManagerDialog::BookmarksManagerDialog (QWidget *parent)
	: QDialog (parent)
	, BMModel_ (new QStandardItemModel (this))
	{
		setAttribute (Qt::WA_DeleteOnClose, true);

		Ui_.setupUi (this);
		Ui_.BookmarksTree_->setModel (BMModel_);

		for (auto proto : Core::Instance ().GetProtocols ())
		{
			if (!qobject_cast<IMUCProtocol*> (proto->GetQObject ()))
				continue;

			for (auto accObj : proto->GetRegisteredAccounts ())
			{
				const auto account = qobject_cast<IAccount*> (accObj);
				if (!account)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< accObj
							<< "to IAccount for protocol"
							<< proto->GetProtocolID ();
					continue;
				}

				if (!qobject_cast<ISupportBookmarks*> (accObj))
					continue;

				Ui_.AccountBox_->addItem (account->GetAccountName (),
						QVariant::fromValue (account));

				connect (accObj,
						SIGNAL (bookmarksChanged ()),
						this,
						SLOT (handleBookmarksChanged ()));
			}
		}

		if (Ui_.AccountBox_->count ())
			on_AccountBox__currentIndexChanged (0);
	}

	bool BookmarksManagerDialog::FocusOn (IAccount *acc)
	{
		const auto& accVar = QVariant::fromValue<IAccount*> (acc);

		for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
			if (Ui_.AccountBox_->itemData (i) == accVar)
			{
				Ui_.AccountBox_->setCurrentIndex (i);
				on_AccountBox__currentIndexChanged (i);
				return true;
			}

		return false;
	}

	namespace
	{
		QVariantMap GetIdentifyingData (QObject *entryObj)
		{
			const auto mucEntry = qobject_cast<IMUCEntry*> (entryObj);
			if (!mucEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< "object doesn't implement IMUCEntry"
						<< entryObj;
				return {};
			}

			return mucEntry->GetIdentifyingData ();
		}
	}

	void BookmarksManagerDialog::SuggestSaving (QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "object doesn't implement ICLEntry"
					<< entryObj;
			return;
		}

		const bool found = FocusOn (entry->GetParentAccount ());

		if (!found)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find parent protocol for entry"
					<< entryObj
					<< entry->GetEntryID ();
			return;
		}

		const auto& data = GetIdentifyingData (entryObj);
		if (data.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty identifying data returned by"
					<< entryObj;
			return;
		}

		AddBookmark (data);
	}

	void BookmarksManagerDialog::on_AccountBox__currentIndexChanged (int index)
	{
		CurrentAccount_ = Ui_.AccountBox_->itemData (index).value<IAccount*> ();

		ReloadModel ();
	}

	void BookmarksManagerDialog::handleBookmarksChanged ()
	{
		const int curIdx = Ui_.AccountBox_->currentIndex ();

		const auto acc = qobject_cast<IAccount*> (sender ());
		if (acc != Ui_.AccountBox_->itemData (curIdx).value<IAccount*> ())
			return;

		ReloadModel ();
	}

	void BookmarksManagerDialog::Save ()
	{
		if (!CurrentAccount_)
		{
			qWarning () << Q_FUNC_INFO
					<< "no current account";
			return;
		}

		QVariantList datas;
		for (int i = 0; i < BMModel_->rowCount (); ++i)
			datas << BMModel_->item (i)->data ();

		qobject_cast<ISupportBookmarks*> (CurrentAccount_->GetQObject())->SetBookmarkedMUCs (datas);

		ReloadModel ();
	}

	QStandardItem* BookmarksManagerDialog::GetSelectedItem () const
	{
		const QModelIndex& currentIdx = Ui_.BookmarksTree_->currentIndex ();
		if (!currentIdx.isValid ())
			return 0;

		QStandardItem *item = BMModel_->itemFromIndex (currentIdx);
		if (!item)
			qWarning () << Q_FUNC_INFO
					<< "null item for index"
					<< currentIdx;

		return item;
	}

	void BookmarksManagerDialog::ReloadModel ()
	{
		BMModel_->clear ();
		const auto supBms = qobject_cast<ISupportBookmarks*> (CurrentAccount_->GetQObject ());
		const QByteArray& accId = CurrentAccount_->GetAccountID ();
		for (const auto& var : supBms->GetBookmarkedMUCs ())
		{
			const auto& map = var.toMap ();
			if (map.value ("AccountID").toByteArray () != accId)
				continue;

			const auto& name = map.value ("HumanReadableName").toString ();
			if (name.isEmpty ())
				continue;

			const auto item = new QStandardItem (name);
			item->setData (var);
			BMModel_->appendRow (item);
		}
	}

	void BookmarksManagerDialog::AddBookmark (const QVariantMap& initialData)
	{
		BookmarkEditDialog editDia { initialData, CurrentAccount_, this };
		editDia.setWindowTitle (tr ("Add bookmark for account %1")
					.arg (CurrentAccount_->GetAccountName ()));
		if (editDia.exec () != QDialog::Accepted)
			return;

		const auto& data = editDia.GetIdentifyingData ();
		const auto item = new QStandardItem (data.value ("HumanReadableName").toString ());
		item->setData (data);
		BMModel_->appendRow (item);

		Save ();
	}

	void BookmarksManagerDialog::on_RemoveButton__released ()
	{
		QStandardItem *item = GetSelectedItem ();
		if (!item)
			return;

		const auto& data = item->data ().toMap ();

		if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to delete the bookmark %1?")
					.arg (data.value ("HumanReadableName").toString ()),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		BMModel_->removeRow (item->row ());

		Save ();
	}

	void BookmarksManagerDialog::on_AddButton__released ()
	{
		AddBookmark ({});
	}

	void BookmarksManagerDialog::on_ModifyButton__released()
	{
		const auto selected = GetSelectedItem ();
		if (!selected)
			return;

		const auto& sourceData = selected->data ().toMap ();

		BookmarkEditDialog editDia { sourceData, CurrentAccount_, this };
		editDia.setWindowTitle (tr ("Edit bookmark for account %1")
					.arg (CurrentAccount_->GetAccountName ()));
		if (editDia.exec () != QDialog::Accepted)
			return;

		const auto& data = editDia.GetIdentifyingData ();
		if (data == sourceData)
			return;

		selected->setText (data.value ("HumanReadableName").toString ());
		selected->setData (data);

		Save ();
	}

	void BookmarksManagerDialog::on_MoveUp__released ()
	{
		const auto selected = GetSelectedItem ();
		if (!selected)
			return;

		const int row = selected->row ();
		if (row <= 0)
			return;

		auto items = BMModel_->takeRow (row);
		BMModel_->insertRow (row - 1, items);

		Save ();
	}

	void BookmarksManagerDialog::on_MoveDown__released ()
	{
		const auto selected = GetSelectedItem ();
		if (!selected)
			return;

		const int row = selected->row ();
		if (row >= BMModel_->rowCount () - 1)
			return;

		auto items = BMModel_->takeRow (row + 1);
		BMModel_->insertRow (row, items);

		Save ();
	}
}
}
