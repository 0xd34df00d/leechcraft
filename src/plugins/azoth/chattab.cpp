/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattab.h"
#include <cmath>
#include <QWebEngineSettings>
#include <QTextDocument>
#include <QBuffer>
#include <QPalette>
#include <QApplication>
#include <QShortcut>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTextBrowser>
#include <QDesktopWidget>
#include <QMimeData>
#include <QToolBar>
#include <QUrlQuery>
#include <util/xpc/defaulthookproxy.h>
#include <util/xpc/util.h>
#include <util/xsd/wkfontswidget.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/gui/geometry.h>
#include <util/gui/findnotificationwe.h>
#include <util/gui/fontsizescrollchanger.h>
#include <util/sll/lambdaeventfilter.h>
#include <util/sll/urloperator.h>
#include <util/sll/scopeguards.h>
#include <util/sll/visitor.h>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imessage.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/imucentry.h"
#include "interfaces/azoth/itransfermanager.h"
#include "interfaces/azoth/iconfigurablemuc.h"
#include "interfaces/azoth/ichatstyleresourcesource.h"
#include "interfaces/azoth/isupportmediacalls.h"
#include "interfaces/azoth/imediacall.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "interfaces/azoth/iupdatablechatentry.h"
#include "interfaces/azoth/iprovidecommands.h"
#ifdef ENABLE_CRYPT
#include "interfaces/azoth/isupportpgp.h"
#endif
#include "components/dialogs/bookmarksmanagerdialog.h"
#include "components/dialogs/userslistwidget.h"
#include "core.h"
#include "textedit.h"
#include "chattabsmanager.h"
#include "xmlsettingsmanager.h"
#include "transferjobmanager.h"
#include "callmanager.h"
#include "callchatwidget.h"
#include "msgformatterwidget.h"
#include "actionsmanager.h"
#include "contactdropfilter.h"
#include "util.h"
#include "proxyobject.h"
#include "customchatstylemanager.h"
#include "coremessage.h"
#include "dummymsgmanager.h"
#include "corecommandsmanager.h"
#include "resourcesmanager.h"
#include "msgeditautocompleter.h"
#include "msgsender.h"
#include "avatarsmanager.h"
#include "chattabpartstatemanager.h"

namespace LC
{
namespace Azoth
{
	QObject *ChatTab::S_ParentMultiTabs_ = 0;
	TabClassInfo ChatTab::S_ChatTabClass_;
	TabClassInfo ChatTab::S_MUCTabClass_;

	void ChatTab::SetParentMultiTabs (QObject *obj)
	{
		S_ParentMultiTabs_ = obj;
	}

	void ChatTab::SetChatTabClassInfo (const TabClassInfo& tc)
	{
		S_ChatTabClass_ = tc;
	}

	void ChatTab::SetMUCTabClassInfo (const TabClassInfo& tc)
	{
		S_MUCTabClass_ = tc;
	}

	const TabClassInfo& ChatTab::GetChatTabClassInfo ()
	{
		return S_ChatTabClass_;
	}

	const TabClassInfo& ChatTab::GetMUCTabClassInfo ()
	{
		return S_MUCTabClass_;
	}

	ChatTab::ChatTab (const QString& entryId,
			IAccount *account,
			AvatarsManager *am,
			Util::WkFontsWidget *fontsWidget,
			QWebEngineProfile *profile,
			QWidget *parent)
	: QWidget (parent)
	, AvatarsManager_ (am)
	, Account_ (account)
	, TabToolbar_ (new QToolBar (tr ("Azoth chat window"), this))
	, MUCEventLog_ (new QTextBrowser ())
	, EntryID_ (entryId)
	, NumUnreadMsgs_ (Core::Instance ().GetUnreadCount (GetEntry<ICLEntry> ()))
	, CDF_ (new ContactDropFilter (entryId, this))
	{
		Ui_.setupUi (this);
		Ui_.View_->InitializePage (profile);

		fontsWidget->RegisterSettable (this);

		Ui_.View_->setFocusProxy (Ui_.MsgEdit_);
		for (const auto child : Ui_.View_->findChildren<QWidget*> ())
			child->setFocusProxy (Ui_.MsgEdit_);

		Ui_.View_->installEventFilter (Util::MakeLambdaEventFilter<QEvent::ChildAdded> ([this] (QChildEvent *e)
				{
					if (const auto w = qobject_cast<QWidget*> (e->child ()))
						w->setFocusProxy (Ui_.MsgEdit_);

					return false;
				},
				*this));

		const auto settings = Ui_.View_->settings ();
		Util::InstallFontSizeChanger (*Ui_.View_,
				{
					.GetViewFontSize_ = [settings] { return settings->fontSize (QWebEngineSettings::DefaultFontSize); },
					.SetViewFontSize_ = [settings] (int newFontSize)
							{
								settings->setFontSize (QWebEngineSettings::DefaultFontSize, newFontSize);
								settings->setFontSize (QWebEngineSettings::DefaultFixedFontSize, newFontSize);
								settings->setFontSize (QWebEngineSettings::MinimumFontSize, newFontSize);
							},
					.SetDefaultFontSize_ = [fontsWidget] (int newFontSize)
							{
								fontsWidget->SetSize (FontSize::DefaultFontSize, newFontSize);
								fontsWidget->SetSize (FontSize::DefaultFixedFontSize, newFontSize);
								fontsWidget->SetSize (FontSize::MinimumFontSize, newFontSize);
							},
				});

		Ui_.MsgEdit_->installEventFilter (Util::MakeLambdaEventFilter<QEvent::KeyRelease> ([this] (QKeyEvent *ev)
				{
					if (ev->matches (QKeySequence::Copy) &&
						!Ui_.View_->page ()->selectedText ().isEmpty ())
					{
						Ui_.View_->pageAction (QWebEnginePage::Copy)->trigger ();
						return true;
					}

					return false;
				},
				*this));

		MUCEventLog_->installEventFilter (this);

		Ui_.View_->installEventFilter (CDF_);
		Ui_.MsgEdit_->installEventFilter (CDF_);

		Ui_.SubjBox_->setVisible (false);
		Ui_.SubjChange_->setEnabled (false);

		Ui_.EventsButton_->setMenu (new QMenu (tr ("Events"), this));
		Ui_.EventsButton_->hide ();

		Ui_.SendButton_->setIcon (Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon ("key-enter"));
		connect (Ui_.SendButton_,
				SIGNAL (released ()),
				this,
				SLOT (messageSend ()));

		ChatFinder_ = new Util::FindNotificationWE (Core::Instance ().GetProxy (), Ui_.View_);
		ChatFinder_->hide ();

		BuildBasicActions ();

		Core::Instance ().RegisterHookable (this);

		connect (Core::Instance ().GetTransferJobManager (),
				SIGNAL (jobNoLongerOffered (QObject*)),
				this,
				SLOT (handleFileNoLongerOffered (QObject*)));

		QSize ccSize = Ui_.CharCounter_->size ();
		ccSize.setWidth (fontMetrics ().horizontalAdvance (" 9999"));
		Ui_.CharCounter_->resize (ccSize);

		connect (Ui_.View_,
				SIGNAL (linkClicked (QUrl, bool)),
				this,
				SLOT (handleViewLinkClicked (QUrl, bool)));
		connect (Ui_.View_,
				SIGNAL (chatWindowSearchRequested (QString)),
				this,
				SLOT (handleChatWindowSearch (QString)));

		DummyMsgManager::Instance ().ClearMessages (GetCLEntry ());
		PrepareTheme ();

		auto entry = GetEntry<ICLEntry> ();
		const int autoNum = XmlSettingsManager::Instance ()
				.property ("ShowLastNMessages").toInt ();
		if (entry->GetAllMessages ().size () <= 100 &&
				entry->GetEntryType () != ICLEntry::EntryType::MUC &&
				autoNum)
			RequestLogs (autoNum);

		ReinitEntry ();
		CheckMUC ();
		InitExtraActions ();
		InitMsgEdit ();
		RegisterSettings ();

		emit hookChatTabCreated (std::make_shared<Util::DefaultHookProxy> (),
				this,
				GetEntry<QObject> (),
				Ui_.View_);

		HandleMUCParticipantsChanged ();

		connect (Core::Instance ().GetCustomChatStyleManager (),
				SIGNAL (accountStyleChanged (IAccount*)),
				this,
				SLOT (handleAccountStyleChanged (IAccount*)));

		if (!IsMUC_)
			new ChatTabPartStateManager { this };

		connect (Ui_.VariantBox_,
				qOverload<const QString&> (&QComboBox::currentTextChanged),
				this,
				&ChatTab::currentVariantChanged);
	}

