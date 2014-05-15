/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "chathistorywidget.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QShortcut>
#include <util/xpc/util.h>
#include <util/gui/clearlineeditaddon.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include "chathistory.h"
#include "xmlsettingsmanager.h"
#include "core.h"

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

	ChatHistoryWidget::ChatHistoryWidget (ICLEntry *entry, QWidget *parent)
	: QWidget (parent)
	, PerPageAmount_ (XmlSettingsManager::Instance ().property ("ItemsPerPage").toInt ())
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
		Ui_.VertSplitter_->setStretchFactor (0, 0);
		Ui_.VertSplitter_->setStretchFactor (1, 4);

		FindBox_ = new ChatFindBox (Core::Instance ()->GetCoreProxy (), Ui_.HistView_);
		connect (FindBox_,
				SIGNAL (next (QString, ChatFindBox::FindFlags)),
				this,
				SLOT (handleNext (QString, ChatFindBox::FindFlags)));
		FindBox_->SetEscCloses (false);

		new QShortcut (QString ("F3"), FindBox_, SLOT (findNext ()));
		new QShortcut (QString ("Shift+F3"), FindBox_, SLOT (findPrevious ()));

		auto proxy = Core::Instance ()->GetCoreProxy ();
		new Util::ClearLineEditAddon (proxy, Ui_.ContactsSearch_);

		SortFilter_->setDynamicSortFilter (true);
		SortFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);
		SortFilter_->setSourceModel (ContactsModel_);
		SortFilter_->sort (0);
		Ui_.Contacts_->setModel (SortFilter_);

		ShowLoading ();

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
		connect (Core::Instance ().get (),
				SIGNAL (gotDaysForSheet (QString, QString, int, int, QList<int>)),
				this,
				SLOT (handleGotDaysForSheet (QString, QString, int, int, QList<int>)));

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
						<< EntryToFocus_->GetQObject ();
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

		Ui_.HistView_->clear ();

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
			item->setToolTip (name);
			item->setEditable (false);
			ContactsModel_->appendRow (item);

			if (!ourFocus && user == focusId)
				ourFocus = item;
		}

		if (ourFocus)
		{
			ShowLoading ();
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

		const auto& bgColor = palette ().color (QPalette::Base);
		const auto& colors = Core::Instance ()->
				GetPluginProxy ()->GenerateColors ("hash", bgColor);

		int scrollPos = -1;

		for (const auto& logVar : logsVar.toList ())
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

			auto msgText = map ["Message"].toString ();
			msgText.replace ('<', "&lt;");
			Core::Instance ()->GetPluginProxy ()->FormatLinks (msgText);
			msgText.replace ('\n', "<br/>");
			html += postNick + ' ' + msgText;

			const bool isSearchRes = SearchResultPosition_ == PerPageAmount_ - Amount_;
			if (isChat && !isSearchRes)
			{
				const auto& color = Core::Instance ()->
						GetPluginProxy ()->GetNickColor (map ["Direction"].toString (), colors);
				html.prepend ("<font color=\"" + color + "\">");
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
		if (accountId != CurrentAccount_ ||
				entryId != CurrentEntry_)
			return;

		if (!position)
		{
			if (!(FindBox_->GetFlags () & ChatFindBox::FindWrapsAround))
				QMessageBox::warning (this,
						"LeechCraft",
						tr ("No more search results for %1.")
							.arg ("<em>" + PreviousSearchText_ + "</em>"));
			else
			{
				SearchShift_ = 0;

				const auto& e = Util::MakeNotification ("Azoth ChatHistory",
						tr ("No more search results for %1, searching from the beginning now.")
							.arg ("<em>" + PreviousSearchText_ + "</em>"),
						PInfo_);
				Core::Instance ()->GetCoreProxy ()->GetEntityManager ()->HandleEntity (e);

				RequestSearch (FindBox_->GetFlags ());
			}
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

		Backpages_ = position / PerPageAmount_;
		SearchResultPosition_ = position % PerPageAmount_;
		RequestLogs ();
	}

	void ChatHistoryWidget::handleGotDaysForSheet (const QString& accountId,
			const QString& entryId, int year, int month, const QList<int>& days)
	{
		if (accountId != CurrentAccount_ ||
			entryId != CurrentEntry_ ||
			year != Ui_.Calendar_->yearShown () ||
			month != Ui_.Calendar_->monthShown ())
			return;

		Ui_.Calendar_->setDateTextFormat (QDate (), QTextCharFormat ());

		QTextCharFormat fmt;
		fmt.setFontWeight (QFont::Bold);
		Q_FOREACH (int day, days)
			Ui_.Calendar_->setDateTextFormat (QDate (year, month, day), fmt);
	}

	void ChatHistoryWidget::on_AccountBox__currentIndexChanged (int idx)
	{
		const QString& id = Ui_.AccountBox_->itemData (idx).toString ();
		Core::Instance ()->GetUsersForAccount (id);
		CurrentEntry_.clear ();
		UpdateDates ();
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

		ShowLoading ();

		RequestLogs ();
		UpdateDates ();
	}

	void ChatHistoryWidget::on_Calendar__currentPageChanged ()
	{
		UpdateDates ();
	}

	void ChatHistoryWidget::on_Calendar__activated (const QDate& date)
	{
		if (CurrentEntry_.isEmpty ())
			return;

		ShowLoading ();

		PreviousSearchText_.clear ();
		FindBox_->clear ();
		Core::Instance ()->Search (CurrentAccount_, CurrentEntry_, QDateTime (date));
	}

	void ChatHistoryWidget::handleNext (const QString& text, ChatFindBox::FindFlags flags)
	{
		ShowLoading ();

		if (text.isEmpty ())
		{
			PreviousSearchText_.clear ();
			Backpages_ = 0;
			SearchResultPosition_ = -1;
			RequestLogs ();
			return;
		}

		if (text != PreviousSearchText_)
		{
			SearchShift_ = 0;
			PreviousSearchText_ = text;
		}
		else if (!(flags & ChatFindBox::FindBackwards))
			++SearchShift_;
		else
			SearchShift_ = std::max (SearchShift_ - 1, 0);

		RequestSearch (flags);
	}

	void ChatHistoryWidget::previousHistory ()
	{
		if (Amount_ < PerPageAmount_)
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
						.arg (EntryID2NameCache_.value (CurrentEntry_, CurrentEntry_)),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Core::Instance ()->ClearHistory (CurrentAccount_, CurrentEntry_);

		Ui_.Contacts_->clearSelection ();
		if (const auto item = FindContactItem (CurrentEntry_))
		{
			CurrentEntry_.clear ();
			ContactsModel_->removeRow (item->row ());
		}

		Backpages_ = 0;
		RequestLogs ();
	}

	void ChatHistoryWidget::on_HistView__anchorClicked (const QUrl& url)
	{
		emit gotEntity (Util::MakeEntity (url,
				QString (),
				FromUserInitiated | OnlyHandle));
	}

	QStandardItem* ChatHistoryWidget::FindContactItem (const QString& id) const
	{
		for (auto i = 0; i < ContactsModel_->rowCount (); ++i)
		{
			const auto item = ContactsModel_->item (i);
			if (item->data (MRIDRole).toString () == id)
				return item;
		}

		return nullptr;
	}

	void ChatHistoryWidget::ShowLoading ()
	{
		const auto& html = "<html><head/><body><span style='color:#666666'>" +
				tr ("History is loading...") +
				"</span></body></html>";
		Ui_.HistView_->setHtml (html);
	}

	void ChatHistoryWidget::UpdateDates ()
	{
		Ui_.Calendar_->setDateTextFormat (QDate (), QTextCharFormat ());

		if (CurrentEntry_.isEmpty ())
			return;

		Core::Instance ()->GetDaysForSheet (CurrentAccount_, CurrentEntry_,
				Ui_.Calendar_->yearShown (), Ui_.Calendar_->monthShown ());
	}

	void ChatHistoryWidget::RequestLogs ()
	{
		Core::Instance ()->GetChatLogs (CurrentAccount_,
				CurrentEntry_, Backpages_, PerPageAmount_);
	}

	void ChatHistoryWidget::RequestSearch (ChatFindBox::FindFlags flags)
	{
		Core::Instance ()->Search (CurrentAccount_, CurrentEntry_,
				PreviousSearchText_, SearchShift_,
				flags & ChatFindBox::FindCaseSensitively);
	}
}
}
}
