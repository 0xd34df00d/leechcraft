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

#include "chathistorywidget.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
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
	, ContactsModel_ (new QStandardItemModel (this))
	, SortFilter_ (new QSortFilterProxyModel (this))
	, Backpages_ (0)
	, Amount_ (0)
	, SearchShift_ (0)
	, SearchResultPosition_ (-1)
	, ContactSelectedAsGlobSearch_ (false)
	, Toolbar_ (new QToolBar (tr ("Chat history")))
	, EntryToFocus_ (entry)
	{
		Ui_.setupUi (this);

		SortFilter_->setDynamicSortFilter (true);
		SortFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);
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
				SIGNAL (gotUsersForAccount (const QStringList&, const QString&, const QStringList&)),
				this,
				SLOT (handleGotUsersForAccount (const QStringList&, const QString&, const QStringList&)));
		connect (Core::Instance ().get (),
				SIGNAL (gotOurAccounts (const QStringList&)),
				this,
				SLOT (handleGotOurAccounts (const QStringList&)));
		connect (Core::Instance ().get (),
				SIGNAL (gotChatLogs (const QString&, const QString&, int, int, const QVariant&)),
				this,
				SLOT (handleGotChatLogs (const QString&, const QString&, int, int, const QVariant&)));
		connect (Core::Instance ().get (),
				SIGNAL (gotSearchPosition (const QString&, const QString&, int)),
				this,
				SLOT (handleGotSearchPosition (const QString&, const QString&, int)));

		Toolbar_->addAction (tr ("Previous"),
				this,
				SLOT (previousHistory ()))->
					setProperty ("ActionIcon", "go-previous");
		Toolbar_->addAction (tr ("Next"),
				this,
				SLOT (nextHistory ()))->
					setProperty ("ActionIcon", "go-next");
		Toolbar_->addSeparator ();
		Toolbar_->addAction (tr ("Clear"),
				this,
				SLOT (clearHistory ()))->
					setProperty ("ActionIcon", "list-remove");

		Core::Instance ()->GetOurAccounts ();
	}

	void ChatHistoryWidget::Remove ()
	{
		emit removeSelf (this);
		deleteLater ();
	}

	QToolBar* ChatHistoryWidget::GetToolBar () const
	{
		return Toolbar_;
	}

	TabClassInfo ChatHistoryWidget::GetTabClassInfo () const
	{
		return Core::Instance ()->GetTabClass ();
	}

	QObject* ChatHistoryWidget::ParentMultiTabs ()
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
			if (CurrentAccount_.isEmpty ())
				CurrentAccount_ = accountID;
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

	void ChatHistoryWidget::handleGotUsersForAccount (const QStringList& users,
			const QString& id, const QStringList& nameCache)
	{
		if (id != Ui_.AccountBox_->itemData (Ui_.AccountBox_->currentIndex ()).toString ())
			return;

		IProxyObject *proxy = Core::Instance ()->GetPluginProxy ();
		ContactsModel_->clear ();

		QStandardItem *ourFocus = 0;
		const QString& focusId = EntryToFocus_ ?
				EntryToFocus_->GetEntryID () :
				CurrentEntry_;
		EntryToFocus_ = 0;
		for (int i = 0; i < users.size (); ++i)
		{
			const QString& user = users.at (i);
			ICLEntry *entry = qobject_cast<ICLEntry*> (proxy->GetEntry (user, id));
			const QString& name = entry ?
					entry->GetEntryName () :
					(nameCache.value (i).isEmpty () ?
						user :
						nameCache.value (i));

			EntryID2NameCache_ [user] = name;

			QStandardItem *item = new QStandardItem (name);
			item->setData (user, MRIDRole);
			ContactsModel_->appendRow (item);

			if (!ourFocus && user == focusId)
				ourFocus = item;
		}

		if (ourFocus)
		{
			QModelIndex idx = ContactsModel_->indexFromItem (ourFocus);
			idx = SortFilter_->mapFromSource (idx);
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

		Amount_ = 0;
		Ui_.HistView_->clear ();

		const auto& defFormat = Ui_.HistView_->currentCharFormat ();

		ICLEntry *entry = qobject_cast<ICLEntry*> (Core::Instance ()->
					GetPluginProxy ()->GetEntry (entryId, accountId));
		const QString& name = entry ?
				entry->GetEntryName () :
				EntryID2NameCache_.value (entryId, entryId);
		const QString& ourName = entry ?
				qobject_cast<IAccount*> (entry->GetParentAccount ())->GetOurNick () :
				QString ();

		QString preNick = Core::Instance ()->GetPluginProxy ()->
				GetSettingsManager ()->property ("PreNickText").toString ();
		QString postNick = Core::Instance ()->GetPluginProxy ()->
				GetSettingsManager ()->property ("PostNickText").toString ();
		preNick.replace ('<', "&lt;");
		postNick.replace ('<', "&lt;");

		QList<QColor> colors = Core::Instance ()->
				GetPluginProxy ()->GenerateColors ("hash");

		int scrollPos = -1;

		Q_FOREACH (const QVariant& logVar, logsVar.toList ())
		{
			const QVariantMap& map = logVar.toMap ();

			const bool isChat = map ["Type"] == "CHAT";

			QString html = "[" + map ["Date"].toDateTime ().toString () + "] " + preNick;
			const QString& var = map ["Variant"].toString ();
			if (isChat)
			{
				QString remoteName;
				if (!entry && !var.isEmpty ())
					remoteName += var;
				else if (entry && var.isEmpty ())
					remoteName += name;
				else
					remoteName += name + '/' + var;

				if (!ourName.isEmpty ())
					html += map ["Direction"] == "IN" ?
							remoteName :
							ourName;
				else
				{
					html += map ["Direction"] == "IN" ?
							QString::fromUtf8 ("← ") :
							QString::fromUtf8 ("→ ");
					html += remoteName;
				}
			}
			else
			{
				const QString& color = Core::Instance ()->
						GetPluginProxy ()->GetNickColor (var, colors);
				html += "<font color=\"" + color + "\">" + var + "</font>";
			}

			html += postNick + ' ' + map ["Message"].toString ()
					.replace ('<', "&lt;")
					.replace ('\n', "<br/>");

			const bool isSearchRes = SearchResultPosition_ == Amount - Amount_;
			if (isChat && !isSearchRes)
			{
				html.prepend (QString ("<font color=\"#") +
						(map ["Direction"] == "IN" ? "0000dd" : "dd0000") +
						"\">");
				html += "</font>";
			}
			else if (isSearchRes)
			{
				QTextCharFormat fmt = defFormat;
				scrollPos = Ui_.HistView_->document ()->characterCount ();

				html.prepend ("<font color='#FF7E00'>");
				html += "</font>";
			}
			++Amount_;

			Ui_.HistView_->append (html);

			if (isSearchRes)
				Ui_.HistView_->setCurrentCharFormat (defFormat);
		}

		if (scrollPos >= 0)
		{
			QTextCursor cur (Ui_.HistView_->document ());
			cur.setPosition (scrollPos);
			Ui_.HistView_->setTextCursor (cur);
			Ui_.HistView_->ensureCursorVisible ();
		}
	}

	void ChatHistoryWidget::handleGotSearchPosition (const QString& accountId,
			const QString& entryId, int position)
	{
		const bool wideSearch = Ui_.SearchType_->currentIndex ();
		if (!wideSearch)
		{
			if (accountId != CurrentAccount_ ||
					entryId != CurrentEntry_)
				return;
		}

		if (!position)
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("No more search results for %1.")
						.arg (PreviousSearchText_));
			return;
		}

		if (CurrentEntry_ != entryId)
		{
			ContactSelectedAsGlobSearch_ = true;
			CurrentEntry_ = entryId;
			if (CurrentAccount_ == accountId)
				for (int i = 0; i < ContactsModel_->rowCount (); ++i)
				{
					auto item = ContactsModel_->item (i);
					if (item->data (MRIDRole) == CurrentEntry_)
					{
						Ui_.Contacts_->setCurrentIndex (item->index ());
						break;
					}
				}
		}
		if (CurrentAccount_ != accountId)
		{
			ContactSelectedAsGlobSearch_ = true;
			CurrentAccount_ = accountId;
			for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
				if (accountId == Ui_.AccountBox_->itemData (i).toString ())
				{
					Ui_.AccountBox_->setCurrentIndex (i);
					CurrentEntry_ = entryId;
					break;
				}
		}

		Backpages_ = position / Amount;
		SearchResultPosition_ = position % Amount;
		RequestLogs ();
	}

	void ChatHistoryWidget::on_AccountBox__currentIndexChanged (int idx)
	{
		const QString& id = Ui_.AccountBox_->itemData (idx).toString ();
		Core::Instance ()->GetUsersForAccount (id);
		CurrentEntry_.clear ();
	}

	void ChatHistoryWidget::handleContactSelected (const QModelIndex& index)
	{
		CurrentAccount_ = Ui_.AccountBox_->
				itemData (Ui_.AccountBox_->currentIndex ()).toString ();
		CurrentEntry_ = index.data (MRIDRole).toString ();
		if (!ContactSelectedAsGlobSearch_)
		{
			SearchShift_ = 0;
			PreviousSearchText_.clear ();
			Backpages_ = 0;
			SearchResultPosition_ = -1;
		}
		ContactSelectedAsGlobSearch_ = false;

		RequestLogs ();
	}

	void ChatHistoryWidget::on_HistorySearch__returnPressed ()
	{
		const QString& text = Ui_.HistorySearch_->text ();
		if (text.isEmpty ())
		{
			PreviousSearchText_.clear ();
			Backpages_ = 0;
			SearchResultPosition_ = -1;
			RequestLogs ();
			return;
		}

		if (text == PreviousSearchText_)
			++SearchShift_;
		else
		{
			SearchShift_ = 0;
			PreviousSearchText_ = text;
		}

		RequestSearch ();
	}

	void ChatHistoryWidget::on_SearchType__currentIndexChanged ()
	{
		if (!Ui_.HistorySearch_->text ().isEmpty ())
		{
			SearchShift_ = 0;
			PreviousSearchText_.clear ();
			on_HistorySearch__returnPressed ();
		}
	}

	void ChatHistoryWidget::previousHistory ()
	{
		if (Amount_ < Amount)
			return;

		++Backpages_;
		SearchResultPosition_ = -1;
		RequestLogs ();
	}

	void ChatHistoryWidget::nextHistory ()
	{
		if (Backpages_ <= 0)
			return;

		--Backpages_;
		SearchResultPosition_ = -1;
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

	void ChatHistoryWidget::RequestSearch ()
	{
		const QString& entryStr = Ui_.SearchType_->currentIndex () > 0 ?
				QString () :
				CurrentEntry_;
		const QString& accStr = Ui_.SearchType_->currentIndex () > 1 ?
				QString () :
				CurrentAccount_;
		Core::Instance ()->Search (accStr, entryStr, PreviousSearchText_, SearchShift_);
	}
}
}
}