	ChatTab::~ChatTab ()
	{
		Ui_.View_->setFocusProxy (nullptr);

		if (auto entry = GetEntry<ICLEntry> ())
			entry->ChatTabClosed ();

		qDeleteAll (HistoryMessages_);
		qDeleteAll (CoreMessages_);
		DummyMsgManager::Instance ().ClearMessages (GetCLEntry ());
		delete Ui_.MsgEdit_->document ();

		delete MUCEventLog_;
	}

	void ChatTab::PrepareTheme ()
	{
		const auto entry = GetEntry<QObject> ();
		auto data = Core::Instance ().GetSelectedChatTemplate (entry, Ui_.View_->page ());
		if (data.isEmpty ())
			data = QString (R"delim(
				<?xml version="1.0" encoding="utf-8"?>
				<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
				<html xmlns="http://www.w3.org/1999/xhtml">
					<head>
						<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
						<title></title>
					</head>
					<body>
						<h1 style="color:red">%1</h1>
					</body>
				</html>)delim")
					.arg (tr ("Unable to load style, please check you've enabled at least one styles plugin."));

		Ui_.View_->setContent (data.toUtf8 (),
				"text/html", //"application/xhtml+xml" fails to work, though better to use it
				Core::Instance ().GetSelectedChatTemplateURL (entry));
	}

	void ChatTab::HasBeenAdded ()
	{
		UpdateStateIcon ();
	}

	TabClassInfo ChatTab::GetTabClassInfo () const
	{
		return IsMUC_ ? S_MUCTabClass_ : S_ChatTabClass_;
	}

	QList<QAction*> ChatTab::GetTabBarContextMenuActions () const
	{
		const auto mgr = Core::Instance ().GetActionsManager ();
		return Util::Filter (mgr->GetEntryActions (GetEntry<ICLEntry> ()),
				[mgr] (QAction *act)
				{
					return act->isSeparator () ||
							mgr->GetAreasForAction (act)
									.contains (ActionsManager::CLEAATabCtxtMenu);
				});
	}

	QObject* ChatTab::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	QToolBar* ChatTab::GetToolBar () const
	{
		return TabToolbar_.get ();
	}

	void ChatTab::Remove ()
	{
		if (IsCurrent_)
			emit entryLostCurrent (GetEntry<QObject> ());
		emit needToClose ();
	}

	void ChatTab::TabMadeCurrent ()
	{
		Core::Instance ().GetChatTabsManager ()->ChatMadeCurrent (this);
		Core::Instance ().FrameFocused (GetEntry<QObject> (), Ui_.View_->page ());

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookMadeCurrent (proxy, this);
		if (proxy->IsCancelled ())
			return;

		IsCurrent_ = true;

		if (auto entry = GetEntry<QObject> ())
			emit entryMadeCurrent (entry);

		NumUnreadMsgs_ = 0;
		HadHighlight_ = false;

		ReformatTitle ();
		Ui_.MsgEdit_->setFocus ();
	}

	void ChatTab::TabLostCurrent ()
	{
		emit entryLostCurrent (GetEntry<QObject> ());

		IsCurrent_ = false;

		MsgFormatter_->HidePopups ();
	}

	QByteArray ChatTab::GetTabRecoverData () const
	{
		QByteArray result;
		auto entry = GetEntry<ICLEntry> ();
		if (!entry)
			return result;

		QDataStream stream (&result, QIODevice::WriteOnly);
		if (entry->GetEntryType () == ICLEntry::EntryType::MUC &&
				GetEntry<IMUCEntry> ())
			stream << QByteArray ("muctab2")
					<< entry->GetEntryID ()
					<< GetEntry<IMUCEntry> ()->GetIdentifyingData ()
					<< entry->GetParentAccount ()->GetAccountID ();
		else
			stream << QByteArray ("chattab2")
					<< entry->GetEntryID ()
					<< GetSelectedVariant ();

		stream << Ui_.MsgEdit_->toPlainText ();

		return result;
	}

	QString ChatTab::GetTabRecoverName () const
	{
		auto entry = GetEntry<ICLEntry> ();
		return entry ?
				tr ("Chat with %1.")
					.arg (entry->GetEntryName ()) :
				GetTabClassInfo ().VisibleName_;
	}

	QIcon ChatTab::GetTabRecoverIcon () const
	{
		return LastAvatar_.isNull () ?
				GetTabClassInfo ().Icon_ :
				QPixmap::fromImage (LastAvatar_);
	}

	void ChatTab::FillMimeData (QMimeData *data)
	{
		if (auto entry = GetEntry<ICLEntry> ())
		{
			const auto& id = entry->GetHumanReadableID ();
			data->setText (id);
			data->setUrls ({ id });
		}
	}

	void ChatTab::HandleDragEnter (QDragMoveEvent *event)
	{
		auto data = event->mimeData ();
		if (data->hasText ())
			event->acceptProposedAction ();
		else if (data->hasUrls ())
		{
			for (const auto& url : data->urls ())
				if (url.isLocalFile () &&
						QFile::exists (url.toLocalFile ()))
				{
					event->acceptProposedAction ();
					break;
				}
		}
	}

	void ChatTab::HandleDrop (QDropEvent *event)
	{
		const auto data = event->mimeData ();
		if (data->hasUrls () || data->hasImage ())
			CDF_->HandleDrop (data);
		else if (data->hasText ())
			appendMessageText (data->text ());
	}

	QObject* ChatTab::GetQObject ()
	{
		return this;
	}

	void ChatTab::SetFontFamily (FontFamily family, const QFont& font)
	{
		Ui_.View_->settings ()->setFontFamily (static_cast<QWebEngineSettings::FontFamily> (family), font.family ());
	}

	void ChatTab::SetFontSize (FontSize type, int size)
	{
		Ui_.View_->settings ()->setFontSize (static_cast<QWebEngineSettings::FontSize> (type), size);
	}

	void ChatTab::ShowUsersList ()
	{
		IMUCEntry *muc = GetEntry<IMUCEntry> ();
		if (!muc)
			return;

		const auto& parts = muc->GetParticipants ();
		UsersListWidget w (parts, [] (ICLEntry *entry) { return entry->GetEntryName (); }, this);
		if (w.exec () != QDialog::Accepted)
			return;

		if (auto part = w.GetActivatedParticipant ())
			InsertNick (qobject_cast<ICLEntry*> (part)->GetEntryName ());
	}

	void ChatTab::HandleMUCParticipantsChanged ()
	{
		IMUCEntry *muc = GetEntry<IMUCEntry> ();
		if (!muc)
			return;

		const auto entry = GetEntry<ICLEntry> ();

		const int parts = muc->GetParticipants ().size ();
		QString text = entry->GetEntryName ();
		if (entry->GetHumanReadableID () != text)
			text += " (" + entry->GetHumanReadableID () + ")";
		text += ' ' + tr ("[%n participant(s)]", 0, parts);
		Ui_.EntryInfo_->setText (text);
	}

	void ChatTab::SetEnabled (bool enabled)
	{
		auto children = findChildren<QWidget*> ();
		children += TabToolbar_.get ();
		children += MUCEventLog_;
		children += MsgFormatter_;
		for (auto child : children)
			if (child != Ui_.View_)
				child->setEnabled (enabled);

		if (enabled)
			AddManagedActions (false);
	}

	QObject* ChatTab::GetCLEntry () const
	{
		return GetEntry<QObject> ();
	}

	QString ChatTab::GetEntryID () const
	{
		return EntryID_;
	}

	QString ChatTab::GetSelectedVariant () const
	{
		return Ui_.VariantBox_->currentText ();
	}

