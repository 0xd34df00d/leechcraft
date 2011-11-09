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
#include <QTextDocument>
#include <QSortFilterProxyModel>
#include <util/util.h>
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
	, MailSortFilterModel_ (new QSortFilterProxyModel (this))
	{
		Ui_.setupUi (this);

		Ui_.AccountsTree_->setModel (Core::Instance ().GetAccountsModel ());

		MailSortFilterModel_->setDynamicSortFilter (true);
		MailSortFilterModel_->setSortRole (RSort);
		MailSortFilterModel_->setSourceModel (MailModel_);
		MailSortFilterModel_->sort (2, Qt::DescendingOrder);
		Ui_.MailTree_->setModel (MailSortFilterModel_);

		connect (Ui_.AccountsTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentAccountChanged (QModelIndex)));
		connect (Ui_.MailTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleMailSelected (QModelIndex)));

		QAction *fetch = new QAction (tr ("Fetch new mail"), this);
		fetch->setProperty ("ActionIcon", "fetchall");
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
		MailID2Item_.clear ();
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

	namespace
	{
		QString GetFrom (Message_ptr message)
		{
			const QString& fromName = message->GetFrom ();
			return fromName.isEmpty () ?
					message->GetFromEmail () :
					fromName + " <" + message->GetFromEmail () + ">";
		}
	}

	void MailTab::handleMailSelected (const QModelIndex& sidx)
	{
		if (!CurrAcc_)
		{
			Ui_.MailView_->setHtml (QString ());
			return;
		}

		const QModelIndex& idx = MailSortFilterModel_->mapToSource (sidx);
		const QByteArray& id = idx.sibling (idx.row (), 0)
				.data (RID).toByteArray ();

		Message_ptr msg;
		try
		{
			msg = Core::Instance ().GetStorage ()->
					LoadMessage (CurrAcc_.get (), id);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to load message"
					<< CurrAcc_->GetID ().toHex ()
					<< id.toHex ()
					<< e.what ();

			const QString& html = tr ("<h2>Unable to load mail</h2><em>%1</em>").arg (e.what ());
			Ui_.MailView_->setHtml (html);
			return;
		}

		if (!msg->IsFullyFetched ())
			CurrAcc_->FetchWholeMessage (msg->GetID ());

		QString html;
		html += "<em>Subject</em>: %1<br />";
		html += "<em>From</em>: %2<br />";
		html += "<em>On</em>: %3<hr />";
		html += "%4";

		const QString& htmlBody = msg->GetHTMLBody ();

		Ui_.MailView_->setHtml (html
				.arg (msg->GetSubject ())
				.arg (Qt::escape (GetFrom (msg)))
				.arg (msg->GetDate ().toString ())
				.arg (htmlBody.isEmpty () ?
						"<pre>" + Qt::escape (msg->GetBody ()) + "</pre> " :
						htmlBody));
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
			if (MailID2Item_.contains (message->GetID ()))
				MailModel_->removeRow (MailID2Item_ [message->GetID ()]->row ());

			QList<QStandardItem*> row;
			row << new QStandardItem (GetFrom (message));
			row << new QStandardItem (message->GetSubject ());
			row << new QStandardItem (message->GetDate ().toString ());
			row << new QStandardItem (Util::MakePrettySize (message->GetSize ()));
			MailModel_->appendRow (row);

			row [CFrom]->setData (row [CFrom]->text (), RSort);
			row [CSubj]->setData (row [CSubj]->text (), RSort);
			row [CDate]->setData (message->GetDate (), RSort);
			row [CSize]->setData (message->GetSize (), RSort);

			row [CFrom]->setData (message->GetID (), RID);
			MailID2Item_ [message->GetID ()] = row.first ();
		}
	}
}
}
