/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "composemessagetab.h"
#include <optional>
#include <QToolBar>
#include <QWebFrame>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QInputDialog>
#include <QTextDocument>
#include <QToolButton>
#include <QTimer>
#include <util/util.h>
#include <util/sys/mimedetector.h>
#include <util/sys/extensionsdata.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sll/unreachable.h>
#include <util/sll/prelude.h>
#include <util/xpc/util.h>
#include <util/gui/util.h>
#include <interfaces/itexteditor.h>
#include <interfaces/iadvancedhtmleditor.h>
#include <interfaces/iadvancedplaintexteditor.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "accountsmanager.h"
#include "accountthread.h"
#include "msgtemplatesmanager.h"
#include "structures.h"
#include "util.h"
#include "attachmentsfetcher.h"
#include "outgoingmessage.h"
#include "messagebodies.h"

namespace LC
{
namespace Snails
{
	QObject *ComposeMessageTab::S_ParentPlugin_ = 0;
	TabClassInfo ComposeMessageTab::S_TabClassInfo_;

	void ComposeMessageTab::SetParentPlugin (QObject *obj)
	{
		S_ParentPlugin_ = obj;
	}

	void ComposeMessageTab::SetTabClassInfo (const TabClassInfo& info)
	{
		S_TabClassInfo_ = info;
	}

	ComposeMessageTab::ComposeMessageTab (const AccountsManager *accsMgr,
			const MsgTemplatesManager *templatesMgr, QWidget *parent)
	: QWidget (parent)
	, AccsMgr_ (accsMgr)
	, TemplatesMgr_ (templatesMgr)
	, Toolbar_ (new QToolBar)
	{
		Ui_.setupUi (this);

		SetupToolbar ();
		SetupEditors ();
	}

	TabClassInfo ComposeMessageTab::GetTabClassInfo () const
	{
		return S_TabClassInfo_;
	}

	QObject* ComposeMessageTab::ParentMultiTabs ()
	{
		return S_ParentPlugin_;
	}

	void ComposeMessageTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* ComposeMessageTab::GetToolBar () const
	{
		return Toolbar_;
	}

	QObject* ComposeMessageTab::GetQObject ()
	{
		return this;
	}

	void ComposeMessageTab::SetFontFamily (FontFamily family, const QFont& font)
	{
		for (const auto editor : Ui_.Editor_->GetAllEditors ())
			if (const auto iwfs = qobject_cast<IWkFontsSettable*> (editor->GetQObject ()))
				iwfs->SetFontFamily (family, font);
	}

	void ComposeMessageTab::SetFontSize (FontSize type, int size)
	{
		for (const auto editor : Ui_.Editor_->GetAllEditors ())
			if (const auto iwfs = qobject_cast<IWkFontsSettable*> (editor->GetQObject ()))
				iwfs->SetFontSize (type, size);
	}

	void ComposeMessageTab::SelectAccount (const Account_ptr& account)
	{
		const auto& var = QVariant::fromValue<Account_ptr> (account);
		for (auto action : AccountsMenu_->actions ())
			if (action->property ("Account") == var)
			{
				action->setChecked (true);
				break;
			}
	}

	namespace
	{
		QString MakeLinkedSubject (QString subj, const QString& marker)
		{
			if (marker.compare (subj.left (marker.size ()), Qt::CaseInsensitive))
				subj.prepend (marker + ": ");
			return subj;
		}

		std::optional<QString> CreateSubj (MsgType type, const MessageInfo& info)
		{
			switch (type)
			{
			case MsgType::New:
				return {};
			case MsgType::Reply:
				return MakeLinkedSubject (info.Subject_, "Re");
			case MsgType::Forward:
				return MakeLinkedSubject (info.Subject_, "Fwd");
			}

			Util::Unreachable ();
		}
	}