	bool ChatTab::eventFilter (QObject *obj, QEvent *event)
	{
		if (obj == MUCEventLog_ && event->type () == QEvent::Close)
			Ui_.MUCEventsButton_->setChecked (false);

		return false;
	}

	namespace
	{
		bool TextMatchesCmd (const QString& text, const QString& cmd)
		{
			if (text == cmd)
				return true;

			if (!text.startsWith (cmd))
				return false;

			return text [cmd.size ()].isSpace ();
		}

		/** Processes the outgoing messages, replacing /nick with calls
		 * to the entity to change nick, for example, etc.
		 *
		 * If this function returns true, processing (and sending) the
		 * message should be aborted.
		 *
		 * @return true if the processing should be aborted, false
		 * otherwise.
		 */
		bool ProcessOutgoingMsg (ICLEntry *entry, QString& text)
		{
			auto cmdProvs = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<IProvideCommands*> ();
			cmdProvs << Core::Instance ().GetCoreCommandsManager ();
			for (const auto prov : cmdProvs)
				for (const auto& cmd : prov->GetStaticCommands (entry))
				{
					if (!std::any_of (cmd.Names_.begin (), cmd.Names_.end (),
							[&text] (const QString& name) { return TextMatchesCmd (text, name); }))
						continue;

					const auto entryObj = entry->GetQObject ();
					try
					{
						const auto handled = Util::Visit (cmd.Command_ (entry, text),
								[] (bool res) { return res; },
								[&text] (const TextMorphResult& result)
								{
									text = result.NewText_;
									return false;
								},
								[entryObj] (const StringCommandResult& result)
								{
									const auto msg = new CoreMessage
									{
										result.Message_,
										QDateTime::currentDateTime (),
										IMessage::Type::ServiceMessage,
										IMessage::Direction::In,
										entryObj,
										entryObj
									};
									msg->Store ();
									return result.StopProcessing_;
								});
						if (handled)
							return true;
					}
					catch (const CommandException& ex)
					{
						auto body = ChatTab::tr ("Cannot execute %1.")
								.arg ("<em>" + text + "</em>");
						body += " " + ex.GetError ();

						const auto msg = new CoreMessage
						{
							body,
							QDateTime::currentDateTime (),
							IMessage::Type::ServiceMessage,
							IMessage::Direction::In,
							entryObj,
							entryObj
						};
						msg->Store ();

						if (!ex.CanTryOtherCommands ())
							return true;
					}
				}

			return false;
		}
	}

	void ChatTab::messageSend ()
	{
		QString text = Ui_.MsgEdit_->toPlainText ();
		if (text.isEmpty ())
			return;

		const auto& richText = ToggleRichEditor_->isChecked () ?
				MsgFormatter_->GetNormalizedRichText () :
				QString {};

		bool clear = true;
		auto clearGuard = Util::MakeScopeGuard ([&clear, &text, this]
				{
					if (!clear)
						return;

					Ui_.MsgEdit_->clear ();
					Ui_.MsgEdit_->document ()->clear ();
					MsgFormatter_->Clear ();
					CurrentHistoryPosition_ = -1;
					MsgHistory_.prepend (text);
				});

		auto variant = Ui_.VariantBox_->count () > 1 ?
				Ui_.VariantBox_->currentText () :
				QString ();

		const auto e = GetEntry<ICLEntry> ();

		auto type = e->GetEntryType () == ICLEntry::EntryType::MUC ?
						IMessage::Type::MUCMessage :
						IMessage::Type::ChatMessage;

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		proxy->SetValue ("text", text);
		emit hookMessageSendRequested (proxy, this, e->GetQObject (), static_cast<int> (type), variant);

		if (proxy->IsCancelled ())
		{
			if (proxy->GetValue ("PreserveMessageEdit").toBool ())
				clear = false;
			return;
		}

		proxy->FillValue ("text", text);

		if (ProcessOutgoingMsg (e, text))
			return;

		try
		{
			new MsgSender { e, type, text, variant, richText };
		}
		catch (const std::exception& ex)
		{
			clear = false;

			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Error sending message to %1: %2.")
						.arg ("<em>" + e->GetEntryName () + "</em>")
						.arg (QString::fromUtf8 (ex.what ())));
		}
	}

	void ChatTab::on_MsgEdit__textChanged ()
	{
		UpdateTextHeight ();
		emit composingTextChanged (Ui_.MsgEdit_->toPlainText ());
		emit tabRecoverDataChanged ();
	}

	void ChatTab::on_SubjectButton__toggled (bool show)
	{
		Ui_.SubjBox_->setVisible (show);
		Ui_.SubjChange_->setEnabled (show);

		if (!show)
			return;

		IMUCEntry *me = GetEntry<IMUCEntry> ();
		if (!me)
			return;

		Ui_.SubjEdit_->setReadOnly (!me->CanChangeSubject ());
		Ui_.SubjEdit_->setText (me->GetMUCSubject ());
	}

	void ChatTab::on_SubjChange__released ()
	{
		Ui_.SubjectButton_->setChecked (false);

		IMUCEntry *me = GetEntry<IMUCEntry> ();
		if (!me)
			return;

		me->SetMUCSubject (Ui_.SubjEdit_->toPlainText ());
	}

	void ChatTab::on_View__loadFinished (bool ok)
	{
		if (!ok)
			return;

		emit hookThemeReloaded (std::make_shared<Util::DefaultHookProxy> (),
				this, Ui_.View_, GetEntry<QObject> ());

		for (const auto msg : HistoryMessages_)
			AppendMessage (msg);

		ICLEntry *e = GetEntry<ICLEntry> ();
		if (!e)
		{
			qWarning () << Q_FUNC_INFO
					<< "null entry";
			return;
		}

		auto messages = e->GetAllMessages ();

		const auto& dummyMsgs = DummyMsgManager::Instance ().GetIMessages (e->GetQObject ());
		if (!dummyMsgs.isEmpty ())
		{
			messages += dummyMsgs;
			std::sort (messages.begin (), messages.end (), Util::ComparingBy (&IMessage::GetDateTime));
		}

		for (const auto msg : messages)
			AppendMessage (msg);

		QFile scrollerJS (":/plugins/azoth/resources/scripts/scrollers.js");
		if (!scrollerJS.open (QIODevice::ReadOnly))
			qWarning () << Q_FUNC_INFO
					<< "unable to open script file"
					<< scrollerJS.errorString ();
		else
			Ui_.View_->page ()->runJavaScript (scrollerJS.readAll ());
	}

#ifdef ENABLE_MEDIACALLS
	void ChatTab::handleCallRequested ()
	{
		const auto& variant = Ui_.VariantBox_->currentText ();
		Core::Instance ().GetCallManager ()->Call (GetEntry<ICLEntry> (), variant);
	}

	void ChatTab::handleCall (QObject *callObj)
	{
		const auto call = qobject_cast<IMediaCall*> (callObj);
		if (!call)
		{
			qWarning () << Q_FUNC_INFO
					<< "null call object from"
					<< sender ();
			return;
		}

		if (call->GetSourceID () != EntryID_)
			return;

		CallChatWidget *widget = new CallChatWidget (callObj);
		const int idx = Ui_.MainLayout_->indexOf (Ui_.View_);
		Ui_.MainLayout_->insertWidget (idx, widget);
	}
#endif

#ifdef ENABLE_CRYPT
	void ChatTab::handleEnableEncryption ()
	{
		const auto accObj = GetEntry<ICLEntry> ()->GetParentAccount ()->GetQObject ();
		const auto pgp = qobject_cast<ISupportPGP*> (accObj);
		if (!pgp)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "doesn't implement ISupportPGP";
			return;
		}

		const bool enable = EnableEncryption_->isChecked ();
		SetEncryptionEnabled (pgp, enable);
	}

	void ChatTab::handleEncryptionStateChanged (QObject *entry, bool enabled)
	{
		if (entry != GetEntry<QObject> ())
			return;

		EnableEncryption_->setChecked (enabled);
	}
