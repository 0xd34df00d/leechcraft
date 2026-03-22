/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chathistorywidget.h"
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QToolBar>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include "chathistory.h"
#include "xmlsettingsmanager.h"
#include "historyvieweventfilter.h"

namespace LC::Azoth::ChatHistory
{
	ChatHistoryWidget::ChatHistoryWidget (const InitParams& params, ICLEntry *entry, QWidget *parent)
	: QWidget (parent)
	, Params_ (params)
	, PerPageAmount_ (XmlSettingsManager::Instance ().property ("ItemsPerPage").toInt ())
	, AccountsModel_ { { { Qt::DisplayRole, Util::FromField<&AccountInfo::AccountName_> } } }
	, SortFilter_ (new QSortFilterProxyModel (this))
	, Toolbar_ (new QToolBar (tr ("Chat history")))
	, FocusEntry_ { entry ? std::optional<FocusEntry> { { .AccId_ = entry->GetParentAccount ()->GetAccountID (), .EntryId_ = entry->GetHumanReadableID () } } : std::nullopt }
	{
		Ui_.setupUi (this);
		Ui_.VertSplitter_->setStretchFactor (0, 0);
		Ui_.VertSplitter_->setStretchFactor (1, 4);

		connect (Ui_.Calendar_,
				&QCalendarWidget::currentPageChanged,
				this,
				&ChatHistoryWidget::UpdateDates);
		connect (Ui_.Calendar_,
				&QCalendarWidget::activated,
				this,
				&ChatHistoryWidget::RequestLogsForDate);

		FindBox_ = new ChatFindBox (Ui_.HistView_);
		connect (FindBox_,
				&ChatFindBox::next,
				this,
				&ChatHistoryWidget::HandleSearch);
		FindBox_->SetEscCloses (false);

		const auto hvef = new HistoryViewEventFilter (Ui_.HistView_);
		connect (hvef,
				&HistoryViewEventFilter::bgLinkRequested,
				this,
				[] (const QUrl& url)
				{
					auto e = Util::MakeEntity (url,
							QString (),
							FromUserInitiated | OnlyHandle);
					e.Additional_ ["BackgroundHandle"] = true;
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
				});
		connect (Ui_.HistView_,
				&QTextBrowser::anchorClicked,
				this,
				[] (const QUrl& url)
				{
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeEntity (url,
							{},
							FromUserInitiated | OnlyHandle));
				});

		SortFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);
		SortFilter_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		SortFilter_->setSourceModel (&EntriesModel_);
		SortFilter_->sort (0);
		Ui_.Contacts_->setModel (SortFilter_);

		ShowLoading ();

		connect (Ui_.ContactsSearch_,
				&QLineEdit::textChanged,
				SortFilter_,
				&QSortFilterProxyModel::setFilterFixedString);
		connect (Ui_.Contacts_->selectionModel (),
				&QItemSelectionModel::currentRowChanged,
				this,
				&ChatHistoryWidget::HandleEntrySelected);

		Toolbar_->addAction (tr ("Previous"),
				this,
				[this]
				{
					const auto firstId = DisplayedSpan_
							.transform ([] (auto span) { return span.FirstId_; })
							.value_or (std::numeric_limits<qint64>::max ());
					RequestLogs (Storage2::Pagination { .CursorMessageId_ = firstId, .Before_ = PerPageAmount_ });
				})->setProperty ("ActionIcon", "go-previous");
		Toolbar_->addAction (tr ("Next"),
				this,
				[this]
				{
					const auto lastId = DisplayedSpan_
							.transform ([] (auto span) { return span.LastId_; })
							.value_or (0);
					RequestLogs (Storage2::Pagination { .CursorMessageId_ = lastId, .After_ = PerPageAmount_ });
				})->setProperty ("ActionIcon", "go-next");
		Toolbar_->addSeparator ();
		Toolbar_->addAction (tr ("Clear"),
				this,
				&ChatHistoryWidget::ClearHistory)->setProperty ("ActionIcon", "list-remove");

		Ui_.AccountBox_->setModel (&AccountsModel_);
		connect (Ui_.AccountBox_,
				&QComboBox::currentIndexChanged,
				this,
				&ChatHistoryWidget::LoadAccountEntries);
		LoadAccounts ();
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

	std::optional<AccountInfo> ChatHistoryWidget::GetCurrentAccount () const
	{
		const auto curIdx = Ui_.AccountBox_->currentIndex ();
		if (curIdx < 0)
			return {};
		return AccountsModel_.GetItems () [curIdx];
	}

	std::optional<ChatHistoryWidget::DisplayEntry> ChatHistoryWidget::GetCurrentEntry () const
	{
		const auto& curIdx = SortFilter_->mapToSource (Ui_.Contacts_->selectionModel ()->currentIndex ());
		if (!curIdx.isValid ())
			return {};
		return EntriesModel_.GetItems () [curIdx.row ()];
	}

	Util::ContextTask<void> ChatHistoryWidget::LoadAccounts ()
	{
		co_await Util::AddContextObject { *this };

		const auto& accToFocus = FocusEntry_.transform ([] (const FocusEntry& fe) { return fe.AccId_; }).value_or ({});

		const auto accounts = co_await Params_.StorageThread_.Run (&Storage2::GetAccounts);

		const QSignalBlocker blocker { Ui_.AccountBox_ };
		AccountsModel_.SetItems (accounts);
		if (accounts.isEmpty ())
			co_return;

		const auto focusPos = std::ranges::find (accounts, accToFocus, &AccountInfo::AccountId_);
		const auto focusIndex = focusPos != accounts.end () ? static_cast<int> (focusPos - accounts.begin ()) : 0;
		Ui_.AccountBox_->setCurrentIndex (focusIndex);
		LoadAccountEntries (focusIndex);
	}

	Util::ContextTask<void> ChatHistoryWidget::LoadAccountEntries (int idx)
	{
		Ui_.HistView_->clear ();
		UpdateDates ();
		if (idx < 0 || idx >= AccountsModel_.rowCount ())
			co_return;

		co_await Util::AddContextObject { *this };

		const auto accountId = AccountsModel_.GetItems () [idx].Id_;
		const auto entries = co_await Params_.StorageThread_.Run (&Storage2::GetEntries, accountId);
		if (Ui_.AccountBox_->currentIndex () != idx)
			co_return;

		const auto& focusId = FocusEntry_.transform ([] (const FocusEntry& fe) { return fe.EntryId_; }).value_or ({});
		std::optional<int> focusIndex;

		QList<DisplayEntry> displayEntries;
		displayEntries.reserve (entries.size ());
		for (const auto& entry : entries)
		{
			using History::EntryDescr;
			using enum History::EntryKind;
			const auto& name = Util::Visit (entry.EntryInfo_,
					[] (const EntryDescr<Chat>& roster) { return roster.Nick_; },
					[] (const EntryDescr<MUC>& muc) { return muc.MucName_; },
					[] (const EntryDescr<PrivateChat>& part) { return part.MucName_ + '/' + part.Nick_; });
			displayEntries << DisplayEntry
			{
				.Id_ = entry.Id_,
				.Name_ = name,
				.Base_ = entry,
			};

			if (!focusIndex && focusId == entry.HumanReadableId_)
				focusIndex = displayEntries.size () - 1;
		}
		EntriesModel_.SetItems (std::move (displayEntries));

		if (focusIndex)
		{
			FocusEntry_.reset ();
			const auto& row = SortFilter_->mapFromSource (EntriesModel_.index (*focusIndex, 0));
			Ui_.Contacts_->selectionModel ()->setCurrentIndex (row, QItemSelectionModel::SelectCurrent);
		}
	}

	void ChatHistoryWidget::HandleEntrySelected (const QModelIndex& idx)
	{
		if (!idx.isValid ())
			return;

		DisplayedSpan_.reset ();

		RequestLogs ({ .Before_ = PerPageAmount_ });
		UpdateDates ();
	}

	Util::ContextTask<void> ChatHistoryWidget::RequestLogs (const Storage2::Pagination& pagination)
	{
		const auto entry = GetCurrentEntry ();
		if (!entry)
			co_return;

		ShowLoading ();

		co_await Util::AddContextObject { *this };
		const auto messages = co_await Params_.StorageThread_.Run (&Storage2::GetMessages, entry->Base_, pagination);
		co_await GuardEntryChanged (entry->Id_);
		RenderMessages (messages);
	}

	Util::ContextTask<void> ChatHistoryWidget::RequestLogsForDate (const QDate& date)
	{
		const auto entry = GetCurrentEntry ();
		if (!entry)
			co_return;

		ShowLoading ();
		FindBox_->Clear ();

		co_await Util::AddContextObject { *this };
		const auto& messages = co_await Params_.StorageThread_.Run (&Storage2::GetMessagesDated, entry->Base_, date);
		co_await GuardEntryChanged (entry->Id_);
		RenderMessages (messages);
	}

	Util::ContextTask<void> ChatHistoryWidget::UpdateDates ()
	{
		Ui_.Calendar_->setDateTextFormat ({}, {});

		const auto entry = GetCurrentEntry ();
		if (!entry)
			co_return;

		co_await Util::AddContextObject { *this };

		const auto year = Ui_.Calendar_->yearShown ();
		const auto month = Ui_.Calendar_->monthShown ();
		const auto days = co_await Params_.StorageThread_.Run (&Storage2::GetDaysWithHistory, entry->Base_, year, month);

		co_await GuardEntryChanged (entry->Id_);

		if (year != Ui_.Calendar_->yearShown () || month != Ui_.Calendar_->monthShown ())
			co_return;

		QTextCharFormat fmt;
		fmt.setFontWeight (QFont::Bold);
		for (auto day : days)
			Ui_.Calendar_->setDateTextFormat ({ year, month, day }, fmt);
	}

	Util::ContextTask<void> ChatHistoryWidget::HandleSearch (const QString& text, ChatFindBox::FindFlags flags)
	{
		if (text.isEmpty ())
			co_return;

		const auto entry = GetCurrentEntry ();
		if (!entry)
			co_return;

		co_await Util::AddContextObject { *this };

		if (PreviousSearchText_ != text)
		{
			LastSearchCursor_.reset ();
			PreviousSearchText_ = text;
		}

		using enum Storage2::SearchDirection;
		const auto cs = flags & ChatFindBox::FindCaseSensitively ? Qt::CaseSensitive : Qt::CaseInsensitive;
		const auto dir = flags & ChatFindBox::FindBackwards ? Backward : Forward;

		const auto def = dir == Backward ? std::numeric_limits<qint64>::max () : -1;
		const auto from = LastSearchCursor_.value_or (def);

		auto nextPos = co_await Params_.StorageThread_.Run (&Storage2::Search, entry->Base_, text, cs, dir, from);
		if (!nextPos && flags & ChatFindBox::FindWrapsAround)
		{
			const auto& e = Util::MakeNotification ("Azoth ChatHistory",
					tr ("No more search results, wrapping the search around…"),
					Priority::Info);
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
			nextPos = co_await Params_.StorageThread_.Run (&Storage2::Search, entry->Base_, text, cs, dir, def);
		}

		co_await GuardEntryChanged (entry->Id_);

		if (!nextPos)
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("No more search results for %1.").arg ("<em>" + text.toHtmlEscaped () + "</em>"));
			co_return;
		}

		LastSearchCursor_ = nextPos;

		const uint16_t halfAmount = PerPageAmount_ / 2;
		RequestLogs ({ .CursorMessageId_ = *nextPos, .Before_ = halfAmount, .After_ = halfAmount });
	}

	Util::Either<ChatHistoryWidget::EntryChanged, Util::Void> ChatHistoryWidget::GuardEntryChanged (qint64 entryId) const
	{
		if (const auto currEntry = GetCurrentEntry ();
			!currEntry || currEntry->Id_ != entryId)
			return Util::Left { EntryChanged {} };
		return Util::Void {};
	}

	void ChatHistoryWidget::ShowLoading ()
	{
		const auto& html = "<html><head/><body><span style='color:#666666'>" +
				tr ("History is loading...") +
				"</span></body></html>";
		Ui_.HistView_->setHtml (html);
	}

	void ChatHistoryWidget::RenderMessages (const QList<Storage2::HistoryMessage>& messages)
	{
		const auto& acc = GetCurrentAccount ();
		if (!acc)
			return;

		const auto& entry = GetCurrentEntry ();
		if (!entry)
			qFatal () << "unexpected null entry";

		Ui_.HistView_->clear ();

		if (messages.isEmpty ())
		{
			DisplayedSpan_.reset ();
			return;
		}

		DisplayedSpan_ = DisplayedMessagesSpan { messages.front ().Id_, messages.back ().Id_ };

		const auto azothSettings = Params_.PluginProxy_->GetSettingsManager ();
		const auto preNick = azothSettings->property ("PreNickText").toString ().toHtmlEscaped ();
		const auto postNick = azothSettings->property ("PostNickText").toString ().toHtmlEscaped ();

		auto& formatter = Params_.PluginProxy_->GetFormatterProxy ();

		const auto& bgColor = palette ().color (QPalette::Base);
		const auto& colors = formatter.GenerateColors ("hash", bgColor);

		std::optional<int> scrollPos;

		for (const auto& message : messages)
		{
			std::optional<QString> lineColor;

			auto html = "[" + message.TS_.toString () + "] " + preNick;
			if (entry->IsMuc ())
			{
				const auto& nick = message.DisplayName_;
				const auto& color = formatter.GetNickColor (nick, colors);
				html += "<font color=\"" + color + "\">" + nick.toHtmlEscaped () + "</font>";
			}
			else
			{
				constexpr static auto withVariant = [] (const QString& nick, const QString& variant)
				{
					return variant.isEmpty () ? nick : nick + '/' + variant;
				};
				auto nick = withVariant (message.DisplayName_, message.Variant_.value_or ({}));
				html += nick.toHtmlEscaped ();
				lineColor = formatter.GetNickColor (nick, colors);
			}
			html += postNick + ' ';

			const auto preparePlain = [&]
			{
				auto text = message.Body_.toHtmlEscaped ();
				formatter.FormatLinks (text);
				text.replace ('\n', "<br/>"_qs);
				return text;
			};
			html += message.RichBody_ ? *message.RichBody_ : preparePlain ();

			if (LastSearchCursor_ == message.Id_)
			{
				lineColor = "#FF7E00"_qs;
				scrollPos = Ui_.HistView_->document ()->characterCount ();
			}

			if (lineColor)
				html = "<font color=\"" + *lineColor + "\">" + html + "</font>";

			Ui_.HistView_->append (html);
		}

		if (scrollPos)
		{
			QTextCursor cur { Ui_.HistView_->document () };
			cur.setPosition (*scrollPos);
			Ui_.HistView_->setTextCursor (cur);
			Ui_.HistView_->ensureCursorVisible ();
		}
	}

	void ChatHistoryWidget::ClearHistory ()
	{
		const auto acc = GetCurrentAccount ();
		if (!acc)
			return;

		const auto entry = GetCurrentEntry ();

		auto selected = Util::Map (Ui_.Contacts_->selectionModel ()->selectedRows (),
				[&] (const QModelIndex& idx) { return SortFilter_->mapToSource (idx).row (); });
		if (const auto current = SortFilter_->mapToSource (Ui_.Contacts_->selectionModel ()->currentIndex ());
			current.isValid () && !selected.contains (current.row ()))
			selected << current.row ();
		if (selected.isEmpty ())
			return;

		std::sort (selected.rbegin (), selected.rend ());

		const auto& msg = selected.size () == 1 ?
				tr ("Are you sure you wish to delete chat history with %1?")
					.arg (EntriesModel_.GetItems () [selected.front ()].Base_.HumanReadableId_) :
				tr ("Are you sure you wish to delete chat history with %n entry(ies)?", nullptr, selected.size ());
		if (QMessageBox::question (nullptr, "LeechCraft", msg, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Ui_.Contacts_->clearSelection ();

		{
			const QSignalBlocker blocker { Ui_.Contacts_->selectionModel () };
			for (const auto row : selected)
			{
				Params_.StorageThread_.Run (&Storage2::ClearHistory, EntriesModel_.GetItems () [row].Base_);
				EntriesModel_.RemoveItem (row);
			}
		}

		if (GetCurrentEntry ())
			RequestLogs ({ .Before_ = PerPageAmount_ });
	}
}
