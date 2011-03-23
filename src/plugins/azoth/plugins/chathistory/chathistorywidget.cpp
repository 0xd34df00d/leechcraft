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

#include "chathistorywidget.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <interfaces/iaccount.h>
#include <interfaces/iclentry.h>
#include <interfaces/iproxyobject.h>
#include "chathistory.h"

namespace LeechCraft
{
namespace Azoth
{
namespace ChatHistory
{
	Plugin *ChatHistoryWidget::S_ParentMultiTabs_ = 0;
	
	const int Amount = 50;

	void ChatHistoryWidget::SetParentMultiTabs (Plugin *ch)
	{
		S_ParentMultiTabs_ = ch;
	}

	ChatHistoryWidget::ChatHistoryWidget (ICLEntry *entry, QWidget *parent)
	: QWidget (parent)
	, HistoryViewModel_ (new QStandardItemModel (this))
	, ContactsModel_ (new QStandardItemModel (this))
	, SortFilter_ (new QSortFilterProxyModel (this))
	, Backpages_ (0)
	, Toolbar_ (new QToolBar (tr ("Chat history")))
	, EntryToFocus_ (entry)
	{
		Ui_.setupUi (this);
		Ui_.HistView_->setModel (HistoryViewModel_);

		SortFilter_->setDynamicSortFilter (true);
		SortFilter_->setSourceModel (ContactsModel_);
		SortFilter_->sort (0);
		Ui_.Contacts_->setModel (SortFilter_);
		
		connect (Ui_.ContactsSearch_,
				SIGNAL (textChanged (const QString&)),
				SortFilter_,
				SLOT (setFilterFixedString (const QString&)));
		connect (Ui_.Contacts_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (handleContactSelected (const QModelIndex&)));
		
		connect (Core::Instance ().get (),
				SIGNAL (gotUsersForAccount (const QStringList&, const QString&)),
				this,
				SLOT (handleGotUsersForAccount (const QStringList&, const QString&)));
		connect (Core::Instance ().get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				this,
				SLOT (handleGotOurAccounts (const QStringList&)));
		connect (Core::Instance ().get (),
				SIGNAL (gotChatLogs (const QString&, const QString&, int, int, const QVariant&)),
				this,
				SLOT (handleGotChatLogs (const QString&, const QString&, int, int, const QVariant&)));
		
		Toolbar_->addAction (tr ("Previous"),
				this,
				SLOT (previousHistory ()))->
					setProperty ("ActionIcon", "back");
		Toolbar_->addAction (tr ("Next"),
				this,
				SLOT (nextHistory ()))->
					setProperty ("ActionIcon", "forward");
		Toolbar_->addSeparator ();
		Toolbar_->addAction (tr ("Clear"),
				this,
				SLOT (clearHistory ()))->
					setProperty ("ActionIcon", "remove");

		Core::Instance ()->GetOurAccounts ();
	}
	
	void ChatHistoryWidget::Remove ()
	{
		emit removeSelf (this);
	}
	
	QToolBar* ChatHistoryWidget::GetToolBar () const
	{
		return Toolbar_;
	}
	
	void ChatHistoryWidget::NewTabRequested ()
	{
		S_ParentMultiTabs_->newTabRequested ();
	}
	
	QObject* ChatHistoryWidget::ParentMultiTabs () const
	{
		return S_ParentMultiTabs_;
	}
	
	QList<QAction*> ChatHistoryWidget::GetTabBarContextMenuActions () const
	{
		return QList<QAction*> ();
	}
	
	void ChatHistoryWidget::handleGotOurAccounts (const QStringList& accounts)
	{
		IProxyObject *proxy = Core::Instance ()->GetPluginProxy ();
		Q_FOREACH (const QString& accountID, accounts)
		{
			IAccount *account = qobject_cast<IAccount*> (proxy->GetAccount (accountID));
			if (!account)
			{
				qWarning () << Q_FUNC_INFO
						<< "got invalid IAccount for"
						<< accountID;
				continue;
			}
			Ui_.AccountBox_->addItem (account->GetAccountName (), accountID);
		}

		disconnect (Core::Instance ().get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				this,
				SLOT (handleGotOurAccounts (const QStringList&)));

		if (EntryToFocus_)
		{
			IAccount *entryAcc = qobject_cast<IAccount*> (EntryToFocus_->GetParentAccount ());
			if (!entryAcc)
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid entry account for entry"
						<< EntryToFocus_->GetObject ();
				return;
			}
			
			const QString& id = entryAcc->GetAccountID ();
			for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
				if (id == Ui_.AccountBox_->itemData (i).toString ())
				{
					Ui_.AccountBox_->setCurrentIndex (i);
					on_AccountBox__currentIndexChanged (i);
					break;
				}
		}
	}
	
