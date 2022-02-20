/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chathistorywidget.h"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QShortcut>
#include <QToolBar>
#include <util/xpc/util.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/threads/futures.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include "chathistory.h"
#include "xmlsettingsmanager.h"
#include "historyvieweventfilter.h"
#include "storagemanager.h"

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	using namespace std::placeholders;

	ChatHistoryWidget::ChatHistoryWidget (const InitParams& params, ICLEntry *entry, QWidget *parent)
	: QWidget (parent)
	, Params_ (params)
	, PerPageAmount_ (XmlSettingsManager::Instance ().property ("ItemsPerPage").toInt ())
	, ContactsModel_ (new QStandardItemModel (this))
	, SortFilter_ (new QSortFilterProxyModel (this))
	, Toolbar_ (new QToolBar (tr ("Chat history")))
	, EntryToFocus_ (entry)
	{
		Ui_.setupUi (this);
		Ui_.VertSplitter_->setStretchFactor (0, 0);
		Ui_.VertSplitter_->setStretchFactor (1, 4);

		FindBox_ = new ChatFindBox (Params_.CoreProxy_, Ui_.HistView_);
		connect (FindBox_,
				SIGNAL (next (QString, ChatFindBox::FindFlags)),
				this,
				SLOT (handleNext (QString, ChatFindBox::FindFlags)));
		FindBox_->SetEscCloses (false);

		new Util::ClearLineEditAddon (Params_.CoreProxy_, Ui_.ContactsSearch_);

		const auto hvef = new HistoryViewEventFilter (Ui_.HistView_);
		connect (hvef,
				SIGNAL (bgLinkRequested (QUrl)),
				this,
				SLOT (handleBgLinkRequested (QUrl)));

		SortFilter_->setDynamicSortFilter (true);
		SortFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);
		SortFilter_->setFilterCaseSensitivity (Qt::CaseInsensitive);
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

		Toolbar_->addAction (tr ("Previous"),
				this,
				SLOT (previousHistory ()))->setProperty ("ActionIcon", "go-previous");
		Toolbar_->addAction (tr ("Next"),
				this,
				SLOT (nextHistory ()))->setProperty ("ActionIcon", "go-next");
		Toolbar_->addSeparator ();
		Toolbar_->addAction (tr ("Clear"),
				this,
				SLOT (clearHistory ()))->setProperty ("ActionIcon", "list-remove");

		Util::Sequence (this, Params_.StorageMgr_->GetOurAccounts ()) >>
				[this] (const QStringList& accs) { HandleGotOurAccounts (accs); };
	}

	void ChatHistoryWidget::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* ChatHistoryWidget::GetToolBar () const
	{
		return Toolbar_;
	}

	TabClassInfo ChatHistoryWidget::GetTabClassInfo () const
	{
		return Params_.Info_;
	}

	QObject* ChatHistoryWidget::ParentMultiTabs ()
	{
		return Params_.ParentMultiTabs_;
	}

	QList<QAction*> ChatHistoryWidget::GetTabBarContextMenuActions () const
	{
		return {};
	}

	void ChatHistoryWidget::HandleGotOurAccounts (const QStringList& accounts)
	{
		for (const auto& accountID : accounts)
		{
			const auto account = qobject_cast<IAccount*> (Params_.PluginProxy_->GetAccount (accountID));
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

		if (EntryToFocus_)
		{
			const auto entryAcc = EntryToFocus_->GetParentAccount ();
			if (!entryAcc)
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid entry account for entry"
						<< EntryToFocus_->GetQObject ();
				return;
			}

			const auto& id = entryAcc->GetAccountID ();
			for (int i = 0; i < Ui_.AccountBox_->count (); ++i)
				if (id == Ui_.AccountBox_->itemData (i).toString ())
				{
					Ui_.AccountBox_->setCurrentIndex (i);
					on_AccountBox__currentIndexChanged (i);
					break;
				}
		}
	}

	namespace
	{
		QString GetEntryName (const QString& entryId, const QString& accountId, const QString& cachedName, IProxyObject *proxy)
		{
			if (const auto entry = qobject_cast<ICLEntry*> (proxy->GetEntry (entryId, accountId)))
			{
				const auto& entryName = entry->GetEntryName ();

				if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
				{
					const auto parent = entry->GetParentCLEntry ();
					return parent->GetEntryName () + '/' + entryName;
				}
				else
					return entryName;
			}

			if (!cachedName.isEmpty ())
				return cachedName;

			return entryId;
		}
	}

	void ChatHistoryWidget::HandleGotUsersForAccount (const QString& id,
			const UsersForAccountResult_t& result)
	{
		if (const auto left = result.MaybeLeft ())
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Unable to get the list of users in the account.") + " " + *left);
			return;
		}

		const auto& right = result.GetRight ();
		const auto& nameCache = right.NameCache_;
		const auto& users = right.Users_;

		ContactsModel_->clear ();

		Ui_.HistView_->clear ();

		QStandardItem *ourFocus = nullptr;
		const auto& focusId = EntryToFocus_ ?
				EntryToFocus_->GetEntryID () :
				CurrentEntry_;
		for (int i = 0; i < users.size (); ++i)
		{
			const auto& user = users.at (i);
			const auto& name = GetEntryName (user, id, nameCache.value (i), Params_.PluginProxy_);

			EntryID2NameCache_ [user] = name;

			const auto item = new QStandardItem (name);
			item->setData (user, MRIDRole);
			item->setToolTip (name);
			item->setEditable (false);
			ContactsModel_->appendRow (item);

			if (!ourFocus && user == focusId)
				ourFocus = item;
		}

		if (ourFocus)
		{
			EntryToFocus_ = nullptr;
			ShowLoading ();
			auto idx = ContactsModel_->indexFromItem (ourFocus);
			idx = SortFilter_->mapFromSource (idx);
			Ui_.Contacts_->selectionModel ()->
					setCurrentIndex (idx, QItemSelectionModel::SelectCurrent);
		}
	}

	void ChatHistoryWidget::HandleGotChatLogs (const QString& accountId,
			const QString& entryId, const ChatLogsResult_t& result)
	{
		const auto& selEntry = Ui_.Contacts_->selectionModel ()->
				currentIndex ().data (MRIDRole).toString ();
		const auto& selAcc = Ui_.AccountBox_->itemData (Ui_.AccountBox_->currentIndex ()).toString ();
		if (accountId != selAcc ||
				entryId != selEntry)
			return;

		Amount_ = 0;
		Ui_.HistView_->clear ();

		auto& formatter = Params_.PluginProxy_->GetFormatterProxy ();

		const auto entry = qobject_cast<ICLEntry*> (Params_.PluginProxy_->GetEntry (entryId, accountId));
		const auto& name = entry ?
				entry->GetEntryName () :
				EntryID2NameCache_.value (entryId, entryId);

		if (const auto err = result.MaybeLeft ())
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Error getting logs with %1.")
						.arg (name) +
					" " + *err);
			return;
		}

		const auto& ourName = entry ?
				entry->GetParentAccount ()->GetOurNick () :
				QString ();

		auto preNick = Params_.PluginProxy_->GetSettingsManager ()->property ("PreNickText").toString ();
		auto postNick = Params_.PluginProxy_->GetSettingsManager ()->property ("PostNickText").toString ();
		preNick.replace ('<', "&lt;");
		postNick.replace ('<', "&lt;");

		const auto& bgColor = palette ().color (QPalette::Base);
		const auto& colors = formatter.GenerateColors ("hash", bgColor);

		int scrollPos = -1;

		for (const auto& logItem : result.GetRight ())
		{
			const bool isChat = logItem.Type_ == IMessage::Type::ChatMessage;
			const bool isIncoming = logItem.Dir_ == IMessage::Direction::In;

			QString remoteName;

			auto html = "[" + logItem.Date_.toString () + "] " + preNick;
			const auto& var = logItem.Variant_;
			if (isChat)
			{
				if (!name.isEmpty () && var.isEmpty ())
					remoteName += name;
				else if (name.isEmpty () && !var.isEmpty ())
					remoteName += var;
				else if (!name.endsWith ('/' + var))
					remoteName += name + '/' + var;
				else
					remoteName += name;

				if (!ourName.isEmpty ())
					html += isIncoming ?
							remoteName :
							ourName;
				else
				{
					html += isIncoming ?
							QString::fromUtf8 ("← ") :
							QString::fromUtf8 ("→ ");
					html += remoteName;
				}
			}
			else
			{
				const auto& color = formatter.GetNickColor (var, colors);
				html += "<font color=\"" + color + "\">" + var + "</font>";
			}

			auto msgText = logItem.RichMessage_;
			if (msgText.isEmpty ())
			{
				const bool escape = logItem.EscPolicy_ == IMessage::EscapePolicy::Escape;

				msgText = logItem.Message_;

				if (escape)
					msgText.replace ('<', "&lt;");
				formatter.FormatLinks (msgText);
				if (escape)
					msgText.replace ('\n', "<br/>");
			}

			html += postNick + ' ' + msgText;

			const bool isSearchRes = SearchResultPosition_ == PerPageAmount_ - ++Amount_;
			if (isChat && !isSearchRes)
			{
				const auto& color = formatter.GetNickColor (isIncoming ? remoteName : ourName, colors);
				html.prepend ("<font color=\"" + color + "\">");
				html += "</font>";
			}
			else if (isSearchRes)
			{
				scrollPos = Ui_.HistView_->document ()->characterCount ();

				html.prepend ("<font color='#FF7E00'>");
				html += "</font>";
			}

			Ui_.HistView_->append (html);
		}

		if (scrollPos >= 0)
		{
			QTextCursor cur (Ui_.HistView_->document ());
			cur.setPosition (scrollPos);
			Ui_.HistView_->setTextCursor (cur);
			Ui_.HistView_->ensureCursorVisible ();
		}
	}

	void ChatHistoryWidget::HandleGotSearchPosition (const QString& accountId,
			const QString& entryId, const SearchResult_t& result)
	{
		if (accountId != CurrentAccount_ ||
				entryId != CurrentEntry_)
			return;

		if (const auto err = result.MaybeLeft ())
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Unable to perform the search.") + " " + *err);
			return;
		}

		const auto position = result.GetRight ();

		if (!position)
		{
			if (!(FindBox_->GetFlags () & ChatFindBox::FindWrapsAround) || !SearchShift_)
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
						Priority::Info);
				Params_.CoreProxy_->GetEntityManager ()->HandleEntity (e);

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

		Backpages_ = *position / PerPageAmount_;
		SearchResultPosition_ = *position % PerPageAmount_;
		RequestLogs ();
	}

	void ChatHistoryWidget::HandleGotDaysForSheet (const QString& accountId,
			const QString& entryId, int year, int month, const DaysResult_t& days)
	{
		if (accountId != CurrentAccount_ ||
			entryId != CurrentEntry_ ||
			year != Ui_.Calendar_->yearShown () ||
			month != Ui_.Calendar_->monthShown ())
			return;

		if (days.IsLeft ())
			return;

		Ui_.Calendar_->setDateTextFormat ({}, {});

		QTextCharFormat fmt;
		fmt.setFontWeight (QFont::Bold);
		for (int day : days.GetRight ())
			Ui_.Calendar_->setDateTextFormat ({ year, month, day }, fmt);
	}

	void ChatHistoryWidget::on_AccountBox__currentIndexChanged (int idx)
	{
		const auto& id = Ui_.AccountBox_->itemData (idx).toString ();
		CurrentEntry_.clear ();
		UpdateDates ();

		Util::Sequence (this, Params_.StorageMgr_->GetUsersForAccount (id)) >>
				std::bind (&ChatHistoryWidget::HandleGotUsersForAccount, this, id, _1);
	}

	void ChatHistoryWidget::handleContactSelected (const QModelIndex& index)
	{
		if (!index.isValid ())
		{
			Ui_.HistView_->clear ();
			return;
		}

		CurrentAccount_ = Ui_.AccountBox_->itemData (Ui_.AccountBox_->currentIndex ()).toString ();
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
		FindBox_->Clear ();

		Util::Sequence (this,
				Params_.StorageMgr_->Search (CurrentAccount_, CurrentEntry_, date.startOfDay ())) >>
				std::bind (&ChatHistoryWidget::HandleGotSearchPosition,
						this, CurrentAccount_, CurrentEntry_, _1);
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

		Params_.StorageMgr_->ClearHistory (CurrentAccount_, CurrentEntry_);

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
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeEntity (url,
				{},
				FromUserInitiated | OnlyHandle));
	}

	void ChatHistoryWidget::handleBgLinkRequested (const QUrl& url)
	{
		auto e = Util::MakeEntity (url,
				QString (),
				FromUserInitiated | OnlyHandle);
		e.Additional_ ["BackgroundHandle"] = true;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
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

		const auto year = Ui_.Calendar_->yearShown ();
		const auto month = Ui_.Calendar_->monthShown ();
		const auto& future = Params_.StorageMgr_->GetDaysForSheet (CurrentAccount_, CurrentEntry_,
				year, month);
		Util::Sequence (this, future) >>
				std::bind (&ChatHistoryWidget::HandleGotDaysForSheet,
						this, CurrentAccount_, CurrentEntry_, year, month, _1);
	}

	void ChatHistoryWidget::RequestLogs ()
	{
		const auto& future = Params_.StorageMgr_->GetChatLogs (CurrentAccount_,
				CurrentEntry_, Backpages_, PerPageAmount_);
		Util::Sequence (this, future) >>
				std::bind (&ChatHistoryWidget::HandleGotChatLogs, this, CurrentAccount_, CurrentEntry_, _1);
	}

	void ChatHistoryWidget::RequestSearch (ChatFindBox::FindFlags flags)
	{
		const auto& future = Params_.StorageMgr_->Search (CurrentAccount_, CurrentEntry_,
				PreviousSearchText_, SearchShift_,
				flags & ChatFindBox::FindCaseSensitively);
		Util::Sequence (this, future) >>
				std::bind (&ChatHistoryWidget::HandleGotSearchPosition,
						this, CurrentAccount_, CurrentEntry_, _1);
	}
}
}
}
