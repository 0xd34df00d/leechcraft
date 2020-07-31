/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupsenddialog.h"
#include <QStandardItemModel>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imessage.h"
#include "core.h"
#include "resourcesmanager.h"
#include "msgsender.h"

namespace LC
{
namespace Azoth
{
	GroupSendDialog::GroupSendDialog (const QList<QObject*>& entries,
			QWidget *parent)
	: QDialog (parent)
	, ContactsModel_ (new QStandardItemModel (this))
	{
		ContactsModel_->setHorizontalHeaderLabels (QStringList (tr ("Name")) << tr ("ID"));

		for (const auto entryObj : entries)
		{
			const auto entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
				continue;

			QList<QStandardItem*> row
			{
				new QStandardItem { entry->GetEntryName () },
				new QStandardItem { entry->GetHumanReadableID () }
			};
			row.first ()->setIcon (ResourcesManager::Instance ()
						.GetIconForState (entry->GetStatus ().State_));
			row.first ()->setData (QVariant::fromValue (entryObj));
			row.first ()->setCheckable (true);
			row.first ()->setCheckState (Qt::Checked);

			ContactsModel_->appendRow (row);

			Entry2Item_ [entryObj] = row.first ();

			connect (entryObj,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleEntryDestroyed ()));
			connect (entryObj,
					SIGNAL (statusChanged (EntryStatus, QString)),
					this,
					SLOT (handleEntryStatusChanged ()));
		}

		Ui_.setupUi (this);
		Ui_.Contacts_->setModel (ContactsModel_);
	}

	void GroupSendDialog::on_Message__textChanged ()
	{
		qDebug () << Q_FUNC_INFO;
		Ui_.SendButton_->setEnabled (!Ui_.Message_->toPlainText ().isEmpty ());
	}

	void GroupSendDialog::on_SendButton__released ()
	{
		const auto& msg = Ui_.Message_->toPlainText ();

		for (const auto item : Entry2Item_)
		{
			if (item->checkState () != Qt::Checked)
				continue;

			const auto entryObj = item->data ().value<QObject*> ();
			const auto entry = qobject_cast<ICLEntry*> (entryObj);

			new MsgSender { entry, IMessage::Type::ChatMessage, msg };
			Core::Instance ().IncreaseUnreadCount (entry, -1);
		}

		Ui_.Message_->clear ();
	}

	void GroupSendDialog::on_AllButton__released ()
	{
		for (const auto item : Entry2Item_)
			item->setCheckState (Qt::Checked);
	}

	void GroupSendDialog::on_NoneButton__released ()
	{
		for (const auto item : Entry2Item_)
			item->setCheckState (Qt::Unchecked);
	}

	namespace
	{
		template<typename T, typename F>
		void MarkOnly (const T& items, const F& f)
		{
			for (const auto item : items)
			{
				const auto entryObj = item->data ().template value<QObject*> ();
				const auto entry = qobject_cast<ICLEntry*> (entryObj);

				const auto state = f (entry->GetStatus ().State_) ?
						Qt::Checked :
						Qt::Unchecked;
				item->setCheckState (state);
			}
		}
	}

	void GroupSendDialog::on_OnlineButton__released ()
	{
		MarkOnly (Entry2Item_, [] (State st) { return st != SOffline; });
	}

	void GroupSendDialog::on_OfflineButton__released ()
	{
		MarkOnly (Entry2Item_, [] (State st) { return st == SOffline; });
	}

	void GroupSendDialog::handleEntryStatusChanged ()
	{
		QStandardItem *item = Entry2Item_ [sender ()];
		if (!item)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		const auto& icon = ResourcesManager::Instance ()
				.GetIconForState (entry->GetStatus ().State_);
		item->setIcon (icon);
	}

	void GroupSendDialog::handleEntryDestroyed ()
	{
		QStandardItem *item = Entry2Item_.take (sender ());
		if (!item)
			return;

		qDeleteAll (ContactsModel_->takeRow (item->row ()));
	}
}
}