	void ChatHistoryWidget::handleGotUsersForAccount (const QStringList& users, const QString& id)
	{
		if (id != Ui_.AccountBox_->itemData (Ui_.AccountBox_->currentIndex ()).toString ())
			return;

		IProxyObject *proxy = Core::Instance ()->GetPluginProxy ();
		ContactsModel_->clear ();
		
		QStandardItem *ourFocus = 0;
		const QString& focusId = EntryToFocus_ ?
				EntryToFocus_->GetEntryID () :
				0;
		Q_FOREACH (const QString& user, users)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (proxy->GetEntry (user, id));
			const QString& name = entry ?
					entry->GetEntryName () :
					user;

			QStandardItem *item = new QStandardItem (name);
			item->setData (user, MRIDRole);
			ContactsModel_->appendRow (item);
			
			if (!ourFocus && user == focusId)
				ourFocus = item;
		}
		
		if (ourFocus)
		{
			const QModelIndex& idx = ContactsModel_->indexFromItem (ourFocus);
			Ui_.Contacts_->selectionModel ()->
					setCurrentIndex (idx, QItemSelectionModel::SelectCurrent);
		}
	}
	
	void ChatHistoryWidget::handleGotChatLogs (const QString& accountId,
			const QString& entryId, int, int, const QVariant& logsVar)
	{
		const QString& selectedEntry = Ui_.Contacts_->selectionModel ()->
				currentIndex ().data (MRIDRole).toString ();
		if (accountId != Ui_.AccountBox_->
					itemData (Ui_.AccountBox_->currentIndex ()).toString () ||
				entryId != selectedEntry)
			return;
		
		HistoryViewModel_->clear ();
		HistoryViewModel_->setHorizontalHeaderLabels (QStringList (tr ("Date"))
					<< tr ("Name")
					<< tr ("Message"));
		
		ICLEntry *entry = qobject_cast<ICLEntry*> (Core::Instance ()->
					GetPluginProxy ()->GetEntry (entryId, accountId));
		const QString& name = entry ?
				entry->GetEntryName () :
				entryId;
				
		QList<QColor> colors = Core::Instance ()->
				GetPluginProxy ()->GenerateColors ("hash");
		
		Q_FOREACH (const QVariant& logVar, logsVar.toList ())
		{
			const QVariantMap& map = logVar.toMap ();
			
			const bool isChat = map ["Type"] == "CHAT";
			
			QList<QStandardItem*> items;
			items << new QStandardItem (map ["Date"].toDateTime ().toString ());
			const QString& var = map ["Variant"].toString ();
			if (isChat)
				items << new QStandardItem (var.isEmpty () ?
							name :
							name + '/' + var);
			else
				items << new QStandardItem (var);
			items << new QStandardItem (map ["Message"].toString ());

			if (isChat)
			{
				const QBrush& brush = map ["Direction"] == "IN" ?
						QBrush (Qt::blue) :
						QBrush (Qt::red);
				Q_FOREACH (QStandardItem *item, items)
					item->setForeground (brush);
			}
			else
			{
				const QString& color = Core::Instance ()->
						GetPluginProxy ()->GetNickColor (var, colors);
				items [1]->setForeground (QColor (color));
			}

			Q_FOREACH (QStandardItem *item, items)
				item->setEditable (false);
				
			HistoryViewModel_->appendRow (items);
		}
	}

	void ChatHistoryWidget::on_AccountBox__currentIndexChanged (int idx)
	{
		const QString& id = Ui_.AccountBox_->itemData (idx).toString ();
		Core::Instance ()->GetUsersForAccount (id);
	}
	
	void ChatHistoryWidget::handleContactSelected (const QModelIndex& index)
	{
		CurrentAccount_ = Ui_.AccountBox_->
				itemData (Ui_.AccountBox_->currentIndex ()).toString ();
		CurrentEntry_ = index.data (MRIDRole).toString ();
		Backpages_ = 0;

		RequestLogs ();
	}
	
	void ChatHistoryWidget::previousHistory ()
	{
		if (HistoryViewModel_->rowCount () < Amount)
			return;
		
		++Backpages_;
		RequestLogs ();
	}
	
	void ChatHistoryWidget::nextHistory ()
	{
		if (Backpages_ <= 0)
			return;
		
		--Backpages_;
		RequestLogs ();
	}
	
	void ChatHistoryWidget::clearHistory ()
	{
		if (CurrentAccount_.isEmpty () ||
				CurrentEntry_.isEmpty ())
			return;
		if (QMessageBox::question (0, "LeechCraft",
					tr ("Are you sure you wish to delete chat history with %1?")
						.arg (CurrentEntry_),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;
		
		Core::Instance ()->ClearHistory (CurrentAccount_, CurrentEntry_);
		
		Backpages_ = 0;
		RequestLogs ();
	}
	
	void ChatHistoryWidget::RequestLogs ()
	{
		Core::Instance ()->GetChatLogs (CurrentAccount_,
				CurrentEntry_, Backpages_, Amount);
	}
}
}
}
