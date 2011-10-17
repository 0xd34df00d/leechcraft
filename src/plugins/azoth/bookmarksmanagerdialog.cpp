/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "bookmarksmanagerdialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include "interfaces/imucjoinwidget.h"
#include "interfaces/imucbookmarkeditorwidget.h"
#include "interfaces/iaccount.h"
#include "core.h"
#include "interfaces/isupportbookmarks.h"

namespace LeechCraft
{
namespace Azoth
{
	BookmarksManagerDialog::BookmarksManagerDialog (QWidget *parent)
	: QDialog (parent)
	, BMModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.BookmarksTree_->setModel (BMModel_);

		connect (Ui_.BookmarksTree_->selectionModel (),
				SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (handleCurrentBMChanged (const QModelIndex&)));

		Q_FOREACH (IProtocol *proto, Core::Instance ().GetProtocols ())
		{
			QWidget *widget = proto->GetMUCJoinWidget ();
			IMUCJoinWidget *joiner = qobject_cast<IMUCJoinWidget*> (widget);
			if (!joiner)
				continue;

			Proto2Joiner_ [proto->GetProtocolID ()] = joiner;

			Q_FOREACH (QObject *accObj, proto->GetRegisteredAccounts ())
			{
				IAccount *account = qobject_cast<IAccount*> (accObj);
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
						QVariant::fromValue<IAccount*> (account));

				connect (accObj,
						SIGNAL (bookmarksChanged ()),
						this,
						SLOT (handleBookmarksChanged ()));
			}
		}

		if (Ui_.AccountBox_->count ())
			on_AccountBox__currentIndexChanged (0);
	}

	void BookmarksManagerDialog::FocusOn (IAccount *acc)
	{
		const QVariant& accVar =
				QVariant::fromValue<IAccount*> (acc);

		for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
			if (Ui_.AccountBox_->itemData (i) == accVar)
			{
				Ui_.AccountBox_->setCurrentIndex (i);
				break;
			}
	}

	void BookmarksManagerDialog::SuggestSaving (QObject *entryObj)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "object doesn't implement ICLEntry"
					<< entryObj;
			return;
		}

		IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (entryObj);
		if (!mucEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< "object doesn't implement IMUCEntry"
					<< entryObj;
			return;
		}

		const QVariantMap& data = mucEntry->GetIdentifyingData ();
		if (data.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty identifying data returned by"
					<< entryObj;
			return;
		}

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		bool found = false;
		for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
			if (account == Ui_.AccountBox_->itemData (i).value<IAccount*> ())
			{
				Ui_.AccountBox_->setCurrentIndex (i);
				on_AccountBox__currentIndexChanged (i);
				found = true;
				break;
			}

		if (!found)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find parent protocol for entry"
					<< entryObj
					<< entry->GetEntryID ();
			return;
		}

		if (CurrentEditor_)
			CurrentEditor_->SetIdentifyingData (data);
		on_AddButton__released ();
		CurrentEditor_->SetIdentifyingData (data);
	}

	void BookmarksManagerDialog::on_AccountBox__currentIndexChanged (int index)
	{
		BMModel_->clear ();
		CurrentEditor_ = 0;

		IAccount *account = Ui_.AccountBox_->itemData (index).value<IAccount*> ();
		ISupportBookmarks *supBms = qobject_cast<ISupportBookmarks*> (account->GetObject ());
		IProtocol *proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());
		IMUCJoinWidget *joiner = Proto2Joiner_ [proto->GetProtocolID ()];
		if (!joiner)
		{
			qWarning () << Q_FUNC_INFO
					<< "null joiner for"
					<< account->GetAccountID ()
					<< proto->GetProtocolID ();
			return;
		}

		qDebug () << Q_FUNC_INFO << supBms->GetBookmarkedMUCs ().size ();

		const QByteArray& accId = account->GetAccountID ();
		Q_FOREACH (const QVariant& var, supBms->GetBookmarkedMUCs ())
		{
			const QVariantMap& map = var.toMap ();
			if (map.value ("AccountID").toByteArray () != accId)
				continue;

			const QString& name = map.value ("HumanReadableName").toString ();
			if (name.isEmpty ())
				continue;

			QStandardItem *item = new QStandardItem (name);
			item->setData (var);
			BMModel_->appendRow (item);
		}

		while (Ui_.BMFrameLayout_->count ())
		{
			QLayoutItem *item = Ui_.BMFrameLayout_->takeAt (0);
			delete item->widget ();
			delete item;
		}
		QWidget *w = supBms->GetMUCBookmarkEditorWidget ();
		CurrentEditor_ = qobject_cast<IMUCBookmarkEditorWidget*> (w);
		if (CurrentEditor_)
			Ui_.BMFrameLayout_->addWidget (w);
	}

	void BookmarksManagerDialog::handleBookmarksChanged ()
	{
		const int curIdx = Ui_.AccountBox_->currentIndex ();

		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (acc != Ui_.AccountBox_->itemData (curIdx).value<IAccount*> ())
			return;

		on_AccountBox__currentIndexChanged (curIdx);
	}

	void BookmarksManagerDialog::handleCurrentBMChanged (const QModelIndex& current)
	{
		if (!current.isValid ())
			return;

		QStandardItem *item = BMModel_->itemFromIndex (current);
		if (!item || !CurrentEditor_)
			return;

		CurrentEditor_->SetIdentifyingData (item->data ().toMap ());
	}

	void BookmarksManagerDialog::Save ()
	{
		QVariantList datas;
		for (int i = 0; i < BMModel_->rowCount (); ++i)
			datas << BMModel_->item (i)->data ();

		const int index = Ui_.AccountBox_->currentIndex ();
		IAccount *account = Ui_.AccountBox_->itemData (index).value<IAccount*> ();
		qobject_cast<ISupportBookmarks*> (account->GetObject ())->SetBookmarkedMUCs (datas);

		on_AccountBox__currentIndexChanged (index);
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

	void BookmarksManagerDialog::on_RemoveButton__released ()
	{
		QStandardItem *item = GetSelectedItem ();
		if (!item)
			return;

		const QVariantMap& data = item->data ().toMap ();

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
		QStandardItem *selected = GetSelectedItem ();
		const QVariantMap& data = selected ?
				selected->data ().toMap () :
				(CurrentEditor_ ?
					CurrentEditor_->GetIdentifyingData () :
					QVariantMap ());

		QStandardItem *item = new QStandardItem (data.value ("HumanReadableName").toString ());
		item->setData (data);
		BMModel_->appendRow (item);
		Ui_.BookmarksTree_->setCurrentIndex (BMModel_->indexFromItem (item));
	}

	void BookmarksManagerDialog::on_ApplyButton__released()
	{
		QStandardItem *selected = GetSelectedItem ();
		if (!selected ||
				!CurrentEditor_ ||
				CurrentEditor_->GetIdentifyingData ().isEmpty ())
			return;

		selected->setData (CurrentEditor_->GetIdentifyingData ());
		Save ();
	}
}
}