#endif

	void ChatTab::clearChat ()
	{
		ICLEntry *entry = GetEntry<ICLEntry> ();
		if (!entry)
			return;

		ScrollbackPos_ = 0;

		const auto grace = XmlSettingsManager::Instance ()
				.property ("ChatClearGraceTime").toInt ();
		const auto& upTo = grace ? QDateTime::currentDateTime ().addSecs (-grace) : QDateTime {};
		entry->PurgeMessages (upTo);

		qDeleteAll (HistoryMessages_);
		HistoryMessages_.clear ();
		qDeleteAll (CoreMessages_);
		CoreMessages_.clear ();
		DummyMsgManager::Instance ().ClearMessages (GetCLEntry ());
		LastDateTime_ = QDateTime ();
		PrepareTheme ();
	}

	void ChatTab::handleHistoryBack ()
	{
		ScrollbackPos_ += 50;
		qDeleteAll (HistoryMessages_);
		HistoryMessages_.clear ();
		qDeleteAll (CoreMessages_);
		CoreMessages_.clear ();
		DummyMsgManager::Instance ().ClearMessages (GetCLEntry ());
		LastDateTime_ = QDateTime ();
		RequestLogs (ScrollbackPos_);
	}

	namespace
	{
		void UpdateSettingWithDefaultValue (bool currentValue,
				const QString& entryId,
				const QString& groupName,
				const QByteArray& propertyName)
		{
			const bool defaultSetting = XmlSettingsManager::Instance ().property (propertyName).toBool ();

			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Azoth");
			settings.beginGroup (groupName);

			auto enabled = settings.value ("Enabled").toStringList ();
			auto disabled = settings.value ("Disabled").toStringList ();

			if (currentValue == defaultSetting)
			{
				enabled.removeAll (entryId);
				disabled.removeAll (entryId);
			}
			else if (defaultSetting)
				disabled << entryId;
			else
				enabled << entryId;

			settings.setValue ("Enabled", enabled);
			settings.setValue ("Disabled", disabled);

			settings.endGroup ();
		}
	}

	void ChatTab::handleRichEditorToggled ()
	{
		UpdateSettingWithDefaultValue (ToggleRichEditor_->isChecked (),
				EntryID_, "RichEditorStates", "ShowRichTextEditor");
	}

	void ChatTab::handleRichTextToggled ()
	{
		PrepareTheme ();
		UpdateSettingWithDefaultValue (ToggleRichText_->isChecked (),
				EntryID_, "RichTextStates", "ShowRichTextMessageBody");
	}

	void ChatTab::handleQuoteSelection ()
	{
		const QString& selected = Ui_.View_->selectedText ();
		if (selected.isEmpty ())
			return;

		auto split = selected.split ('\n');
		for (auto& item : split)
			item.prepend ("> ");
		split.push_back ({});

		const auto& toInsert = split.join ("\n");
		Ui_.MsgEdit_->textCursor ().insertText (toInsert);
	}

	void ChatTab::handleOpenLastLink ()
	{
		if (LastLink_.isEmpty ())
			return;

		const auto& e = Util::MakeEntity (QUrl (LastLink_), {}, FromUserInitiated | OnlyHandle);
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}

	void ChatTab::handleFileOffered (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}

		if (job->GetSourceID () != EntryID_)
			return;

		Ui_.EventsButton_->show ();

		const QString& text = tr ("File offered: %1.")
				.arg (job->GetName ());
		QAction *act = Ui_.EventsButton_->menu ()->
				addAction (text, this, SLOT (handleOfferActionTriggered ()));
		act->setData (QVariant::fromValue<QObject*> (jobObj));
		act->setToolTip (job->GetComment ());
	}

	void ChatTab::handleFileNoLongerOffered (QObject *jobObj)
	{
		for (const auto action : Ui_.EventsButton_->menu ()->actions ())
			if (action->data ().value<QObject*> () == jobObj)
			{
				action->deleteLater ();
				break;
			}

		if (Ui_.EventsButton_->menu ()->actions ().count () == 1)
			Ui_.EventsButton_->hide ();
	}

	void ChatTab::handleOfferActionTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		QObject *jobObj = action->data ().value<QObject*> ();
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);

		QString text = tr ("Would you like to accept or reject file transfer "
				"request for file %1?")
					.arg (job->GetName ());
		if (!job->GetComment ().isEmpty ())
		{
			text += "<br /><br />" + tr ("The file description is:") + "<br /><br /><em>";
			auto comment = job->GetComment ().toHtmlEscaped ();
			comment.replace ("\n", "<br />");
			text += comment + "</em>";
		}

		auto questResult = QMessageBox::question (this,
				tr ("File transfer request"), text,
				QMessageBox::Save | QMessageBox::Abort | QMessageBox::Cancel);

		if (questResult == QMessageBox::Cancel)
			return;
		else if (questResult == QMessageBox::Abort)
			Core::Instance ().GetTransferJobManager ()->DenyJob (jobObj);
		else
		{
			const QString& path = QFileDialog::getExistingDirectory (this,
					tr ("Select save path for incoming file"),
					XmlSettingsManager::Instance ()
							.property ("DefaultXferSavePath").toString ());
			if (path.isEmpty ())
				return;

			Core::Instance ().GetTransferJobManager ()->AcceptJob (jobObj, path);
		}

		action->deleteLater ();

		if (Ui_.EventsButton_->menu ()->actions ().size () == 1)
			Ui_.EventsButton_->hide ();
	}

	void ChatTab::handleEntryMessage (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage"
					<< sender ();
			return;
		}

		auto entry = GetEntry<ICLEntry> ();

		const bool isActiveChat = Core::Instance ().GetChatTabsManager ()->IsActiveChat (entry);

		bool shouldReformat = false;
		if (Core::Instance ().ShouldCountUnread (entry, msg))
		{
			++NumUnreadMsgs_;
			shouldReformat = true;
		}
		else if (isActiveChat)
			GetEntry<ICLEntry> ()->MarkMsgsRead ();

		if (msg->GetMessageType () == IMessage::Type::MUCMessage &&
				!isActiveChat &&
				!HadHighlight_)
		{
			HadHighlight_ = Core::Instance ().IsHighlightMessage (msg);
			if (HadHighlight_)
				shouldReformat = true;
		}

		if (shouldReformat)
			ReformatTitle ();

		if (msg->GetMessageType () == IMessage::Type::ChatMessage &&
				msg->GetDirection () == IMessage::Direction::In)
		{
			const int idx = Ui_.VariantBox_->findText (msg->GetOtherVariant ());
			if (idx != -1)
				Ui_.VariantBox_->setCurrentIndex (idx);
		}

		AppendMessage (msg);
	}

	void ChatTab::handleVariantsChanged (QStringList variants)
	{
		if (!variants.isEmpty () &&
				!variants.contains (QString ()))
			variants.prepend (QString ());

		if (variants.size () == Ui_.VariantBox_->count ())
		{
			bool samelist = true;
			for (int i = 0, size = variants.size (); i < size; ++i)
				if (variants.at (i) != Ui_.VariantBox_->itemText (i))
				{
					samelist = false;
					break;
				}

			if (samelist)
				return;
		}

		const QString& current = Ui_.VariantBox_->currentText ();
		Ui_.VariantBox_->clear ();

		for (const auto& variant : variants)
		{
			const State& st = GetEntry<ICLEntry> ()->GetStatus (variant).State_;
			const QIcon& icon = ResourcesManager::Instance ().GetIconForState (st);
			Ui_.VariantBox_->addItem (icon, variant);
		}

		if (!variants.isEmpty ())
		{
			const int pos = std::max (0, Ui_.VariantBox_->findText (current));
			Ui_.VariantBox_->setCurrentIndex (pos);
		}

		Ui_.VariantBox_->setVisible (variants.size () > 1);

		if (variants.isEmpty ())
			handleStatusChanged (EntryStatus (), QString ());
	}

	void ChatTab::handleNameChanged (const QString&)
	{
		ReformatTitle ();
	}

	void ChatTab::handleStatusChanged (const EntryStatus& status,
			const QString& variant)
	{
		auto entry = GetEntry<ICLEntry> ();
		if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
			return;

		const QStringList& vars = entry->Variants ();
		handleVariantsChanged (vars);

		if (vars.value (0) == variant ||
				variant.isEmpty () ||
				vars.isEmpty ())
		{
			const QIcon& icon = ResourcesManager::Instance ().GetIconForState (status.State_);
			TabIcon_ = icon;
			UpdateStateIcon ();
		}
	}

	void ChatTab::handleChatPartStateChanged (const ChatPartState& state, const QString&)
	{
		auto entry = GetEntry<ICLEntry> ();
		QString text = entry->GetEntryName ();
		if (entry->GetHumanReadableID () != text)
			text += " (" + entry->GetHumanReadableID () + ")";

		QString chatState;
		switch (state)
		{
		case CPSActive:
			chatState = tr ("participating");
			break;
		case CPSInactive:
			chatState = tr ("inactive");
			break;
		case CPSComposing:
			chatState = tr ("composing");
			break;
		case CPSPaused:
			chatState = tr ("paused composing");
			break;
		case CPSGone:
			chatState = tr ("left the conversation");
			break;
		default:
			break;
		}
		if (!chatState.isEmpty ())
			text += " (" + chatState + ")";

		Ui_.EntryInfo_->setText (text);
	}

	namespace
	{
		void OpenChatWithText (QUrl newUrl, const QString& id, ICLEntry *own)
		{
			Util::UrlOperator { newUrl } -= "hrid";
			for (QObject *entryObj : own->GetParentAccount ()->GetCLEntries ())
			{
				ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry || entry->GetHumanReadableID () != id)
					continue;

				auto w = Core::Instance ().GetChatTabsManager ()->OpenChat (entry, true);
				QMetaObject::invokeMethod (w,
						"handleViewLinkClicked",
						Qt::QueuedConnection,
						Q_ARG (QUrl, newUrl));
				break;
			}
		}
	}

	void ChatTab::handleViewLinkClicked (QUrl url, bool raise)
	{
		if (url.scheme () != "azoth")
		{
			Core::Instance ().HandleURLGeneric (url, raise, GetEntry<ICLEntry> ());
			return;
		}

		const auto& host = url.host ();
		if (host == "msgeditreplace")
		{
			const auto& queryItems = QUrlQuery { url }.queryItems ();
			if (queryItems.isEmpty ())
			{
				Ui_.MsgEdit_->setText (url.path ().mid (1));
				Ui_.MsgEdit_->moveCursor (QTextCursor::End);
				Ui_.MsgEdit_->setFocus ();
			}
			else
				for (const auto& item : queryItems)
					if (item.first == "hrid")
					{
						OpenChatWithText (url, item.second, GetEntry<ICLEntry> ());
						return;
					}
		}
		else if (host == "msgeditinsert")
		{
			const auto& text = url.path ().mid (1);
			const auto& split = text.split ("/#/", Qt::SkipEmptyParts);

			const auto& insertText = split.value (0);
			const auto& replaceText = split.size () > 1 ?
					split.value (1) :
					insertText;

			if (Ui_.MsgEdit_->toPlainText ().simplified ().trimmed ().isEmpty ())
			{
				Ui_.MsgEdit_->setText (replaceText);
				Ui_.MsgEdit_->moveCursor (QTextCursor::End);
				Ui_.MsgEdit_->setFocus ();
			}
			else
				Ui_.MsgEdit_->textCursor ().insertText (insertText);
		}
		else if (host == "insertnick")
		{
			const auto& nick = QUrlQuery { url }.queryItemValue ("nick", QUrl::FullyDecoded);
			InsertNick (nick);

			if (!GetMucParticipants (EntryID_).contains (nick))
				Core::Instance ().SendEntity (Util::MakeNotification ("Azoth",
							tr ("%1 isn't present in this conference at the moment.")
								.arg ("<em>" + nick + "</em>"),
							Priority::Warning));
		}
		else if (host == "sendentities")
		{
			const QUrlQuery queryObject { url };
			const auto& count = std::max (queryObject.queryItemValue ("count").toInt (), 1);
			for (int i = 0; i < count; ++i)
			{
				const auto& numStr = QString::number (i);

				const auto& entityStr = queryObject.queryItemValue ("entityVar" + numStr);
				const auto& type = queryObject.queryItemValue ("entityType" + numStr);

				QVariant entityVar;
				if (type == "url")
					entityVar = QUrl::fromEncoded (entityStr.toUtf8 ());
				else
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown entity type"
							<< type;
					continue;
				}

				const auto& mime = queryObject.queryItemValue ("mime" + numStr);

				const auto& flags = queryObject.queryItemValue ("flags" + numStr).split (",");
				TaskParameters tp = TaskParameter::FromUserInitiated;
				if (flags.contains ("OnlyHandle"))
					tp |= TaskParameter::OnlyHandle;
				else if (flags.contains ("OnlyDownload"))
					tp |= TaskParameter::OnlyDownload;

				auto e = Util::MakeEntity (entityVar, {}, tp, mime);

				const auto& addCountStr = queryObject.queryItemValue ("addCount" + numStr);
				for (int j = 0, cnt = std::max (addCountStr.toInt (), 1); j < cnt; ++j)
				{
					const auto& addStr = QString::number (j);
					const auto& key = queryObject.queryItemValue ("add" + numStr + "key" + addStr);
					const auto& value = queryObject.queryItemValue ("add" + numStr + "value" + addStr);
					e.Additional_ [key] = value;
				}

				Core::Instance ().SendEntity (e);
			}
		}
	}

	void ChatTab::InsertNick (const QString& nicknameHtml)
	{
		const QString& post = XmlSettingsManager::Instance ()
				.property ("PostAddressText").toString ();
		QTextCursor cursor = Ui_.MsgEdit_->textCursor ();
		cursor.insertHtml (cursor.atStart () ?
				nicknameHtml + post + " " :
				" &nbsp;" + nicknameHtml + " ");
		Ui_.MsgEdit_->setFocus ();
	}

	void ChatTab::handleHistoryUp ()
	{
		if (CurrentHistoryPosition_ == MsgHistory_.size () - 1)
			return;

		Ui_.MsgEdit_->setText (MsgHistory_
					.at (++CurrentHistoryPosition_));
	}

	void ChatTab::handleHistoryDown ()
	{
		if (CurrentHistoryPosition_ == -1)
			return;

		if (CurrentHistoryPosition_-- == 0)
			Ui_.MsgEdit_->clear ();
		else
			Ui_.MsgEdit_->setText (MsgHistory_
						.at (CurrentHistoryPosition_));

		Ui_.MsgEdit_->moveCursor (QTextCursor::End);
	}

	void ChatTab::handleGotLastMessages (QObject *entryObj, const QList<QObject*>& messages)
	{
		if (entryObj != GetEntry<QObject> ())
			return;

		const auto entry = GetEntry<ICLEntry> ();
		auto rMsgs = entry->GetAllMessages ();
		std::reverse (rMsgs.begin (), rMsgs.end ());

		for (const auto msgObj : messages)
		{
			const auto msg = qobject_cast<IMessage*> (msgObj);
			const auto& dt = msg->GetDateTime ();

			if (std::any_of (rMsgs.begin (), rMsgs.end (),
					[msg] (IMessage *tMsg)
					{
						return tMsg->GetDirection () == msg->GetDirection () &&
								tMsg->GetBody () == msg->GetBody () &&
								std::abs (tMsg->GetDateTime ().secsTo (msg->GetDateTime ())) < 5;
					}))
				continue;

			if (HistoryMessages_.isEmpty () ||
					HistoryMessages_.last ()->GetDateTime () <= dt)
				HistoryMessages_ << msg;
			else
			{
				auto pos = std::find_if (HistoryMessages_.begin (), HistoryMessages_.end (),
						[dt] (IMessage *msg) { return msg->GetDateTime () > dt; });
				HistoryMessages_.insert (pos, msg);
			}
		}

		if (!messages.isEmpty ())
			PrepareTheme ();

		disconnect (sender (),
				SIGNAL (gotLastMessages (QObject*, const QList<QObject*>&)),
				this,
				SLOT (handleGotLastMessages (QObject*, const QList<QObject*>&)));
	}

	void ChatTab::handleSendButtonVisible ()
	{
		Ui_.SendButton_->setVisible (XmlSettingsManager::Instance ()
					.property ("SendButtonVisible").toBool ());
	}

	void ChatTab::handleMinLinesHeightChanged ()
	{
		PreviousTextHeight_ = 0;
		UpdateTextHeight ();
	}

	void ChatTab::handleRichFormatterPosition ()
	{
		const QString& posStr = XmlSettingsManager::Instance ()
				.property ("RichFormatterPosition").toString ();
		const int pos = Ui_.MainLayout_->indexOf (Ui_.View_) + (posStr == "belowEdit" ? 2 : 1);
		Ui_.MainLayout_->insertWidget (pos, MsgFormatter_);
	}

	void ChatTab::handleAccountStyleChanged (IAccount *acc)
	{
		auto entry = GetEntry<ICLEntry> ();
		if (!entry)
			return;

		if (entry->GetParentAccount () != acc)
			return;

		PrepareTheme ();
	}

	void ChatTab::performJS (const QString& js)
	{
		Ui_.View_->page ()->runJavaScript (js);
	}

	template<typename T>
	T* ChatTab::GetEntry () const
	{
		QObject *obj = Core::Instance ().GetEntry (EntryID_);
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "no entry for"
					<< EntryID_;
			return 0;
		}

		return qobject_cast<T*> (obj);
	}

	namespace
	{
		bool CheckWithDefaultValue (const QString& entryId,
				const QString& groupName, const QByteArray& propertyName)
		{
			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Azoth");

			settings.beginGroup (groupName);
			auto guard = Util::MakeEndGroupScopeGuard (settings);

			if (settings.value ("Enabled").toStringList ().contains (entryId))
				return true;
			if (settings.value ("Disabled").toStringList ().contains (entryId))
				return false;

			return XmlSettingsManager::Instance ().property (propertyName).toBool ();
		}
	}

	void ChatTab::BuildBasicActions ()
	{
		auto sm = Core::Instance ().GetShortcutManager ();
		const auto& infos = sm->GetActionInfo ();

		const auto& clearInfo = infos ["org.LeechCraft.Azoth.ClearChat"];
		QAction *clearAction = new QAction (clearInfo.UserVisibleText_, this);
		clearAction->setProperty ("ActionIcon", "edit-clear-history");
		clearAction->setShortcuts (clearInfo.Seqs_);
		connect (clearAction,
				SIGNAL (triggered ()),
				this,
				SLOT (clearChat ()));
		TabToolbar_->addAction (clearAction);
		sm->RegisterAction ("org.LeechCraft.Azoth.ClearChat", clearAction);

		const auto& backInfo = infos ["org.LeechCraft.Azoth.ScrollHistoryBack"];
		QAction *historyBack = new QAction (backInfo.UserVisibleText_, this);
		historyBack->setProperty ("ActionIcon", "go-previous");
		historyBack->setShortcuts (backInfo.Seqs_);
		connect (historyBack,
				SIGNAL (triggered ()),
				this,
				SLOT (handleHistoryBack ()));
		TabToolbar_->addAction (historyBack);
		sm->RegisterAction ("org.LeechCraft.Azoth.ScrollHistoryBack", historyBack);

		TabToolbar_->addSeparator ();

		ToggleRichEditor_ = new QAction (tr ("Enable rich text editor"), this);
		ToggleRichEditor_->setProperty ("ActionIcon", "accessories-text-editor");
		ToggleRichEditor_->setCheckable (true);
		ToggleRichEditor_->setChecked (CheckWithDefaultValue (EntryID_, "RichEditorStates", "ShowRichTextEditor"));
		connect (ToggleRichEditor_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleRichEditorToggled ()));
		TabToolbar_->addAction (ToggleRichEditor_);

		ToggleRichText_ = new QAction (tr ("Enable rich text"), this);
		ToggleRichText_->setProperty ("ActionIcon", "text-enriched");
		ToggleRichText_->setCheckable (true);
		ToggleRichText_->setChecked (CheckWithDefaultValue (EntryID_, "RichTextStates", "ShowRichTextMessageBody"));
		connect (ToggleRichText_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleRichTextToggled ()));
		TabToolbar_->addAction (ToggleRichText_);
		TabToolbar_->addSeparator ();

		const auto& quoteInfo = infos ["org.LeechCraft.Azoth.QuoteSelected"];
		QAction *quoteSelection = new QAction (tr ("Quote selection"), this);
		quoteSelection->setProperty ("ActionIcon", "mail-reply-sender");
		quoteSelection->setShortcuts (quoteInfo.Seqs_);
		connect (quoteSelection,
				SIGNAL (triggered ()),
				this,
				SLOT (handleQuoteSelection ()));
		TabToolbar_->addAction (quoteSelection);
		TabToolbar_->addSeparator ();
		sm->RegisterAction ("org.LeechCraft.Azoth.QuoteSelected", quoteSelection);

		Ui_.View_->SetQuoteAction (quoteSelection);

		const auto& openLinkInfo = infos ["org.LeechCraft.Azoth.OpenLastLink"];
		auto shortcut = new QShortcut (openLinkInfo.Seqs_.value (0),
				this, SLOT (handleOpenLastLink ()), 0, Qt::WidgetWithChildrenShortcut);
		sm->RegisterShortcut ("org.LeechCraft.Azoth.OpenLastLink", openLinkInfo, shortcut);
	}

	void ChatTab::ReinitEntry ()
	{
		auto obj = GetEntry<QObject> ();
		connect (obj,
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleEntryMessage (QObject*)));
		connect (obj,
				SIGNAL (statusChanged (EntryStatus, QString)),
				this,
				SLOT (handleStatusChanged (EntryStatus, QString)));
		connect (obj,
				SIGNAL (availableVariantsChanged (QStringList)),
				this,
				SLOT (handleVariantsChanged (QStringList)));
		connect (obj,
				SIGNAL (nameChanged (QString)),
				this,
				SLOT (handleNameChanged (QString)));

		ICLEntry *e = GetEntry<ICLEntry> ();
		handleVariantsChanged (e->Variants ());

		QString infoText = e->GetEntryName ();
		if (e->GetHumanReadableID () != infoText)
			infoText += " (" + e->GetHumanReadableID () + ")";
		Ui_.EntryInfo_->setText (infoText);

		const auto& accName = e->GetParentAccount ()->GetAccountName ();
		Ui_.AccountName_->setText (accName);

		if (GetEntry<IUpdatableChatEntry> ())
			connect (obj,
					SIGNAL (performJS (QString)),
					this,
					SLOT (performJS (QString)));

		ReinitAvatar ();
	}

	void ChatTab::ReinitAvatar ()
	{
		const auto obj = GetEntry<QObject> ();

		auto avatarSetter = [this] (QImage avatar)
		{
			LastAvatar_ = QImage {};

			Ui_.AvatarLabel_->setVisible (!avatar.isNull ());

			if (avatar.isNull ())
				return;

			avatar = avatar.scaled ({ 18, 18 }, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			LastAvatar_ = avatar;

			const auto& px = QPixmap::fromImage (avatar);
			Ui_.AvatarLabel_->setPixmap (px);
			Ui_.AvatarLabel_->resize (px.size ());
			Ui_.AvatarLabel_->setMaximumSize (px.size ());
		};
		AvatarChangeSubscription_ = AvatarsManager_->Subscribe (obj,
				IHaveAvatars::Size::Thumbnail, avatarSetter);

		Util::Sequence (this,
				AvatarsManager_->GetAvatar (obj, IHaveAvatars::Size::Thumbnail)) >> avatarSetter;
	}

	void ChatTab::CheckMUC ()
	{
		ICLEntry *e = GetEntry<ICLEntry> ();

		bool claimsMUC = e->GetEntryType () == ICLEntry::EntryType::MUC;
		IsMUC_ = true;
		if (!claimsMUC)
			IsMUC_ = false;

		if (claimsMUC &&
				!GetEntry<IMUCEntry> ())
		{
			qWarning () << Q_FUNC_INFO
				<< e->GetEntryName ()
				<< "declares itself to be a MUC, "
					"but doesn't implement IMUCEntry";
			IsMUC_  = false;
		}

		if (IsMUC_ )
			HandleMUC ();
		else
		{
			Ui_.SubjectButton_->hide ();
			Ui_.MUCEventsButton_->hide ();
			TabIcon_ = ResourcesManager::Instance ().GetIconForState (e->GetStatus ().State_);

			connect (GetEntry<QObject> (),
					SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
					this,
					SLOT (handleChatPartStateChanged (const ChatPartState&, const QString&)));
		}
	}

	void ChatTab::HandleMUC ()
	{
		TabIcon_ = QIcon ("lcicons:/plugins/azoth/resources/images/azoth.svg");
		Ui_.AvatarLabel_->hide ();

		const int height = Util::AvailableGeometry (QCursor::pos ()).height ();

		MUCEventLog_->setWindowTitle (tr ("MUC log for %1")
					.arg (GetEntry<ICLEntry> ()->GetHumanReadableID ()));
		MUCEventLog_->setStyleSheet ("background-color: rgb(0, 0, 0);");
		MUCEventLog_->resize (600, height * 2 / 3);

		XmlSettingsManager::Instance ().RegisterObject ("SeparateMUCEventLogWindow",
				this, "handleSeparateMUCLog");
		handleSeparateMUCLog (true);
	}

	void ChatTab::InitExtraActions ()
	{
		ICLEntry *e = GetEntry<ICLEntry> ();
		const auto acc = e->GetParentAccount ();
		if (qobject_cast<ITransferManager*> (acc->GetTransferManager ()))
		{
			connect (acc->GetTransferManager (),
					SIGNAL (fileOffered (QObject*)),
					this,
					SLOT (handleFileOffered (QObject*)));

			for (const auto object : Core::Instance ().GetTransferJobManager ()->
							GetPendingIncomingJobsFor (EntryID_))
				handleFileOffered (object);
		}

#if defined(ENABLE_MEDIACALLS) || defined(ENABLE_CRYPT)
		const auto accObj = acc->GetQObject ();
#endif

#ifdef ENABLE_MEDIACALLS
		if (qobject_cast<ISupportMediaCalls*> (accObj) &&
				e->GetEntryType () == ICLEntry::EntryType::Chat)
		{
			Call_ = new QAction (tr ("Call..."), this);
			Call_->setProperty ("ActionIcon", "call-start");
			connect (Call_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleCallRequested ()));
			TabToolbar_->addAction (Call_);

			connect (accObj,
					SIGNAL (called (QObject*)),
					this,
					SLOT (handleCall (QObject*)));

			for (auto object : Core::Instance ().GetCallManager ()->GetCallsForEntry (EntryID_))
				handleCall (object);
		}
#endif

#ifdef ENABLE_CRYPT
		if (const auto isp = qobject_cast<ISupportPGP*> (accObj))
		{
			EnableEncryption_ = new QAction (tr ("Enable encryption"), this);
			EnableEncryption_->setProperty ("ActionIcon", "document-encrypt");
			EnableEncryption_->setCheckable (true);
			EnableEncryption_->setChecked (isp->IsEncryptionEnabled (e->GetQObject ()));
			connect (EnableEncryption_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleEnableEncryption ()));
			TabToolbar_->addAction (EnableEncryption_);

			connect (accObj,
					SIGNAL (encryptionStateChanged (QObject*, bool)),
					this,
					SLOT (handleEncryptionStateChanged (QObject*, bool)));
		}
