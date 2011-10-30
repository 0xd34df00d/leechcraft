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

#include "groupsenddialog.h"
#include <boost/function.hpp>
#include <QStandardItemModel>
#include "interfaces/iclentry.h"
#include "interfaces/imessage.h"
#include "core.h"
#include <boost/bind.hpp>

namespace LeechCraft
{
namespace Azoth
{
	GroupSendDialog::GroupSendDialog (const QList<QObject*>& entries,
			QWidget *parent)
	: QDialog (parent)
	, ContactsModel_ (new QStandardItemModel (this))
	{
		ContactsModel_->setHorizontalHeaderLabels (QStringList (tr ("Name")) << tr ("ID"));

		Q_FOREACH (QObject *entryObj, entries)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
				continue;

			QList<QStandardItem*> row;
			row << new QStandardItem (entry->GetEntryName ());
			row << new QStandardItem (entry->GetHumanReadableID ());
			row.first ()->setIcon (Core::Instance ()
						.GetIconForState (entry->GetStatus ().State_));
			row.first ()->setData (QVariant::fromValue<QObject*> (entryObj));
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
		const QString& msg = Ui_.Message_->toPlainText ();

		Q_FOREACH (QStandardItem *item, Entry2Item_.values ())
		{
			if (item->checkState () != Qt::Checked)
				continue;

			QObject *entryObj = item->data ().value<QObject*> ();
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

			QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage,
					QString (), msg);
			IMessage *msg = qobject_cast<IMessage*> (msgObj);
			if (!msg)
				continue;

			msg->Send ();
			Core::Instance ().IncreaseUnreadCount (entry, -1);
		}

		Ui_.Message_->clear ();
	}

	void GroupSendDialog::on_AllButton__released()
	{
		Q_FOREACH (QStandardItem *item, Entry2Item_.values ())
			item->setCheckState (Qt::Checked);
	}

	void GroupSendDialog::on_NoneButton__released()
	{
		Q_FOREACH (QStandardItem *item, Entry2Item_.values ())
			item->setCheckState (Qt::Unchecked);
	}

	namespace
	{
		void MarkOnly (const QList<QStandardItem*>& items, boost::function<bool (State)> f)
		{
			Q_FOREACH (QStandardItem *item, items)
			{
				QObject *entryObj = item->data ().value<QObject*> ();
				ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

				const Qt::CheckState state = f (entry->GetStatus ().State_) ?
						Qt::Checked :
						Qt::Unchecked;
				item->setCheckState (state);
			}
		}
	}

	void GroupSendDialog::on_OnlineButton__released ()
	{
		MarkOnly (Entry2Item_.values (), !boost::bind (std::equal_to<State> (), SOffline, _1));
	}

	void GroupSendDialog::on_OfflineButton__released ()
	{
		MarkOnly (Entry2Item_.values (), boost::bind (std::equal_to<State> (), SOffline, _1));
	}

	void GroupSendDialog::handleEntryStatusChanged ()
	{
		QStandardItem *item = Entry2Item_ [sender ()];
		if (!item)
			return;

		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		const QIcon& icon = Core::Instance ()
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
