/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailtab.h"
#include <QToolBar>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QFileDialog>
#include <QToolButton>
#include <QMessageBox>
#include <QShortcut>
#include <QInputDialog>
#include <QLabel>
#include <util/util.h>
#include <util/models/util.h>
#include <util/tags/categoryselector.h>
#include <util/sys/extensionsdata.h>
#include <util/sll/urloperator.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sll/util.h>
#include <util/xpc/util.h>
#include <util/gui/util.h>
#include <util/shortcuts/shortcutmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "storage.h"
#include "accountdatabase.h"
#include "mailtreedelegate.h"
#include "mailmodel.h"
#include "viewcolumnsmanager.h"
#include "accountfoldermanager.h"
#include "vmimeconversions.h"
#include "mailsortmodel.h"
#include "headersviewwidget.h"
#include "mailwebpage.h"
#include "foldersmodel.h"
#include "mailmodelsmanager.h"
#include "messagelisteditormanager.h"
#include "composemessagetabfactory.h"
#include "accountsmanager.h"
#include "structures.h"
#include "util.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Snails
{
	MailTab::MailTab (const ICoreProxy_ptr& proxy,
			const AccountsManager *accsMgr,
			ComposeMessageTabFactory *cmpMsgTabFactory,
			Storage *st,
			const TabClassInfo& tc,
			Util::ShortcutManager *sm,
			QObject *pmt,
			QWidget *parent)
	: QWidget (parent)
	, Proxy_ (proxy)
	, ComposeMessageTabFactory_ (cmpMsgTabFactory)
	, AccsMgr_ (accsMgr)
	, Storage_ (st)
	, TabToolbar_ (new QToolBar)
	, MsgToolbar_ (new QToolBar)
	, TabClass_ (tc)
	, PMT_ (pmt)
	, MailSortFilterModel_ (new MailSortModel { this })
	{
		Ui_.setupUi (this);

		Ui_.TreeViewSplitter_->setSizes ({ Ui_.MailTree_->minimumWidth (), Ui_.MailView_->minimumWidth () });

		MailWebPage_ = new MailWebPage { Proxy_, Ui_.MailView_ };
		Ui_.MailView_->setPage (MailWebPage_);
		Ui_.MailView_->settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, true);

		Ui_.AccountsTree_->setModel (AccsMgr_->GetAccountsModel ());

		MailSortFilterModel_->setDynamicSortFilter (true);
		MailSortFilterModel_->setSortRole (MailModel::MailRole::Sort);
		MailSortFilterModel_->sort (static_cast<int> (MailModel::Column::Date),
				Qt::DescendingOrder);

		MailTreeDelegate_ = new MailTreeDelegate ([this] (const QByteArray& id) -> std::optional<MessageInfo>
				{
					if (!CurrAcc_ || !MailModel_)
						return {};

					return Storage_->GetMessageInfo (CurrAcc_.get (), MailModel_->GetCurrentFolder (), id);
				},
				Ui_.MailTree_,
				this);
		Ui_.MailTree_->setItemDelegate (MailTreeDelegate_);
		Ui_.MailTree_->setModel (MailSortFilterModel_);

		MsgListEditorMgr_ = new MessageListEditorManager { Ui_.MailTree_, this };

		connect (Ui_.AccountsTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentAccountChanged (QModelIndex)));
		connect (Ui_.MailTree_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (handleMailSelected ()));

		FillTabToolbarActions (sm);

		ClearMessage ();
	}

	namespace
	{
		struct AugmentedActionInfo
		{
			QString IconName_;
			ActionInfo Info_;
		public:
			AugmentedActionInfo () = default;

			AugmentedActionInfo (const QString& name, const QKeySequence& seq, const QString& iconName)
			: IconName_ { iconName }
			, Info_ { name, seq, {} }
			{
			}

			QString GetName () const
			{
				return Info_.UserVisibleText_;
			}

			QString GetIconName () const
			{
				return IconName_;
			}

			ActionInfo GetInfo (const ICoreProxy_ptr& proxy) const
			{
				auto info = Info_;
				if (!IconName_.isEmpty ())
					info.Icon_ = proxy->GetIconThemeManager ()->GetIcon (IconName_);
				return info;
			}
		};

		using ActionsHash_t = QHash<QString, AugmentedActionInfo>;

		const ActionsHash_t& GetActionInfos ()
		{
			static const ActionsHash_t result
			{
				{ "MailTab.Fetch", { MailTab::tr ("Fetch new mail"), { "Shift+F" }, "mail-receive" } },
				{ "MailTab.Refresh", { MailTab::tr ("Refresh the folder"), { "F" }, "view-refresh" } },
				{ "MailTab.Compose", { MailTab::tr ("Compose a message..."), { "C, N" }, "mail-message-new" } },
				{ "MailTab.Reply", { MailTab::tr ("Reply..."), { "C, R" }, "mail-reply-sender" } },
				{ "MailTab.Forward", { MailTab::tr ("Forward..."), { "C, F" }, "mail-forward" } },
				{ "MailTab.MarkRead", { MailTab::tr ("Mark as read"), { "R" }, "mail-mark-read" } },
				{ "MailTab.MarkUnread", { MailTab::tr ("Mark as unread"), { "U" }, "mail-mark-unread" } },
				{ "MailTab.Remove", { MailTab::tr ("Delete messages"), { "D" }, "list-remove" } },
				{ "MailTab.ViewHeaders", { MailTab::tr ("View headers"), {}, "view-list-text" } },
				{ "MailTab.MultiSelect", { MailTab::tr ("Select multiple messages mode"), {}, "edit-select" } },

				{ "MailTab.SelectAllChildren", { MailTab::tr ("Select all children"), { "S" }, "edit-select-all" } },
				{ "MailTab.ExpandAllChildren", { MailTab::tr ("Expand all children"), { "E" }, "view-list-tree" } },
			};

			return result;
		}

		template<typename Invokable>
		QAction* MakeAction (const QString& id,
				Util::ShortcutManager *sm,
				QObject *parent, Invokable&& slot)
		{
			const auto& info = GetActionInfos () [id];

			auto action = new QAction { info.GetName (), parent };
			action->setProperty ("ActionIcon", info.GetIconName ());

			Util::InvokeOn (slot,
					[&] (const char *slot) { QObject::connect (action, SIGNAL (triggered (bool)), parent, slot); },
					[&] (auto slot) { QObject::connect (action, &QAction::triggered, parent, slot); });

			sm->RegisterAction (id, action);

			return action;
		}

		template<typename Invokable>
		QShortcut* MakeShortcut (const QString& id,
				Util::ShortcutManager *sm,
				const ICoreProxy_ptr& proxy,
				QWidget *parent, Invokable&& slot)
		{
			const auto& info = GetActionInfos () [id];

			auto shortcut = new QShortcut { parent };

			Util::InvokeOn (slot,
					[&] (const char *slot) { QObject::connect (shortcut, SIGNAL (activated ()), parent, slot); },
					[&] (auto slot) { QObject::connect (shortcut, &QShortcut::activated, parent, slot); });

			sm->RegisterShortcut (id, info.GetInfo (proxy), shortcut);

			return shortcut;
		}
	}

	void MailTab::FillShortcutsManager (Util::ShortcutManager *sm, const ICoreProxy_ptr& proxy)
	{
		for (const auto& pair : Util::Stlize (GetActionInfos ()))
			sm->RegisterActionInfo (pair.first, pair.second.GetInfo (proxy));
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

	QObject* MailTab::GetQObject ()
	{
		return this;
	}

	void MailTab::SetFontFamily (FontFamily family, const QFont& font)
	{
		const auto settings = Ui_.MailView_->settings ();
		if (font == QFont {})
			settings->resetFontFamily (static_cast<QWebSettings::FontFamily> (family));
		else
			settings->setFontFamily (static_cast<QWebSettings::FontFamily> (family), font.family ());
	}

	void MailTab::SetFontSize (FontSize type, int size)
	{
		Ui_.MailView_->settings ()->setFontSize (static_cast<QWebSettings::FontSize> (type), size);
	}

	void MailTab::FillCommonActions (Util::ShortcutManager *sm)
	{
		TabToolbar_->addAction (MakeAction ("MailTab.Fetch", sm, this, SLOT (handleFetchNewMail ())));
		TabToolbar_->addAction (MakeAction ("MailTab.Refresh", sm, this, SLOT (handleRefreshFolder ())));
		TabToolbar_->addAction (MakeAction ("MailTab.Compose", sm, this, SLOT (handleCompose ())));
	}

	void MailTab::FillMailActions (Util::ShortcutManager *sm)
	{
		auto registerMailAction = [this] (QObject *obj)
		{
			connect (this,
					SIGNAL (mailActionsEnabledChanged (bool)),
					obj,
					SLOT (setEnabled (bool)));
		};

		const auto msgReply = MakeAction ("MailTab.Reply", sm, this, [this] { HandleLinkedRequested (MsgType::Reply); });
		TabToolbar_->addAction (msgReply);
		registerMailAction (msgReply);

		const auto msgFwd = MakeAction ("MailTab.Forward", sm, this, [this] { HandleLinkedRequested (MsgType::Forward); });
		TabToolbar_->addAction (msgFwd);
		registerMailAction (msgFwd);

		MsgAttachments_ = new QMenu (tr ("Attachments"));
		MsgAttachmentsButton_ = new QToolButton;
		MsgAttachmentsButton_->setProperty ("ActionIcon", "mail-attachment");
		MsgAttachmentsButton_->setMenu (MsgAttachments_);
		MsgAttachmentsButton_->setPopupMode (QToolButton::InstantPopup);
		TabToolbar_->addWidget (MsgAttachmentsButton_);

		TabToolbar_->addSeparator ();

		MsgCopy_ = new QMenu (tr ("Copy messages"));

		const auto msgCopyButton = new QToolButton;
		msgCopyButton->setMenu (MsgCopy_);
		msgCopyButton->setProperty ("ActionIcon", "edit-copy");
		msgCopyButton->setPopupMode (QToolButton::InstantPopup);
		TabToolbar_->addWidget (msgCopyButton);

		registerMailAction (msgCopyButton);

		MsgMove_ = new QMenu (tr ("Move messages"));
		connect (MsgMove_,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleMoveMessages (QAction*)));

		const auto msgMoveButton = new QToolButton;
		msgMoveButton->setMenu (MsgMove_);
		msgMoveButton->setProperty ("ActionIcon", "transform-move");
		msgMoveButton->setPopupMode (QToolButton::InstantPopup);
		TabToolbar_->addWidget (msgMoveButton);

		registerMailAction (msgMoveButton);

		const auto msgMarkRead = MakeAction ("MailTab.MarkRead", sm, this, SLOT (handleMarkMsgRead ()));
		TabToolbar_->addAction (msgMarkRead);
		registerMailAction (msgMarkRead);

		const auto msgMarkUnread = MakeAction ("MailTab.MarkUnread", sm, this, SLOT (handleMarkMsgUnread ()));
		TabToolbar_->addAction (msgMarkUnread);
		registerMailAction (msgMarkUnread);

		const auto msgRemove = MakeAction ("MailTab.Remove", sm, this, SLOT (handleRemoveMsgs ()));
		TabToolbar_->addAction (msgRemove);
		registerMailAction (msgRemove);

		TabToolbar_->addSeparator ();

		const auto msgViewHeaders = MakeAction ("MailTab.ViewHeaders", sm, this, SLOT (handleViewHeaders ()));
		TabToolbar_->addAction (msgViewHeaders);
		registerMailAction (msgViewHeaders);

		const auto multiSelect = MakeAction ("MailTab.MultiSelect", sm, this, SLOT (handleMultiSelect (bool)));
		multiSelect->setCheckable (true);
		TabToolbar_->addAction (multiSelect);

		UpdateMsgActionsStatus ();

		MakeShortcut ("MailTab.ExpandAllChildren", sm, Proxy_, this, SLOT (expandAllChildren ()));
		MakeShortcut ("MailTab.SelectAllChildren", sm, Proxy_, this, SLOT (selectAllChildren ()));

		TabToolbar_->addSeparator ();

		MakeViewTypeButton ();
	}

	void MailTab::MakeViewTypeButton ()
	{
		const auto viewTypeMenu = new QMenu (tr ("Message view type"));
		const auto viewTypePlain = viewTypeMenu->addAction (tr ("Plain text"), this, [this] { SetHtmlViewAllowed (false); });
		viewTypePlain->setCheckable (true);
		viewTypePlain->setChecked (!HtmlViewAllowed_);
		viewTypePlain->setProperty ("ActionIcon", "text-plain");
		const auto viewTypeHtml = viewTypeMenu->addAction (tr ("HTML"), this, [this] { SetHtmlViewAllowed (true); });
		viewTypeHtml->setCheckable (true);
		viewTypeHtml->setChecked (HtmlViewAllowed_);
		viewTypeHtml->setProperty ("ActionIcon", "text-html");
		const auto viewTypeGroup = new QActionGroup { viewTypeMenu };
		viewTypeGroup->addAction (viewTypePlain);
		viewTypeGroup->addAction (viewTypeHtml);

		const auto viewTypeButton = new QToolButton;
		viewTypeButton->setMenu (viewTypeMenu);
		viewTypeButton->setProperty ("ActionIcon", "text-enriched");
		viewTypeButton->setPopupMode (QToolButton::InstantPopup);
		TabToolbar_->addWidget (viewTypeButton);
	}

	void MailTab::FillTabToolbarActions (Util::ShortcutManager *sm)
	{
		FillCommonActions (sm);
		TabToolbar_->addSeparator ();
		FillMailActions (sm);
	}

	QList<QByteArray> MailTab::GetSelectedIds () const
	{
		switch (MailListMode_)
		{
		case MailListMode::Normal:
		{
			QList<QByteArray> ids;
			for (const auto& index : Ui_.MailTree_->selectionModel ()->selectedRows ())
				ids << index.data (MailModel::MailRole::ID).toByteArray ();

			const auto& currentId = Ui_.MailTree_->currentIndex ()
					.data (MailModel::MailRole::ID).toByteArray ();
			if (!currentId.isEmpty () && !ids.contains (currentId))
				ids << currentId;
			return ids;
		}
		case MailListMode::MultiSelect:
			return MailModel_->GetCheckedIds ();
		}
	}

	QList<Folder> MailTab::GetActualFolders () const
	{
		if (!CurrAcc_)
			return {};

		auto folders = CurrAcc_->GetFolderManager ()->GetFolders ();
		const auto& curFolder = MailModel_->GetCurrentFolder ();

		const auto curPos = std::find_if (folders.begin (), folders.end (),
				[&curFolder] (const Folder& other) { return other.Path_ == curFolder; });
		if (curPos != folders.end ())
			folders.erase (curPos);

		return folders;
	}

	namespace
	{
		QString FormatAddresses (const Addresses_t& adds)
		{
			QStringList result;

			for (const auto& addr : adds)
			{
				if (addr.Email_.isEmpty ())
					continue;

				const bool hasName = !addr.Name_.isEmpty ();

				QString thisStr;

				if (hasName)
					thisStr += addr.Name_.toHtmlEscaped () + " &lt;";
				thisStr += u"<a href='mailto:%1'>%1</a>"_qsv
						.arg (addr.Email_.toHtmlEscaped ());
				if (hasName)
					thisStr += '>';

				result << thisStr;
			}

			return result.join (", ");
		}

		QString GetStyle ()
		{
			const auto& palette = qApp->palette ();

			auto result = Core::Instance ().GetMsgViewTemplate ();
			result.replace ("$WindowText", palette.color (QPalette::ColorRole::WindowText).name ());
			result.replace ("$Window", palette.color (QPalette::ColorRole::Window).name ());
			result.replace ("$Base", palette.color (QPalette::ColorRole::Base).name ());
			result.replace ("$Text", palette.color (QPalette::ColorRole::Text).name ());
			result.replace ("$LinkVisited", palette.color (QPalette::ColorRole::LinkVisited).name ());
			result.replace ("$Link", palette.color (QPalette::ColorRole::Link).name ());
			return result;
		}

		QString GetMessageContents (const std::optional<MessageBodies>& bodies, bool allowHtml)
		{
			if (!bodies)
				return "<em>" + MailTab::tr ("Fetching the message...") + "</em>";

			const auto& htmlBody = bodies->HTML_;
			if (allowHtml && !htmlBody.isEmpty ())
				return htmlBody;

			auto body = bodies->PlainText_;
			body.replace ("\r\n", "\n");

			auto lines = body.split ('\n');
			for (auto& line : lines)
			{
				const auto& escaped = line.toHtmlEscaped ();
				if (line.startsWith ('>'))
					line = "<span class='replyPart'>" + escaped + "</span>";
				else
					line = escaped;
			}

			return "<pre>" + lines.join ("\n") + "</pre>";
		}

		QString ToHtml (const std::optional<MessageBodies>& bodies, bool htmlAllowed)
		{
			QString html = "<!DOCTYPE html><html><head><title>Message</title><style>";
			html += GetStyle ();
			html += "</style></head><body><div class='body' id='msgBody'>";
			html += GetMessageContents (bodies, htmlAllowed);
			html += "</div></body></html>";

			return html;
		}

		QString ToHtmlError (const QString& err)
		{
			QString html = "<!DOCTYPE html><html><head><title>Message</title><style>";
			html += GetStyle ();
			html += "</style><body><div style='errormessage'>" + err + "</div></body></html>";
			return html;
		}
	}

	void MailTab::SetMessage (const MessageInfo& msgInfo, const std::optional<MessageBodies>& bodies)
	{
		ClearMessage ();

		auto addField = [this] (const QString& name, const QString& text)
		{
			if (!text.isEmpty ())
			{
				auto label = new QLabel;
				label->setTextFormat (Qt::RichText);
				label->setText (text);
				label->setWordWrap (true);
				Ui_.MailInfoLayout_->addRow (name + ":", label);
			}
		};

		addField (tr ("Subject"), msgInfo.Subject_.toHtmlEscaped ());
		addField (tr ("From"), FormatAddresses (msgInfo.Addresses_ [AddressType::From]));
		addField (tr ("Reply to"), FormatAddresses (msgInfo.Addresses_ [AddressType::ReplyTo]));
		addField (tr ("To"), FormatAddresses (msgInfo.Addresses_ [AddressType::To]));
		addField (tr ("Copy"), FormatAddresses (msgInfo.Addresses_ [AddressType::Cc]));
		addField (tr ("Blind copy"), FormatAddresses (msgInfo.Addresses_ [AddressType::Bcc]));
		addField (tr ("Date"), msgInfo.Date_.toString ());

		MailWebPage_->SetMessageContext ({ CurrAcc_.get (), msgInfo });
		Ui_.MailView_->setHtml (ToHtml (bodies, HtmlViewAllowed_));

		MsgAttachments_->clear ();
		MsgAttachmentsButton_->setEnabled (!msgInfo.Attachments_.isEmpty ());

		const auto& msgId = msgInfo.FolderId_;
		const auto& folder = msgInfo.Folder_;
		for (const auto& att : msgInfo.Attachments_)
		{
			const auto& name = att.GetName () + " (" + Util::MakePrettySize (att.GetSize ()) + ")";
			MsgAttachments_->addAction (name,
					this,
					[this, msgId, folder, name = att.GetName ()] { HandleAttachment (msgId, folder, name); });
		}
	}

	void MailTab::ClearMessage ()
	{
		Ui_.MailView_->setHtml ({});

		while (Ui_.MailInfoLayout_->rowCount ())
			Ui_.MailInfoLayout_->removeRow (0);
	}

	void MailTab::handleCurrentAccountChanged (const QModelIndex& idx)
	{
		if (CurrAcc_)
		{
			disconnect (CurrAcc_.get (),
					0,
					this,
					0);
			disconnect (CurrAcc_->GetFolderManager (),
					0,
					this,
					0);

			MailSortFilterModel_->setSourceModel (nullptr);
			MailModel_.reset ();
			CurrAcc_.reset ();

			rebuildOpsToFolders ();
		}

		CurrAcc_ = AccsMgr_->GetAccount (idx);
		if (!CurrAcc_)
			return;

		connect (CurrAcc_.get (),
				&Account::willMoveMessages,
				this,
				&MailTab::deselectCurrent);
		connect (CurrAcc_->GetFoldersModel (),
				&FoldersModel::msgMoveRequested,
				this,
				[this] (const QList<QByteArray>& ids, const QStringList& src, const QStringList& to, Qt::DropAction act)
				{
					PerformMoveMessages (ids, src, { to },
							act == Qt::MoveAction ?
									MoveMessagesAction::Move :
									MoveMessagesAction::Copy);
				},
				Qt::UniqueConnection);

		MailModel_ = CurrAcc_->GetMailModelsManager ()->CreateModel ();

		connect (MailModel_.get (),
				&MailModel::messageListUpdated,
				MsgListEditorMgr_,
				&MessageListEditorManager::HandleMessageListUpdated);
		connect (MailModel_.get (),
				&MailModel::messagesSelectionChanged,
				this,
				&MailTab::UpdateMsgActionsStatus);

		MailSortFilterModel_->setSourceModel (MailModel_.get ());
		MailSortFilterModel_->setDynamicSortFilter (true);
		for (int i = 1; i < MailModel_->columnCount (); ++i)
			Ui_.MailTree_->hideColumn (i);

		if (Ui_.TagsTree_->selectionModel ())
			Ui_.TagsTree_->selectionModel ()->deleteLater ();
		Ui_.TagsTree_->setModel (CurrAcc_->GetFoldersModel ());

		connect (Ui_.TagsTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentTagChanged (QModelIndex)));

		const auto fm = CurrAcc_->GetFolderManager ();
		connect (fm,
				SIGNAL (foldersUpdated ()),
				this,
				SLOT (rebuildOpsToFolders ()));
		rebuildOpsToFolders ();
	}

	void MailTab::handleCurrentTagChanged (const QModelIndex& sidx)
	{
		const auto& folder = sidx.data (FoldersModel::Role::FolderPath).toStringList ();
		CurrAcc_->GetMailModelsManager ()->ShowFolder (folder, MailModel_.get ());
		Ui_.MailTree_->setCurrentIndex ({});

		handleMailSelected ();
		rebuildOpsToFolders ();
	}

	void MailTab::UpdateMsgActionsStatus ()
	{
		switch (MailListMode_)
		{
		case MailListMode::Normal:
			emit mailActionsEnabledChanged (static_cast<bool> (CurrMsgInfo_));
			break;
		case MailListMode::MultiSelect:
			emit mailActionsEnabledChanged (MailModel_->HasCheckedIds ());
			break;
		}
	}

	void MailTab::CheckFetchChildMessages (const QModelIndex& rootIdx)
	{
		if (XmlSettingsManager::Instance ().property ("FetchMessagesPolicy").toByteArray () != "MessageAndChildren")
			return;

		QList<QByteArray> missing;

		const auto& folder = MailModel_->GetCurrentFolder ();
		Util::EnumerateChildren (rootIdx, false,
				[this, &missing, &folder] (const QModelIndex& idx)
				{
					const auto& id = idx.data (MailModel::MailRole::ID).toByteArray ();
					if (!Storage_->HasMessageBodies (CurrAcc_.get (), folder, id))
						missing << id;
				});

		if (!missing.isEmpty ())
			CurrAcc_->PrefetchWholeMessages (folder, missing);
	}

	void MailTab::handleMailSelected ()
	{
		const auto updateActionsGuard = Util::MakeScopeGuard ([this] { UpdateMsgActionsStatus (); });
		if (!CurrAcc_)
		{
			ClearMessage ();
			return;
		}

		const auto& folder = MailModel_->GetCurrentFolder ();

		CurrMsgInfo_.reset ();
		CurrMsgBodies_.reset ();
		CurrMsgFetchFuture_.reset ();

		const auto& sidx = Ui_.MailTree_->currentIndex ();

		if (!sidx.isValid () ||
				!Ui_.MailTree_->selectionModel ()->selectedIndexes ().contains (sidx))
		{
			ClearMessage ();
			return;
		}

		const auto& idx = MailSortFilterModel_->mapToSource (sidx);
		const auto& id = idx.data (MailModel::MailRole::ID).toByteArray ();

		const auto& msgInfo = idx.data (MailModel::MailRole::MsgInfo).value<MessageInfo> ();
		auto bodies = Storage_->GetMessageBodies (CurrAcc_.get (), folder, id);

		SetMessage (msgInfo, bodies);

		if (!bodies)
		{
			auto future = CurrAcc_->FetchWholeMessage (folder, id);
			CurrMsgFetchFuture_ = std::make_shared<Account::FetchWholeMessageResult_t> (future);
			Util::Sequence (this, future) >>
					[this, thisFolderId = id] (const auto& result)
					{
						const auto& cur = Ui_.MailTree_->currentIndex ();
						const auto& curId = cur.data (MailModel::MailRole::ID).toByteArray ();
						if (curId != thisFolderId)
							return;

						CurrMsgFetchFuture_.reset ();
						Util::Visit (result.AsVariant (),
								[this] (const MessageBodies& bodies)
								{
									CurrMsgBodies_ = bodies;
									SetMessage (*CurrMsgInfo_, CurrMsgBodies_);
								},
								[this] (auto err)
								{
									ClearMessage ();

									const auto& errMsg = Util::Visit (err,
											[] (auto e) { return QString::fromUtf8 (e.what ()); });
									const auto& msg = tr ("Unable to fetch whole message: %1.")
												.arg (errMsg);
									Ui_.MailView_->setHtml (ToHtmlError (msg));
								});
					};
		}

		CurrMsgInfo_ = msgInfo;
		CurrMsgBodies_ = std::move (bodies);

		CurrAcc_->SetReadStatus (true, { id }, folder);

		CheckFetchChildMessages (idx);
	}

	void MailTab::rebuildOpsToFolders ()
	{
		MsgCopy_->clear ();
		MsgMove_->clear ();

		if (!CurrAcc_)
			return;

		const auto& folders = GetActualFolders ();

		auto setter = [this, &folders] (QMenu *menu, const char *multislot)
		{
			menu->addAction ("Multiple folders...", this, multislot);
			menu->addSeparator ();

			for (const auto& folder : folders)
			{
				const auto& icon = GetFolderIcon (folder.Type_);
				const auto act = menu->addAction (icon, folder.Path_.join ("/"));
				act->setProperty ("Snails/FolderPath", folder.Path_);
			}
		};

		setter (MsgCopy_, SLOT (handleCopyMultipleFolders ()));
		setter (MsgMove_, SLOT (handleMoveMultipleFolders ()));
	}

	void MailTab::handleCompose ()
	{
		if (!CurrAcc_)
			return;

		ComposeMessageTabFactory_->PrepareComposeTab (CurrAcc_);
	}

	void MailTab::HandleLinkedRequested (MsgType type)
	{
		if (!CurrAcc_ || !CurrMsgInfo_)
			return;

		if (CurrMsgFetchFuture_)
			ComposeMessageTabFactory_->PrepareLinkedTab (type, CurrAcc_, *CurrMsgInfo_, *CurrMsgFetchFuture_);
		else if (CurrMsgBodies_)
			ComposeMessageTabFactory_->PrepareLinkedTab (type, CurrAcc_, *CurrMsgInfo_, *CurrMsgBodies_);
	}

	void MailTab::SetHtmlViewAllowed (bool allowed)
	{
		if (allowed == HtmlViewAllowed_)
			return;

		HtmlViewAllowed_ = allowed;
		if (CurrMsgInfo_)
			SetMessage (*CurrMsgInfo_, CurrMsgBodies_);
	}

	namespace
	{
		QList<QStringList> RunSelectFolders (const QList<Folder>& folders, const QString& title)
		{
			Util::CategorySelector sel;
			sel.setWindowTitle (title);
			sel.SetCaption (MailTab::tr ("Folders"));

			QStringList folderNames;
			QList<QStringList> folderPaths;
			for (const auto& folder : folders)
			{
				folderNames << folder.Path_.join ("/");
				folderPaths << folder.Path_;
			}

			sel.SetPossibleSelections (folderNames, false);
			sel.SetButtonsMode (Util::CategorySelector::ButtonsMode::AcceptReject);

			if (sel.exec () != QDialog::Accepted)
				return {};

			QList<QStringList> selectedPaths;
			for (const auto index : sel.GetSelectedIndexes ())
				selectedPaths << folderPaths [index];
			return selectedPaths;
		}
	}

	void MailTab::PerformMoveMessages (const QList<QByteArray>& ids,
			const QList<QStringList>& targets, MoveMessagesAction action)
	{
		PerformMoveMessages (ids, MailModel_->GetCurrentFolder (), targets, action);
	}

	void MailTab::PerformMoveMessages (const QList<QByteArray>& ids,
			const QStringList& source, const QList<QStringList>& targets, MoveMessagesAction action)
	{
		if (ids.isEmpty () ||
				targets.isEmpty () ||
				std::any_of (targets.begin (), targets.end (), [] (const auto& folder) { return folder.isEmpty (); }))
			return;

		switch (action)
		{
		case MoveMessagesAction::Copy:
			CurrAcc_->CopyMessages (ids, source, targets);
			break;
		case MoveMessagesAction::Move:
			CurrAcc_->MoveMessages (ids, source, targets);
			break;
		}
	}

	void MailTab::handleCopyMultipleFolders ()
	{
		const auto& ids = GetSelectedIds ();
		const auto& selectedPaths = RunSelectFolders (GetActualFolders (),
				ids.size () == 1 ?
					tr ("Copy message") :
					tr ("Copy %n message(s)", nullptr, ids.size ()));
		PerformMoveMessages (ids, selectedPaths, MoveMessagesAction::Copy);
	}

	void MailTab::handleCopyMessages (QAction *action)
	{
		PerformMoveMessages (GetSelectedIds (),
				{ action->property ("Snails/FolderPath").toStringList () },
				MoveMessagesAction::Copy);
	}

	void MailTab::handleMoveMultipleFolders ()
	{
		const auto& ids = GetSelectedIds ();
		const auto& selectedPaths = RunSelectFolders (GetActualFolders (),
				ids.size () == 1 ?
					tr ("Move message") :
					tr ("Move %n message(s)", nullptr, ids.size ()));
		PerformMoveMessages (ids, selectedPaths, MoveMessagesAction::Move);
	}

	void MailTab::handleMoveMessages (QAction *action)
	{
		PerformMoveMessages (GetSelectedIds (),
				{ action->property ("Snails/FolderPath").toStringList () },
				MoveMessagesAction::Move);
	}

	void MailTab::handleMarkMsgRead ()
	{
		if (!CurrAcc_)
			return;

		CurrAcc_->SetReadStatus (true, GetSelectedIds (), MailModel_->GetCurrentFolder ());
	}

	void MailTab::handleMarkMsgUnread ()
	{
		if (!CurrAcc_)
			return;

		CurrAcc_->SetReadStatus (false, GetSelectedIds (), MailModel_->GetCurrentFolder ());
	}

	void MailTab::handleRemoveMsgs ()
	{
		if (!CurrAcc_)
			return;

		const auto& ids = GetSelectedIds ();

		const auto& folder = MailModel_->GetCurrentFolder ();

		CurrAcc_->DeleteMessages (ids, folder);
	}

	void MailTab::handleViewHeaders ()
	{
		if (!CurrAcc_)
			return;

		const auto& folder = MailModel_->GetCurrentFolder ();
		for (const auto& id : GetSelectedIds ())
		{
			const auto& header = Storage_->BaseForAccount (CurrAcc_.get ())->GetMessageHeader (folder, id);
			if (!header)
				continue;

			auto widget = new HeadersViewWidget { *header, this };
			widget->setAttribute (Qt::WA_DeleteOnClose);
			widget->setWindowFlags (Qt::Dialog);
			widget->setWindowTitle (tr ("Message headers"));
			widget->show ();
		}
	}

	void MailTab::handleMultiSelect (bool checked)
	{
		const auto mode = checked ?
				MailListMode::MultiSelect :
				MailListMode::Normal;

		MailListMode_ = mode;

		MailTreeDelegate_->SetMailListMode (mode);
		MsgListEditorMgr_->SetMailListMode (mode);

		UpdateMsgActionsStatus ();
	}

	namespace
	{
		template<typename F>
		void Recurse (const QModelIndex& index, QAbstractItemModel *model, const F& f)
		{
			f (index);

			for (int i = 0; i < model->rowCount (index); ++i)
				Recurse (model->index (i, 0, index), model, f);
		}
	}

	void MailTab::selectAllChildren ()
	{
		if (!CurrAcc_)
			return;

		const auto model = Ui_.MailTree_->model ();

		const auto sm = Ui_.MailTree_->selectionModel ();
		for (const auto& idx : sm->selectedRows ())
			Recurse (idx, model,
					[&] (const QModelIndex& idx)
					{
						Ui_.MailTree_->expand (idx);
						sm->select (idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
					});
	}

	void MailTab::expandAllChildren ()
	{
		if (!CurrAcc_)
			return;

		const auto model = Ui_.MailTree_->model ();
		for (const auto& idx : Ui_.MailTree_->selectionModel ()->selectedRows ())
			Recurse (idx, model, [&] (const QModelIndex& idx) { Ui_.MailTree_->expand (idx); });
	}

	void MailTab::deselectCurrent (const QList<QByteArray>& ids, const QStringList& folder)
	{
		if (folder != MailModel_->GetCurrentFolder ())
			return;

		const auto selModel = Ui_.MailTree_->selectionModel ();
		if (!selModel)
			return;

		const auto& curIdx = Ui_.MailTree_->currentIndex ();
		const auto& currentId = curIdx.data (MailModel::MailRole::ID).toByteArray ();
		if (!ids.contains (currentId))
			return;

		selModel->clearCurrentIndex ();
	}

	void MailTab::HandleAttachment (const QByteArray& id, const QStringList& folder, const QString& name)
	{
		if (!CurrAcc_)
			return;

		RunAttachmentSaveDialog (CurrAcc_.get (), Proxy_->GetEntityManager (), id, folder, name);
	}

	void MailTab::handleFetchNewMail ()
	{
		for (const auto& acc : AccsMgr_->GetAccounts ())
			acc->Synchronize ();
	}

	void MailTab::handleRefreshFolder ()
	{
		if (!CurrAcc_)
			return;

		auto future = CurrAcc_->Synchronize (MailModel_->GetCurrentFolder ());

		auto proxy = Proxy_;
		Util::Sequence (nullptr, future) >>
				Util::Visitor
				{
					[proxy] (const Account::SyncStats& stats)
					{
						const auto& text = stats.NewMsgsCount_ ?
								tr ("Got %n new messages.", 0, stats.NewMsgsCount_) :
								tr ("No new messages.");
						const auto& e = Util::MakeNotification ("Snails", text, Priority::Info);
						proxy->GetEntityManager ()->HandleEntity (e);
					},
					[proxy] (const auto& err)
					{
						qDebug () << "!!";
						const auto& text = tr ("Error fetching new mail: %1")
								.arg (Util::Visit (err, [] (auto e) { return e.what (); }));
						const auto& e = Util::MakeNotification ("Snails", text, Priority::Critical);
						proxy->GetEntityManager ()->HandleEntity (e);
					}
				};
	}

	namespace
	{
		void RunCreateFolder (QStringList path, Account& acc, const ICoreProxy_ptr& proxy, QWidget *parent)
		{
			const auto& label = path.isEmpty () ?
					MailTab::tr ("Enter the name of the folder:") :
					MailTab::tr ("Enter the name of the folder to be created under %1:")
							.arg (Util::FormatName (path.join ("/")));
			const auto& name = QInputDialog::getText (parent, MailTab::tr ("Create folder"), label);
			if (name.isEmpty ())
				return;

			path.append (name);

			auto errHandler = [name, proxy] (const auto& err)
			{
				const auto& text = Util::Visit (err,
						[name] (const FolderAlreadyExists&)
						{
							return MailTab::tr ("Folder %1 already exists.").arg (Util::FormatName (name));
						},
						[] (const InvalidPathComponent& comp)
						{
							return MailTab::tr ("Path component %1 is invalid.").arg (Util::FormatName (comp.Component_));
						},
						[] (const auto& e) { return QString { e.what () }; });
				const auto& e = Util::MakeNotification ("Snails", text, Priority::Warning);
				proxy->GetEntityManager ()->HandleEntity (e);
			};

			Util::Sequence (parent, acc.CreateFolder (path)) >>
					Util::Visitor
					{
						[] (Util::Void) {},
						errHandler
					};
		}
	}

	void MailTab::on_TagsTree__customContextMenuRequested (const QPoint& pos)
	{
		if (!CurrAcc_)
			return;

		const auto& idx = Ui_.TagsTree_->indexAt (pos);

		auto path = idx.isValid () ?
				idx.data (FoldersModel::Role::FolderPath).toStringList () :
				QStringList {};

		QMenu menu;
		menu.addAction (tr ("Create folder..."), [this, path] { RunCreateFolder (path, *CurrAcc_, Proxy_, this); });
		menu.exec (Ui_.TagsTree_->viewport ()->mapToGlobal (pos));
	}
}
}
