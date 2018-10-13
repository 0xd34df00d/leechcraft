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
#include <util/util.h>
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

namespace LeechCraft
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

		const auto mailWebPage = new MailWebPage { Proxy_, Ui_.MailView_ };
		connect (mailWebPage,
				&MailWebPage::attachmentSelected,
				this,
				&MailTab::HandleAttachment);
		Ui_.MailView_->setPage (mailWebPage);
		Ui_.MailView_->settings ()->setAttribute (QWebSettings::DeveloperExtrasEnabled, true);

		Ui_.AccountsTree_->setModel (AccsMgr_->GetAccountsModel ());

		MailSortFilterModel_->setDynamicSortFilter (true);
		MailSortFilterModel_->setSortRole (MailModel::MailRole::Sort);
		MailSortFilterModel_->sort (static_cast<int> (MailModel::Column::Date),
				Qt::DescendingOrder);

		MailTreeDelegate_ = new MailTreeDelegate ([this] (const QByteArray& id) -> Message_ptr
				{
					if (!CurrAcc_ || !MailModel_)
						return {};
					return Storage_->LoadMessage (CurrAcc_.get (), MailModel_->GetCurrentFolder (), id);
				},
				Ui_.MailTree_,
				this);
		Ui_.MailTree_->setItemDelegate (MailTreeDelegate_);
		Ui_.MailTree_->setModel (MailSortFilterModel_);

		MsgListEditorMgr_ = new MessageListEditorManager { Ui_.MailTree_, MailTreeDelegate_, this };

		connect (Ui_.AccountsTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleCurrentAccountChanged (QModelIndex)));
		connect (Ui_.MailTree_->selectionModel (),
				SIGNAL (selectionChanged (QItemSelection, QItemSelection)),
				this,
				SLOT (handleMailSelected ()));

		FillTabToolbarActions (sm);
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

	void MailTab::SetFontSizeMultiplier (qreal factor)
	{
		Ui_.MailView_->setTextSizeMultiplier (factor);
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
		QList<QByteArray> ids;
		switch (MailListMode_)
		{
		case MailListMode::Normal:
			for (const auto& index : Ui_.MailTree_->selectionModel ()->selectedRows ())
				ids << index.data (MailModel::MailRole::ID).toByteArray ();
			break;
		case MailListMode::MultiSelect:
			if (MailModel_)
				ids = MailModel_->GetCheckedIds ();
			break;
		}

		const auto& currentId = Ui_.MailTree_->currentIndex ()
				.data (MailModel::MailRole::ID).toByteArray ();
		if (!currentId.isEmpty () && !ids.contains (currentId))
			ids << currentId;

		return ids;
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
		QString HTMLize (const QList<QPair<QString, QString>>& adds)
		{
			QStringList result;

			for (const auto& pair : adds)
			{
				if (pair.second.isEmpty ())
					continue;

				const bool hasName = !pair.first.isEmpty ();

				QString thisStr;

				if (hasName)
					thisStr += "<span style='address_name'>" + pair.first + "</span> &lt;";

				thisStr += QString ("<span style='address_email'><a href='mailto:%1'>%1</a></span>")
						.arg (pair.second);

				if (hasName)
					thisStr += '>';

				result << thisStr;
			}

			return result.join (", ");
		}

		QString GetStyle (QString headerClass)
		{
			headerClass.prepend ('.');

			const auto& palette = qApp->palette ();

			auto result = Core::Instance ().GetMsgViewTemplate ();
			result.replace (".header", headerClass);
			result.replace ("$WindowText", palette.color (QPalette::ColorRole::WindowText).name ());
			result.replace ("$Window", palette.color (QPalette::ColorRole::Window).name ());
			result.replace ("$Base", palette.color (QPalette::ColorRole::Base).name ());
			result.replace ("$Text", palette.color (QPalette::ColorRole::Text).name ());
			result.replace ("$LinkVisited", palette.color (QPalette::ColorRole::LinkVisited).name ());
			result.replace ("$Link", palette.color (QPalette::ColorRole::Link).name ());

			result += R"(
						html, body {
							width: 100%;
							height: 100%;
							overflow: hidden;
						}
					)" + headerClass + R"( {
							top: 0;
							left: 0;
							right: 0;
							position: fixed;
						}

						.body {
							width: 100%;
							bottom: 0;
							position: absolute;
							overflow: auto;
						}
					)";

			return result;
		}

		QString AttachmentsToHtml (const Message_ptr& msg, const QList<AttDescr>& attachments)
		{
			if (attachments.isEmpty ())
				return {};

			QString result;
			result += "<div class='attachments'>";

			const auto& extData = Util::ExtensionsData::Instance ();
			for (const auto& attach : attachments)
			{
				result += "<span class='attachment'>";

				QUrl linkUrl { "snails://attachment/" };
				Util::UrlOperator { linkUrl }
						("msgId", msg->GetFolderID ())
						("folderId", msg->GetFolders ().value (0).join ("/"))
						("attName", attach.GetName ());
				const auto& link = linkUrl.toEncoded ();

				result += "<a href='" + link + "'>";

				const auto& mimeType = attach.GetType () + '/' + attach.GetSubType ();
				const auto& icon = extData.GetMimeIcon (mimeType);
				if (!icon.isNull ())
				{
					const auto& iconData = Util::GetAsBase64Src (icon.pixmap (16, 16).toImage ());

					result += "<img class='attachMime' style='float:left' src='" + iconData + "' alt='" + mimeType + "' />";
				}

				result += "</a><span><a href='" + link + "'>";

				result += attach.GetName ();
				if (!attach.GetDescr ().isEmpty ())
					result += " (" + attach.GetDescr () + ")";
				result += " &mdash; " + Util::MakePrettySize (attach.GetSize ());

				result += "</a></span>";
				result += "</span>";
			}

			result += "</div>";
			return result;
		}

		QString GenerateId (const QString& body, QString classId)
		{
			int pos = 0;
			while ((pos = body.indexOf (classId)) >= 0)
			{
				const auto nextCharIdx = pos + classId.size ();
				if (nextCharIdx >= body.size ())
					classId += '1';
				else
					classId += body.at (nextCharIdx) == '1' ?
							'2' :
							'1';
			}

			return classId;
		}

		QString GetMessageContents (const Message_ptr& msg, bool allowHtml)
		{
			if (!msg->IsFullyFetched ())
				return "<em>" + MailTab::tr ("Fetching the message...") + "</em>";

			const auto& htmlBody = msg->GetHTMLBody ();
			if (allowHtml && !htmlBody.isEmpty ())
				return htmlBody;

			auto body = msg->GetBody ();
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

		QString ToHtml (const Message_ptr& msg, bool htmlAllowed)
		{
			const auto& headerClass = GenerateId (msg->GetHTMLBody (), "header");

			QString html = R"(<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">)";
			html += "<html xmlns='http://www.w3.org/1999/xhtml'><head><title>Message</title><style>";
			html += GetStyle (headerClass);
			html += "</style>";
			html += R"d(
						<script language='javascript'>
							function resizeBody() {
								var headHeight = document.getElementById('msgHeader').offsetHeight;

								var bodyElem = document.getElementById('msgBody');
								bodyElem.style.top = headHeight + "px";
							}
							function setupResize() {
								window.onresize = resizeBody;
								resizeBody();
							}
						</script>
					)d";
			html += "</head><body onload='setupResize()'><header class='" + headerClass + "' id='msgHeader'>";
			auto addField = [&html] (const QString& cssClass, const QString& name, const QString& text)
			{
				if (!text.trimmed ().isEmpty ())
					html += "<span class='field " + cssClass + "'><span class='fieldName'>" +
							name + ": </span>" + text + "</span><br />";
			};

			addField ("subject", MailTab::tr ("Subject"), msg->GetSubject ());
			addField ("from", MailTab::tr ("From"), HTMLize ({ msg->GetAddress (AddressType::From) }));
			addField ("replyTo", MailTab::tr ("Reply to"), HTMLize ({ msg->GetAddress (AddressType::ReplyTo) }));
			addField ("to", MailTab::tr ("To"), HTMLize (msg->GetAddresses (AddressType::To)));
			addField ("cc", MailTab::tr ("Copy"), HTMLize (msg->GetAddresses (AddressType::Cc)));
			addField ("bcc", MailTab::tr ("Blind copy"), HTMLize (msg->GetAddresses (AddressType::Bcc)));
			addField ("date", MailTab::tr ("Date"), msg->GetDate ().toString ());
			html += AttachmentsToHtml (msg, msg->GetAttachments ());
			html += "</header><div class='body' id='msgBody'>";
			html += GetMessageContents (msg, htmlAllowed);
			html += "</div><script language='javascript'>setupResize();</script>";
			html += "</body></html>";

			return html;
		}

		QString ToHtmlError (const QString& err)
		{
			QString html = R"(<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">)";
			html += "<html xmlns='http://www.w3.org/1999/xhtml'><head><title>Message</title><style>";
			html += GetStyle (".header");
			html += "</style><body><div style='errormessage'>" + err + "</div></body></html>";
			return html;
		}
	}

	void MailTab::SetMessage (const Message_ptr& msg)
	{
		const auto& html = ToHtml (msg, HtmlViewAllowed_);

		Ui_.MailView_->setHtml (html);

		MsgAttachments_->clear ();
		MsgAttachmentsButton_->setEnabled (!msg->GetAttachments ().isEmpty ());
		for (const auto& att : msg->GetAttachments ())
		{
			const auto& name = att.GetName () + " (" + Util::MakePrettySize (att.GetSize ()) + ")";
			MsgAttachments_->addAction (name,
					this,
					[this, id = msg->GetFolderID (), folder = msg->GetFolders ().value (0), name = att.GetName ()]
					{
						HandleAttachment (id, folder, name);
					});
		}
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

		connect (this,
				&MailTab::willMoveMessages,
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
				SIGNAL (messageListUpdated ()),
				MsgListEditorMgr_,
				SLOT (handleMessageListUpdated ()));
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
			emit mailActionsEnabledChanged (static_cast<bool> (CurrMsg_));
			break;
		case MailListMode::MultiSelect:
			emit mailActionsEnabledChanged (MailModel_->HasCheckedIds ());
			break;
		}
	}

	void MailTab::handleMailSelected ()
	{
		const auto updateActionsGuard = Util::MakeScopeGuard ([this] { UpdateMsgActionsStatus (); });
		if (!CurrAcc_)
		{
			Ui_.MailView_->setHtml ({});
			return;
		}

		const auto& folder = MailModel_->GetCurrentFolder ();

		CurrMsg_.reset ();
		CurrMsgFetchFuture_.reset ();

		const auto& sidx = Ui_.MailTree_->currentIndex ();

		if (!sidx.isValid () ||
				!Ui_.MailTree_->selectionModel ()->selectedIndexes ().contains (sidx))
		{
			Ui_.MailView_->setHtml ({});
			return;
		}

		const auto& idx = MailSortFilterModel_->mapToSource (sidx);
		const auto& id = idx.sibling (idx.row (), 0).data (MailModel::MailRole::ID).toByteArray ();

		Message_ptr msg;
		try
		{
			msg = Storage_->LoadMessage (CurrAcc_.get (), folder, id);
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

		SetMessage (msg);

		if (!msg->IsFullyFetched ())
		{
			auto future = CurrAcc_->FetchWholeMessage (msg);
			CurrMsgFetchFuture_ = std::make_shared<Account::FetchWholeMessageResult_t> (future);
			Util::Sequence (this, future) >>
					[this, thisFolderId = msg->GetFolderID ()] (const auto& result)
					{
						const auto& cur = Ui_.MailTree_->currentIndex ();
						const auto& curId = cur.data (MailModel::MailRole::ID).toByteArray ();
						if (curId != thisFolderId)
							return;

						CurrMsgFetchFuture_.reset ();
						Util::Visit (result.AsVariant (),
								[this] (const Message_ptr& msg) { SetMessage (msg); },
								[this] (auto err)
								{
									const auto& errMsg = Util::Visit (err,
											[] (auto e) { return QString::fromUtf8 (e.what ()); });
									const auto& msg = tr ("Unable to fetch whole message: %1.")
												.arg (errMsg);
									Ui_.MailView_->setHtml (ToHtmlError (msg));
								});
					};
		}

		CurrMsg_ = msg;

		CurrAcc_->SetReadStatus (true, { id }, folder);
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
		if (!CurrAcc_ || !CurrMsg_)
			return;

		if (CurrMsgFetchFuture_)
			ComposeMessageTabFactory_->PrepareLinkedTab (type, CurrAcc_, *CurrMsgFetchFuture_);
		else
			ComposeMessageTabFactory_->PrepareLinkedTab (type, CurrAcc_, CurrMsg_);
	}

	void MailTab::SetHtmlViewAllowed (bool allowed)
	{
		if (allowed == HtmlViewAllowed_)
			return;

		HtmlViewAllowed_ = allowed;
		if (CurrMsg_)
			SetMessage (CurrMsg_);
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

			sel.setPossibleSelections (folderNames, false);
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

		emit willMoveMessages (ids, source);

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
					tr ("Copy %n message(s)", 0, ids.size ()));
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
					tr ("Move %n message(s)", 0, ids.size ()));
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
		emit willMoveMessages (ids, folder);

		CurrAcc_->DeleteMessages (ids, folder);
	}

	void MailTab::handleViewHeaders ()
	{
		if (!CurrAcc_)
			return;

		for (const auto& id : GetSelectedIds ())
		{
			const auto& msg = MailModel_->GetMessage (id);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "no message for id"
						<< id;
				continue;
			}

			const auto& header = Storage_->BaseForAccount (CurrAcc_.get ())->GetMessageHeader (msg->GetMessageID ());
			if (!header)
				continue;

			auto widget = new HeadersViewWidget { *header, this };
			widget->setAttribute (Qt::WA_DeleteOnClose);
			widget->setWindowFlags (Qt::Dialog);
			widget->setWindowTitle (tr ("Headers for %1").arg ('"' + msg->GetSubject () + '"'));
			widget->show ();
		}
	}

	void MailTab::handleMultiSelect (bool checked)
	{
		const auto mode = checked ?
				MailListMode::MultiSelect :
				MailListMode::Normal;

		MailListMode_ = mode;

		MailTreeDelegate_->setMailListMode (mode);
		MsgListEditorMgr_->setMailListMode (mode);

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

		MailModel_->MarkUnavailable (ids);

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

		const auto& path = QFileDialog::getSaveFileName (0,
				tr ("Save attachment"),
				QDir::homePath () + '/' + name);
		if (path.isEmpty ())
			return;

		const auto iem = Proxy_->GetEntityManager ();

		const auto& msg = Storage_->LoadMessage (CurrAcc_.get (), folder, id);
		Util::Sequence (nullptr, CurrAcc_->FetchAttachment (msg, name, path)) >>
				Util::Visitor
				{
					[iem, name] (Util::Void)
					{
						iem->HandleEntity (Util::MakeNotification ("LeechCraft Snails",
									tr ("Attachment %1 fetched successfully.")
										.arg (Util::FormatName (name)),
									Priority::Info));
					},
					[iem, name] (auto errVar)
					{
						iem->HandleEntity (Util::MakeNotification ("LeechCraft Snails",
									tr ("Unable to fetch %1: %2.")
											.arg (Util::FormatName (name))
											.arg (Util::Visit (errVar, [] (auto err) { return err.what (); })),
									Priority::Critical));
					}
				};
	}

	void MailTab::handleFetchNewMail ()
	{
		for (const auto acc : AccsMgr_->GetAccounts ())
			acc->Synchronize ();
	}

	void MailTab::handleRefreshFolder ()
	{
		if (!CurrAcc_)
			return;

		CurrAcc_->Synchronize (MailModel_->GetCurrentFolder ());
	}
}
}
