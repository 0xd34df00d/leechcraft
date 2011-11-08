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

#include "mailtab.h"
#include <QToolBar>
#include <QStandardItemModel>
#include "core.h"
#include "storage.h"

namespace LeechCraft
{
namespace Snails
{
	MailTab::MailTab (const TabClassInfo& tc, QObject *pmt, QWidget *parent)
	: QWidget (parent)
	, TabToolbar_ (new QToolBar)
	, TabClass_ (tc)
	, PMT_ (pmt)
	, MailModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);

		Ui_.AccountsTree_->setModel (Core::Instance ().GetAccountsModel ());
		Ui_.MailTree_->setModel (MailModel_);

		connect (Ui_.AccountsTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentAccountChanged (QModelIndex)));

		QAction *fetch = new QAction (tr ("Fetch new mail"), this);
		TabToolbar_->addAction (fetch);
		connect (fetch,
				SIGNAL (triggered ()),
				this,
				SLOT (handleFetchNewMail ()));
	}

	TabClassInfo MailTab::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* MailTab::ParentMultiTabs ()
	{
		return PMT_;
	}

	void MailTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* MailTab::GetToolBar () const
	{
		return TabToolbar_;
	}

	void MailTab::handleCurrentAccountChanged (const QModelIndex& idx)
	{
		MailModel_->clear ();
		if (CurrAcc_)
			disconnect (CurrAcc_.get (),
					0,
					this,
					0);

		CurrAcc_ = Core::Instance ().GetAccount (idx);
		if (!CurrAcc_)
			return;

		connect (CurrAcc_.get (),
				SIGNAL (gotNewMessages (QList<Message_ptr>)),
				this,
				SLOT (handleGotNewMessages (QList<Message_ptr>)));

		QStringList headers;
		headers << tr ("From")
				<< tr ("Subject")
				<< tr ("Date")
				<< tr ("Size");
		MailModel_->setHorizontalHeaderLabels (headers);

		handleGotNewMessages (Core::Instance ().GetStorage ()->
					LoadMessages (CurrAcc_.get ()));
	}

	void MailTab::handleFetchNewMail ()
	{
		Q_FOREACH (auto acc, Core::Instance ().GetAccounts ())
			acc->FetchNewHeaders (1);
	}

	void MailTab::handleGotNewMessages (QList<Message_ptr> messages)
	{
		Q_FOREACH (Message_ptr message, messages)
		{
			const QString& fromName = message->GetFrom ();
			const QString& from = fromName.isEmpty () ?
					message->GetFromEmail () :
					fromName + " <" + message->GetFromEmail () + ">";
			QList<QStandardItem*> row;
			row << new QStandardItem (from);
			row << new QStandardItem (message->GetSubject ());
			row << new QStandardItem (message->GetDate ().toString ());
			row << new QStandardItem (QString::number (message->GetSize ()));
			MailModel_->appendRow (row);
		}
	}
}
}
