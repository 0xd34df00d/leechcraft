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
	, ContactsModel_ (new QStandardItemModel (this))
	, SortFilter_ (new QSortFilterProxyModel (this))
	{
		Ui_.setupUi (this);
		SortFilter_->setDynamicSortFilter (true);
		SortFilter_->setSourceModel (ContactsModel_);
		SortFilter_->sort (0);
		Ui_.Contacts_->setModel (SortFilter_);
		
		connect (Ui_.ContactsSearch_,
				SIGNAL (textChanged (const QString&)),
				SortFilter_,
				SLOT (setFilterFixedString (const QString&)));
		
		connect (Core::Instance ().get (),
				SIGNAL (gotUsersForAccount (const QStringList&, const QString&)),
				this,
				SLOT (handleGotUsersForAccount (const QStringList&, const QString&)));
		connect (Core::Instance ().get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				this,
				SLOT (handleGotOurAccounts (const QStringList&)));
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
		Q_FOREACH (const QString& accountID, accounts)
			Ui_.AccountBox_->addItem (accountID, accountID);

		disconnect (Core::Instance ().get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				this,
				SLOT (handleGotOurAccounts (const QStringList&)));
	}
	
	void ChatHistoryWidget::handleGotUsersForAccount (const QStringList& users, const QString& id)
	{
		if (id != Ui_.AccountBox_->itemData (Ui_.AccountBox_->currentIndex ()).toString ())
			return;

		ContactsModel_->clear ();
		Q_FOREACH (const QString& user, users)
			ContactsModel_->appendRow (new QStandardItem (user));
	}
	
	void ChatHistoryWidget::on_AccountBox__currentIndexChanged (int idx)
	{
		const QString& id = Ui_.AccountBox_->itemData (idx).toString ();
		Core::Instance ()->GetUsersForAccount (id);
	}
}
}
}