	void ComposeMessageTab::PrepareLinked (MsgType type, const MessageInfo& info, const MessageBodies& bodies)
	{
		if (type == MsgType::Reply)
		{
			auto address = info.Addresses_ [AddressType::ReplyTo].value (0);
			if (address.Email_.isEmpty ())
				address = info.Addresses_ [AddressType::From].value (0);
			Ui_.To_->setText (GetNiceMail (address));
		}
		else if (type == MsgType::Forward)
			CopyAttachments (info);

		if (const auto& subj = CreateSubj (type, info))
			Ui_.Subject_->setText (*subj);

		PrepareLinkedBody (type, info, bodies);

		OrigReferences_ = info.References_;
		OrigMessageId_ = info.MessageId_;

		QTimer::singleShot (0, this,
				[this] { Ui_.Editor_->GetCurrentEditor ()->GetWidget ()->setFocus (); });
	}

	void ComposeMessageTab::PrepareLinkedEditor (const MessageBodies& bodies)
	{
		const auto& replyOpt = XmlSettingsManager::Instance ().property ("ReplyMessageFormat").toString ();
		if (replyOpt == "Plain")
			Ui_.Editor_->SelectEditor (ContentType::PlainText);
		else if (replyOpt == "HTML")
			Ui_.Editor_->SelectEditor (ContentType::HTML);
		else if (replyOpt == "Orig")
		{
			if (bodies.HTML_.isEmpty ())
				Ui_.Editor_->SelectEditor (ContentType::PlainText);
			else
				Ui_.Editor_->SelectEditor (ContentType::HTML);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown option"
					<< replyOpt;
	}

	namespace
	{
		void SetupReplyPlaintextContents (IEditorWidget *editor, const QString& text)
		{
			editor->SetContents (text, ContentType::PlainText);

			if (const auto iape = dynamic_cast<IAdvancedPlainTextEditor*> (editor))
				if (iape->FindText ("${CURSOR}"))
					iape->DeleteSelection ();
		}

		static const QString BlockquoteBreakJS =
			R"delim(
				var input = document.querySelector('body');

				input.addEventListener('keydown', function(e, data) {
						if (e.keyCode === 13 && !e.altKey && !e.shiftKey && !e.ctrlKey && !e.metaKey) {
							if (split())
								e.preventDefault();
						}
					});

				function split() {
					var sel = window.getSelection(),
							range,
							textNode,
							elem;

					if (sel.rangeCount) {
						range = sel.getRangeAt(0);
						textNode = range.commonAncestorContainer;
						elem = textNode.parentNode;
						var topBlockquote = findTopBlockquote(elem);
						if (topBlockquote) {
							splitBlockquote(textNode, range.endOffset, topBlockquote);
							return true;
						}
					}

					return false;
				}

				function isBlockquote(elem) {
					return elem.tagName.toLowerCase() === 'blockquote';
				}

				function findTopBlockquote(elem) {
					var lastBlockquote = null;
					while (elem.parentNode && elem.parentNode.tagName) {
						elem = elem.parentNode;
						if (isBlockquote(elem))
							lastBlockquote = elem;
					}

					return lastBlockquote;
				}

				function splitBlockquote(textNode, pos, topBq) {
					var parent = topBq.parentNode,
						parentPos = getNodeIndex(parent, topBq),
						doc = textNode.ownerDocument,
						range = doc.createRange(),
						fragment,
						div = createExtra("div");

					range.setStart(parent, parentPos);
					range.setEnd(textNode, pos);
					fragment = range.extractContents();
					fragment.appendChild(div);

					parent.insertBefore(fragment, topBq);
					select(div);
				}

				function getNodeIndex(parent, node) {
					var index = parent.childNodes.length - 1;

					while (index > 0 && node !== parent.childNodes[index]) {
						index--;
					}

					return index;
				}

				function createExtra(tag) {
					var elem = document.createElement(tag);

					elem.innerHTML = "&#160;";

					return elem;
				}

				function select(elem) {
					var sel = window.getSelection();

					sel.removeAllRanges();
					sel.selectAllChildren(elem);
				}
			)delim";

		static const QString DeleteCursorJS =
			R"delim(
				(function(){
					var el = document.getElementById('place_cursor_here');

					var sel = window.getSelection();
					sel.removeAllRanges();
					sel.selectAllChildren(el);
					sel.collapseToEnd();
					sel.modify("move", "forward", "character");

					el.remove();
				}());
			)delim";

