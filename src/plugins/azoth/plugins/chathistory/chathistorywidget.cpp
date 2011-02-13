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
#include <QWebFrame>
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

	void ChatHistoryWidget::SetParentMultiTabs (Plugin *ch)
	{
		S_ParentMultiTabs_ = ch;
	}

	ChatHistoryWidget::ChatHistoryWidget (QWidget *parent)
	: QWidget (parent)
	, HistoryViewModel_ (new QStandardItemModel (this))
	, ContactsModel_ (new QStandardItemModel (this))
	, SortFilter_ (new QSortFilterProxyModel (this))
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
		Core::Instance ()->GetOurAccounts ();
	}
	
	void ChatHistoryWidget::Remove ()
	{
		emit removeSelf (this);
	}
	
	QToolBar* ChatHistoryWidget::GetToolBar () const
	{
		return 0;
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
	}
	
	void ChatHistoryWidget::handleGotUsersForAccount (const QStringList& users, const QString& id)
	{
		if (id != Ui_.AccountBox_->itemData (Ui_.AccountBox_->currentIndex ()).toString ())
			return;

		IProxyObject *proxy = Core::Instance ()->GetPluginProxy ();
		ContactsModel_->clear ();
		Q_FOREACH (const QString& user, users)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (proxy->GetEntry (user, id));
			const QString& name = entry ?
					entry->GetEntryName () :
					user;

			QStandardItem *item = new QStandardItem (name);
			item->setData (user, MRIDRole);
			ContactsModel_->appendRow (item);
		}
	}
	
	void ChatHistoryWidget::handleGotChatLogs (const QString& accountId,
			const QString& entryId, int backpages, int amount, const QVariant& logsVar)
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
		
		Ui_.HistView_->resizeColumnsToContents ();
	}

	void ChatHistoryWidget::on_AccountBox__currentIndexChanged (int idx)
	{
		const QString& id = Ui_.AccountBox_->itemData (idx).toString ();
		Core::Instance ()->GetUsersForAccount (id);
	}
	
	void ChatHistoryWidget::handleContactSelected (const QModelIndex& index)
	{
		const QString& accountId = Ui_.AccountBox_->
				itemData (Ui_.AccountBox_->currentIndex ()).toString ();
		const QString& entryId = index.data (MRIDRole).toString ();
		
		Core::Instance ()->GetChatLogs (accountId, entryId, 0, 50);
	}
}
}
}
