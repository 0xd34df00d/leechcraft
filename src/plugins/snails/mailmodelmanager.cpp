/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "mailmodelmanager.h"
#include <QStandardItemModel>
#include <util/util.h>

namespace LeechCraft
{
namespace Snails
{
	MailModelManager::MailModelManager (QObject *parent)
	: QObject (parent)
	, Model_ (new QStandardItemModel (this))
	, CurrentFolder_ ("INBOX")
	{
		clear ();
	}

	QAbstractItemModel* MailModelManager::GetModel () const
	{
		return Model_;
	}

	void MailModelManager::UpdateReadStatus (const QByteArray& id, bool isRead)
	{
		if (!MailID2Item_.contains (id))
			return;

		QStandardItem *item = MailID2Item_ [id];
		const QModelIndex& sIdx = item->index ();
		for (int i = 0; i < MailColumns::Max; ++i)
		{
			QStandardItem *other = Model_->
					itemFromIndex (sIdx.sibling (sIdx.row (), i));
			other->setData (isRead, MailRole::ReadStatus);
		}
		QMetaObject::invokeMethod (Model_,
				"dataChanged",
				Q_ARG (QModelIndex, sIdx.sibling (sIdx.row (), 0)),
				Q_ARG (QModelIndex, sIdx.sibling (sIdx.row (), MailColumns::Max - 1)));
	}

	void MailModelManager::SetCurrentFolder (const QStringList& folder)
	{
		CurrentFolder_ = folder;
	}

	void MailModelManager::clear ()
	{
		Model_->clear ();
		MailID2Item_.clear ();

		QStringList headers;
		headers << tr ("From")
				<< tr ("Subject")
				<< tr ("Date")
				<< tr ("Size");
		Model_->setHorizontalHeaderLabels (headers);
	}

	void MailModelManager::appendMessages (const QList<Message_ptr>& messages)
	{
		Q_FOREACH (Message_ptr message, messages)
		{
			if (!message->GetFolders ().contains (CurrentFolder_))
				continue;

			if (MailID2Item_.contains (message->GetID ()))
				Model_->removeRow (MailID2Item_ [message->GetID ()]->row ());

			QList<QStandardItem*> row;
			row << new QStandardItem (GetNiceMail (message->GetAddress (Message::Address::From)));
			row << new QStandardItem (message->GetSubject ());
			row << new QStandardItem (message->GetDate ().toString ());
			row << new QStandardItem (Util::MakePrettySize (message->GetSize ()));
			Model_->appendRow (row);

			row [MailColumns::From]->setData (row [MailColumns::From]->text (), MailRole::Sort);
			row [MailColumns::Subj]->setData (row [MailColumns::Subj]->text (), MailRole::Sort);
			row [MailColumns::Date]->setData (message->GetDate (), MailRole::Sort);
			row [MailColumns::Size]->setData (message->GetSize (), MailRole::Sort);

			Q_FOREACH (auto item, row)
				item->setData (message->GetID (), MailRole::ID);
			MailID2Item_ [message->GetID ()] = row.first ();

			UpdateReadStatus (message->GetID (), message->IsRead ());
		}
	}

	void MailModelManager::replaceMessages (const QList<Message_ptr>& messages)
	{
		clear ();
		appendMessages (messages);
	}
}
}