		void SetupReplyRichContents (IEditorWidget *editor, const QString& text)
		{
			editor->SetContents (text, ContentType::HTML);

			if (const auto iahe = dynamic_cast<IAdvancedHTMLEditor*> (editor))
			{
				iahe->ExecJS (BlockquoteBreakJS);
				iahe->ExecJS (DeleteCursorJS);
			}
		}
	}

	void ComposeMessageTab::PrepareLinkedBody (MsgType type, const MessageInfo& info, const MessageBodies& bodies)
	{
		PrepareLinkedEditor (bodies);

		const auto editor = Ui_.Editor_->GetCurrentEditor ();

		const auto acc = GetSelectedAccount ();

		auto tpl = [&] (ContentType cty) { return TemplatesMgr_->GetTemplatedText (cty, type, acc, info, bodies); };

		SetupReplyPlaintextContents (editor, tpl (ContentType::PlainText));
		SetupReplyRichContents (editor, tpl (ContentType::HTML));
	}

	namespace
	{
		QString AttachmentsFetchErrorDescr (const AttachmentsFetcher::Errors_t& err)
		{
			return Util::Visit (err,
					[] (const AttachmentsFetcher::TemporaryDirError&)
					{
						return ComposeMessageTab::tr ("Unable to create temporary directory to "
								"fetch the attachments of the source message.");
					},
					[] (const auto& e)
					{
						return ComposeMessageTab::tr ("Unable to fetch the attachments of the source message: %1.")
								.arg ("<em>" + QString::fromUtf8 (e.what ()) + "</em>");
					});
		}
	}

	void ComposeMessageTab::CopyAttachments (const MessageInfo& info)
	{
		const auto& attachments = info.Attachments_;
		if (attachments.isEmpty ())
			return;

		LinkedAttachmentsFetcher_ = std::make_shared<AttachmentsFetcher> (GetSelectedAccount (),
				info.Folder_, info.FolderId_, Util::Map (attachments, &AttDescr::GetName));
		Util::Sequence (this, LinkedAttachmentsFetcher_->GetFuture ()) >>
				Util::Visitor
				{
					[this] (const AttachmentsFetcher::FetchResult& result)
					{
						for (const auto& path : result.Paths_)
							AppendAttachment (path, {});

						const auto& notify = Util::MakeNotification ("Snails",
								tr ("Attached %n file(s) from the source message.",
										0,
										result.Paths_.size ()),
								Priority::Info);
						Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (notify);
					},
					[] (auto err)
					{
						const auto& text = AttachmentsFetchErrorDescr (err);
						const auto& notify = Util::MakeNotification ("Snails", text, Priority::Critical);
						Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (notify);
					}
				};
	}

	void ComposeMessageTab::SetupToolbar ()
	{
		QAction *send = new QAction (tr ("Send"), this);
		send->setProperty ("ActionIcon", "mail-send");
		connect (send,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSend ()));
		Toolbar_->addAction (send);

		AccountsMenu_ = new QMenu (tr ("Accounts"));
		auto accsGroup = new QActionGroup (this);
		for (const auto& account : AccsMgr_->GetAccounts ())
		{
			QAction *act = new QAction (account->GetConfig ().AccName_, this);
			accsGroup->addAction (act);
			act->setCheckable (true);
			act->setChecked (true);
			act->setProperty ("Account", QVariant::fromValue<Account_ptr> (account));

			AccountsMenu_->addAction (act);
		}

		auto accountsButton = new QToolButton (Toolbar_);
		accountsButton->setMenu (AccountsMenu_);
		accountsButton->setPopupMode (QToolButton::InstantPopup);
		accountsButton->setProperty ("ActionIcon", "system-users");
		Toolbar_->addWidget (accountsButton);