#endif

		AddManagedActions (true);
	}

	void ChatTab::AddManagedActions (bool first)
	{
		QList<QAction*> coreActions;
		const auto manager = Core::Instance ().GetActionsManager ();
		for (const auto action : manager->GetEntryActions (GetEntry<ICLEntry> ()))
			if (manager->GetAreasForAction (action).contains (ActionsManager::CLEAAToolbar))
				coreActions << action;

		if (!first)
		{
			const auto& toolbarActions = TabToolbar_->actions ();
			for (auto i = coreActions.begin (); i != coreActions.end (); )
				if (toolbarActions.contains (*i))
					i = coreActions.erase (i);
				else
					++i;
		}

		if (coreActions.isEmpty ())
			return;

		if (!first)
			TabToolbar_->addSeparator ();
		TabToolbar_->addActions (coreActions);
	}

	void ChatTab::InitMsgEdit ()
	{
#ifndef Q_OS_MAC
		const auto histModifier = Qt::CTRL;
#else
		const auto histModifier = Qt::ALT;
#endif
		QShortcut *histUp = new QShortcut (histModifier + Qt::Key_Up,
				Ui_.MsgEdit_, 0, 0, Qt::WidgetShortcut);
		connect (histUp,
				SIGNAL (activated ()),
				this,
				SLOT (handleHistoryUp ()));

		QShortcut *histDown = new QShortcut (histModifier + Qt::Key_Down,
				Ui_.MsgEdit_, 0, 0, Qt::WidgetShortcut);
		connect (histDown,
				SIGNAL (activated ()),
				this,
				SLOT (handleHistoryDown ()));

		connect (Ui_.MsgEdit_,
				SIGNAL (keyReturnPressed ()),
				this,
				SLOT (messageSend ()));
		connect (Ui_.MsgEdit_,
				SIGNAL (scroll (int)),
				this,
				SLOT (handleEditScroll (int)));

		const auto completer = new MsgEditAutocompleter (EntryID_, Ui_.MsgEdit_, this);
		connect (Ui_.MsgEdit_,
				SIGNAL (keyTabPressed ()),
				completer,
				SLOT (complete ()));
		connect (Ui_.MsgEdit_,
				SIGNAL (clearAvailableNicks ()),
				completer,
				SLOT (resetState ()));

		UpdateTextHeight ();

		MsgFormatter_ = new MsgFormatterWidget (Ui_.MsgEdit_, Ui_.MsgEdit_);
		handleRichFormatterPosition ();
		connect (ToggleRichEditor_,
				SIGNAL (toggled (bool)),
				MsgFormatter_,
				SLOT (setVisible (bool)));
		MsgFormatter_->setVisible (ToggleRichEditor_->isChecked ());
	}

	void ChatTab::RegisterSettings ()
	{
		XmlSettingsManager::Instance ().RegisterObject ("RichFormatterPosition",
				this, "handleRichFormatterPosition");

		XmlSettingsManager::Instance ().RegisterObject ("SendButtonVisible",
				this, "handleSendButtonVisible");
		handleSendButtonVisible ();

		XmlSettingsManager::Instance ().RegisterObject ("MinLinesHeight",
				this, "handleMinLinesHeightChanged");
	}

	void ChatTab::RequestLogs (int num)
	{
		ICLEntry *entry = GetEntry<ICLEntry> ();
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "null entry for"
					<< EntryID_;
			return;
		}

		QObject *entryObj = entry->GetQObject ();

		const auto& histories = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<IHistoryPlugin*> ();
		for (const auto histObj : histories)
		{
			const auto hist = qobject_cast<IHistoryPlugin*> (histObj);
			if (!hist->IsHistoryEnabledFor (entryObj))
				continue;

			connect (histObj,
					SIGNAL (gotLastMessages (QObject*, const QList<QObject*>&)),
					this,
					SLOT (handleGotLastMessages (QObject*, const QList<QObject*>&)),
					Qt::UniqueConnection);

			hist->RequestLastMessages (entryObj, num);
		}
	}

	namespace
	{
		bool IsSameDay (const QDateTime& dt, const IMessage *msg)
		{
			return dt.date () == msg->GetDateTime ().date ();
		}
	}

	void ChatTab::AppendMessage (IMessage *msg)
	{
		auto other = qobject_cast<ICLEntry*> (msg->OtherPart ());

		if (msg->GetQObject ()->property ("Azoth/HiddenMessage").toBool ())
			return;

		ICLEntry *parent = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());

		if (msg->GetDirection () == IMessage::Direction::Out &&
				other &&
				other->GetEntryType () == ICLEntry::EntryType::MUC)
			return;

		if (msg->GetMessageSubType () == IMessage::SubType::ParticipantStatusChange &&
				(!parent || parent->GetEntryType () == ICLEntry::EntryType::MUC) &&
				!XmlSettingsManager::Instance ().property ("ShowStatusChangesEvents").toBool ())
			return;

		if (msg->GetMessageSubType () == IMessage::SubType::ParticipantStatusChange &&
				(!parent || parent->GetEntryType () != ICLEntry::EntryType::MUC) &&
				!XmlSettingsManager::Instance ().property ("ShowStatusChangesEventsInPrivates").toBool ())
			return;

		if ((msg->GetMessageSubType () == IMessage::SubType::ParticipantJoin ||
					msg->GetMessageSubType () == IMessage::SubType::ParticipantLeave) &&
				!XmlSettingsManager::Instance ().property ("ShowJoinsLeaves").toBool ())
			return;

		if (msg->GetMessageSubType () == IMessage::SubType::ParticipantEndedConversation)
		{
			if (!XmlSettingsManager::Instance ().property ("ShowEndConversations").toBool ())
				return;
			else if (other)
				msg->SetBody (tr ("%1 ended the conversation.")
						.arg (other->GetEntryName ()));
			else
				msg->SetBody (tr ("Conversation ended."));
		}

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookGonnaAppendMsg (proxy, msg->GetQObject ());
		if (proxy->IsCancelled ())
			return;

		if (XmlSettingsManager::Instance ().property ("SeparateMUCEventLogWindow").toBool () &&
				(!parent || parent->GetEntryType () == ICLEntry::EntryType::MUC) &&
				(msg->GetMessageType () != IMessage::Type::MUCMessage &&
					msg->GetMessageType () != IMessage::Type::ServiceMessage))
		{
			const auto& dt = msg->GetDateTime ().toString ("HH:mm:ss.zzz");
			MUCEventLog_->append (QString ("<font color=\"#56ED56\">[%1] %2</font>")
						.arg (dt)
						.arg (FormatterProxyObject {}.EscapeBody (msg->GetBody (), msg->GetEscapePolicy ())));
			if (msg->GetMessageSubType () != IMessage::SubType::RoomSubjectChange)
				return;
		}

		const auto frame = Ui_.View_->page ();

		const bool isActiveChat = Core::Instance ()
				.GetChatTabsManager ()->IsActiveChat (GetEntry<ICLEntry> ());

		if (!LastDateTime_.isNull () && !IsSameDay (LastDateTime_, msg) && parent)
		{
			auto datetime = msg->GetDateTime ();
			const auto& thisDate = datetime.date ();
			const auto& str = QLocale ().toString (thisDate, QLocale::LongFormat);

			datetime.setTime ({0, 0});

			auto coreMessage = new CoreMessage (str, datetime,
					IMessage::Type::ServiceMessage, IMessage::Direction::In, parent->GetQObject (), this);
			ChatMsgAppendInfo coreInfo
			{
				false,
				isActiveChat,
				ToggleRichText_->isChecked (),
				Account_
			};
			Core::Instance ().AppendMessageByTemplate (frame, coreMessage, coreInfo);
			CoreMessages_ << coreMessage;
		}

		LastDateTime_ = msg->GetDateTime ();

		ChatMsgAppendInfo info
		{
			Core::Instance ().IsHighlightMessage (msg),
			isActiveChat,
			ToggleRichText_->isChecked (),
			Account_
		};

		const auto& links = FormatterProxyObject {}.FindLinks (msg->GetBody ());
		if (!links.isEmpty ())
			LastLink_ = links.last ();

		if (!Core::Instance ().AppendMessageByTemplate (frame,
				msg->GetQObject (), info))
			qWarning () << Q_FUNC_INFO
					<< "unhandled append message :(";
	}

	QString ChatTab::ReformatTitle ()
	{
		if (!GetEntry<ICLEntry> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "GetEntry<ICLEntry> returned NULL";
			return {};
		}

		QString title = GetEntry<ICLEntry> ()->GetEntryName ();
		if (NumUnreadMsgs_)
			title.prepend (QString ("(%1) ")
					.arg (NumUnreadMsgs_));
		if (HadHighlight_)
			title.prepend ("* ");
		emit changeTabName (title);

		QStringList path ("Azoth");
		switch (GetEntry<ICLEntry> ()->GetEntryType ())
		{
		case ICLEntry::EntryType::Chat:
			path << tr ("Chat");
			break;
		case ICLEntry::EntryType::MUC:
			path << tr ("Conference");
			break;
		case ICLEntry::EntryType::PrivateChat:
			path << tr ("Private chat");
			break;
		case ICLEntry::EntryType::UnauthEntry:
			path << tr ("Unauthorized user");
			break;
		}

		path << title;

		setProperty ("WidgetLogicalPath", path);

		return title;
	}

	void ChatTab::UpdateTextHeight ()
	{
		Ui_.CharCounter_->setText (QString::number (Ui_.MsgEdit_->toPlainText ().size ()));

		const int docHeight = Ui_.MsgEdit_->document ()->size ().toSize ().height ();
		if (docHeight == PreviousTextHeight_)
			return;

		PreviousTextHeight_ = docHeight;
		const int numLines = XmlSettingsManager::Instance ().property ("MinLinesHeight").toInt ();
		const int fontHeight = Ui_.MsgEdit_->fontMetrics ().lineSpacing () * numLines +
				Ui_.MsgEdit_->document ()->documentMargin () * 2;
		const int resHeight = std::min (height () / 3, std::max (docHeight, fontHeight));
		Ui_.MsgEdit_->setMinimumHeight (resHeight);
		Ui_.MsgEdit_->setMaximumHeight (resHeight);
	}

	void ChatTab::prepareMessageText (const QString& text)
	{
		Ui_.MsgEdit_->setText (text);
		Ui_.MsgEdit_->moveCursor (QTextCursor::End);
	}

	void ChatTab::appendMessageText (const QString& text)
	{
		prepareMessageText (Ui_.MsgEdit_->toPlainText () + text);
	}

	void ChatTab::insertMessageText (const QString& text)
	{
		Ui_.MsgEdit_->textCursor ().insertText (text);
	}

	void ChatTab::selectVariant (const QString& var)
	{
		const int idx = Ui_.VariantBox_->findText (var);
		if (idx == -1)
			return;

		Ui_.VariantBox_->setCurrentIndex (idx);
	}

	QTextEdit* ChatTab::getMsgEdit ()
	{
		return Ui_.MsgEdit_;
	}

	void ChatTab::on_MUCEventsButton__toggled (bool on)
	{
		MUCEventLog_->setVisible (on);
		if (!on)
			return;

		MUCEventLog_->move (Util::FitRectScreen (QCursor::pos () + QPoint (2, 2),
					MUCEventLog_->size ()));
	}

	void ChatTab::handleSeparateMUCLog (bool initial)
	{
		MUCEventLog_->clear ();
		const bool isSep = XmlSettingsManager::Instance ()
				.property ("SeparateMUCEventLogWindow").toBool ();

		Ui_.MUCEventsButton_->setVisible (isSep);
		if (!initial)
			PrepareTheme ();
	}

	void ChatTab::handleChatWindowSearch (const QString& text)
	{
		ChatFinder_->SetText (text);
		ChatFinder_->FindNext ();

		ChatFinder_->show ();
	}

	void ChatTab::handleEditScroll (int direction)
	{
		Ui_.View_->page ()->runJavaScript (QStringLiteral ("ScrollPage(%1);").arg (direction));
	}

	void ChatTab::UpdateStateIcon ()
	{
		emit changeTabIcon (TabIcon_);
	}

#ifdef ENABLE_CRYPT
	void ChatTab::SetEncryptionEnabled (ISupportPGP *pgp, bool enable)
	{
		const auto& result = pgp->SetEncryptionEnabled (GetEntry<QObject> (), enable);
		if (!result)
			return;

		Util::Visit (*result,
				[this, pgp, enable] (const GPGExceptions::NullPubkey&)
				{
					if (QMessageBox::question (this,
								"LeechCraft",
								tr ("This entry has no pubkey assigned to it. Do you want to choose one?"),
								QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
						return;

					if (ChoosePGPKey (pgp, GetEntry<ICLEntry> ()))
						SetEncryptionEnabled (pgp, enable);
				},
				[this] (const GPGExceptions::General& ex)
				{
					QMessageBox::critical (this,
							"LeechCraft",
							tr ("Cannot enable encryption: %1.")
								.arg ("<br/><em>" + QString::fromStdString (ex.what ()) + "</em>"));
				});
	}
#endif
}
}
