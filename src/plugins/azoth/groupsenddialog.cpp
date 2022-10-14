/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "groupsenddialog.h"
#include <QStandardItemModel>
#include <util/sll/qtutil.h>
#include <util/sll/slotclosure.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imessage.h"
#include "core.h"
#include "resourcesmanager.h"
#include "msgsender.h"

namespace LC
{
namespace Azoth
{
	GroupSendDialog::GroupSendDialog (const QList<ICLEntry*>& entries, QWidget *parent)
	: QDialog (parent)
	, ContactsModel_ (new QStandardItemModel (this))
	{
		ContactsModel_->setHorizontalHeaderLabels ({ tr ("Name"), tr ("ID") });

		auto& rmi = ResourcesManager::Instance ();

		for (const auto& entry : entries)
		{
			QList<QStandardItem*> row
			{
				new QStandardItem { entry->GetEntryName () },
				new QStandardItem { entry->GetHumanReadableID () }
			};
			const auto item = row.first ();
			item->setIcon (rmi.GetIconForState (entry->GetStatus ().State_));
			item->setCheckable (true);
			item->setCheckState (Qt::Checked);

			ContactsModel_->appendRow (row);

			Entry2Item_ [entry] = item;

			// TODO this won't be needed once raw ICLEntry pointers are gone
			connect (entry->GetQObject (),
					&QObject::destroyed,
					this,
					[this, entry]
					{
						const auto item = Entry2Item_.take (entry);
						qDeleteAll (ContactsModel_->takeRow (item->row ()));
					});
			new Util::SlotClosure<Util::NoDeletePolicy> ([entry, item]
					{
						const auto& icon = ResourcesManager::Instance ().GetIconForState (entry->GetStatus ().State_);
						item->setIcon (icon);
					},
					entry->GetQObject (),
					SIGNAL (statusChanged (EntryStatus, QString)),
					this);
		}

		Ui_.setupUi (this);
		Ui_.Contacts_->setModel (ContactsModel_);

		Ui_.OnlineButton_->setIcon (rmi.GetIconForState (SOnline));
		Ui_.OfflineButton_->setIcon (rmi.GetIconForState (SOffline));
	}

	void GroupSendDialog::on_Message__textChanged ()
	{
		Ui_.SendButton_->setEnabled (!Ui_.Message_->toPlainText ().isEmpty ());
	}

	void GroupSendDialog::on_SendButton__released ()
	{
		const auto& msg = Ui_.Message_->toPlainText ();

		for (const auto [entry, item] : Util::Stlize (Entry2Item_))
		{
			if (item->checkState () != Qt::Checked)
				continue;

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
			for (const auto [entry, item] : Util::Stlize (items))
			{
				const auto state = f (entry->GetStatus ().State_) ?
						Qt::Checked :
						Qt::Unchecked;
				item->setCheckState (state);
			}
		}
	}

	void GroupSendDialog::on_OnlineButton__released ()
	{
		MarkOnly (Entry2Item_, [] (State st) { return st != SOffline && st != SError && st != SInvalid; });
	}

	void GroupSendDialog::on_OfflineButton__released ()
	{
		MarkOnly (Entry2Item_, [] (State st) { return st == SOffline; });
	}
}
}