		AttachmentsMenu_ = new QMenu (tr ("Attachments"));
		AttachmentsMenu_->addSeparator ();
		QAction *add = AttachmentsMenu_->
				addAction (tr ("Add..."), this, SLOT (handleAddAttachment ()));
		add->setProperty ("ActionIcon", "list-add");

		auto attachmentsButton = new QToolButton (Toolbar_);
		attachmentsButton->setProperty ("ActionIcon", "mail-attachment");
		attachmentsButton->setMenu (AttachmentsMenu_);
		attachmentsButton->setPopupMode (QToolButton::InstantPopup);
		Toolbar_->addWidget (attachmentsButton);

		EditorsMenu_ = new QMenu (tr ("Editors"));

		auto editorsButton = new QToolButton (Toolbar_);
		editorsButton->setProperty ("ActionIcon", "story-editor");
		editorsButton->setMenu (EditorsMenu_);
		editorsButton->setPopupMode (QToolButton::InstantPopup);
		Toolbar_->addWidget (editorsButton);
	}

	void ComposeMessageTab::SetupEditors ()
	{
		Ui_.Editor_->SetupEditors ([this] (QAction *act) { EditorsMenu_->addAction (act); });

		connect (Ui_.Editor_,
				SIGNAL (editorChanged (IEditorWidget*, IEditorWidget*)),
				this,
				SLOT (handleEditorChanged (IEditorWidget*, IEditorWidget*)));
	}

	void ComposeMessageTab::SetMessageReferences (OutgoingMessage& message) const
	{
		if (OrigMessageId_.isEmpty ())
			return;

		message.InReplyTo_ = QList { OrigMessageId_ };

		auto references = OrigReferences_;
		while (references.size () > 20)
			references.removeAt (1);
		references << OrigMessageId_;
		message.References_ = references;
	}

	Account* ComposeMessageTab::GetSelectedAccount () const
	{
		for (auto act : AccountsMenu_->actions ())
			if (act->isChecked ())
				return act->property ("Account").value<Account_ptr> ().get ();

		return nullptr;
	}

	void ComposeMessageTab::AppendAttachment (const QString& path, const QString& descr)
	{
		const QFileInfo fi { path };

		const auto& filename = fi.fileName ();

		const auto& size = Util::MakePrettySize (fi.size ());
		const auto attAct = new QAction (QString { "%1 (%2)" }.arg (filename, size), this);
		attAct->setProperty ("Snails/AttachmentPath", path);
		attAct->setProperty ("Snails/Description", descr);

		const auto& mime = Util::MimeDetector {} (path);
		attAct->setIcon (Util::ExtensionsData::Instance ().GetMimeIcon (mime));

		connect (attAct,
				&QAction::triggered,
				[filename, attAct, this]
				{
					if (QMessageBox::question (this,
							"LeechCraft",
							tr ("Are you sure you want to remove attachment %1?")
								.arg (Util::FormatName (filename)),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
						return;

					attAct->deleteLater ();
					AttachmentsMenu_->removeAction (attAct);
				});

		const auto& acts = AttachmentsMenu_->actions ();
		AttachmentsMenu_->insertAction (acts.at (acts.size () - 2), attAct);
	}

	namespace
	{
		Addresses_t FromUserInput (const QString& text)
		{
			Addresses_t result;

			for (auto address : text.split (',', Qt::SkipEmptyParts))
			{
				address = address.trimmed ();

				QString name;

				const int idx = address.lastIndexOf (' ');
				if (idx > 0)
				{
					name = address.left (idx).trimmed ();
					address = address.mid (idx).simplified ();
				}

				if (address.startsWith ('<') &&
						address.endsWith ('>'))
				{
					address = address.mid (1);
					address.chop (1);
				}

				result.append ({ name, address });
			}

			return result;
		}
	}

	void ComposeMessageTab::AddAttachments (OutgoingMessage& message)
	{
		Util::MimeDetector detector;

		for (auto act : AttachmentsMenu_->actions ())
		{
			const auto& path = act->property ("Snails/AttachmentPath").toString ();
			if (path.isEmpty ())
				continue;

			const auto& descr = act->property ("Snails/Description").toString ();

			const auto& split = detector (path).split ('/');
			const auto& type = split.value (0);
			const auto& subtype = split.value (1);

			message.Attachments_ << AttDescr { path, descr, type, subtype, QFileInfo (path).size () };
		}
	}

	void ComposeMessageTab::Send (Account *account, const OutgoingMessage& message)
	{
		Util::Sequence (nullptr, account->SendMessage (message)) >>
				[safeThis = QPointer<ComposeMessageTab> { this }] (const auto& result)
				{
					Util::Visit (result.AsVariant (),
							[safeThis] (Util::Void) { if (safeThis) safeThis->Remove (); },
							[safeThis] (const auto& err)
							{
								Util::Visit (err,
										[safeThis] (const vmime::exceptions::authentication_error& err)
										{
											QMessageBox::critical (safeThis, "LeechCraft",
													tr ("Unable to send the message: authorization failure. Server reports: %1.")
														.arg ("<br/><em>" + QString::fromStdString (err.response ()) + "</em>"));
										},
										[] (const vmime::exceptions::connection_error&)
										{
											const auto& notify = Util::MakeNotification ("Snails",
													tr ("Unable to send email: operation timed out."),
													Priority::Critical);
											Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (notify);
										},
										[] (const auto& err)
										{
											qWarning () << Q_FUNC_INFO << "caught exception:" << err.what ();

											const auto& notify = Util::MakeNotification ("Snails",
													tr ("Unable to send email: %1.")
															.arg (QString::fromUtf8 (err.what ())),
													Priority::Critical);
											Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (notify);
										});
							});
				};
	}

	void ComposeMessageTab::handleSend ()
	{
		const auto account = GetSelectedAccount ();
		if (!account)
			return;

		const auto editor = Ui_.Editor_->GetCurrentEditor ();

		auto message = std::make_shared<OutgoingMessage> ();

		const auto& config = account->GetConfig ();
		message->From_ = Address { config.UserName_, config.UserEmail_ };

		message->To_ = FromUserInput (Ui_.To_->text ());
		message->Subject_ = Ui_.Subject_->text ();
		message->Body_ = editor->GetContents (ContentType::PlainText);
		message->HTMLBody_ = editor->GetContents (ContentType::HTML);

		SetMessageReferences (*message);

		if (!LinkedAttachmentsFetcher_)
		{
			AddAttachments (*message);
			Send (account, *message);
			return;
		}

		Util::Sequence (this, LinkedAttachmentsFetcher_->GetFuture ()) >>
				Util::Visitor
				{
					[=] (const AttachmentsFetcher::FetchResult&)
					{
						AddAttachments (*message);
						Send (account, *message);
					},
					[=] (auto err)
					{
						QMessageBox::critical (this, "LeechCraft", AttachmentsFetchErrorDescr (err));
					}
				};
	}

	void ComposeMessageTab::handleAddAttachment ()
	{
		const QString& path = QFileDialog::getOpenFileName (this,
				tr ("Select file to attach"),
				QDir::homePath ());
		if (path.isEmpty ())
			return;

		QFile file (path);
		if (!file.open (QIODevice::ReadOnly))
		{
			QMessageBox::critical (this,
					tr ("Error attaching file"),
					tr ("Error attaching file: %1 cannot be read.")
						.arg (Util::FormatName (path)));
			return;
		}

		const QString& descr = QInputDialog::getText (this,
				tr ("Attachment description"),
				tr ("Enter optional attachment description (you may leave it blank):"));

		AppendAttachment (path, descr);
	}

	void ComposeMessageTab::handleEditorChanged (IEditorWidget *newEditor, IEditorWidget *previous)
	{
		const auto& currentHtml = previous->GetContents (ContentType::HTML);
		const auto& currentPlain = previous->GetContents (ContentType::PlainText);

		newEditor->SetContents (currentPlain, ContentType::PlainText);
		if (!currentHtml.isEmpty ())
			SetupReplyRichContents (newEditor, currentHtml);
	}
}
}
