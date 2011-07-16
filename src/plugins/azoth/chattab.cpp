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

#include "chattab.h"
#include <boost/bind.hpp>
#include <QWebFrame>
#include <QWebElement>
#include <QTextDocument>
#include <QTimer>
#include <QPalette>
#include <QApplication>
#include <QShortcut>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <util/defaulthookproxy.h>
#include <util/util.h>
#include "interfaces/iclentry.h"
#include "interfaces/imessage.h"
#include "interfaces/irichtextmessage.h"
#include "interfaces/iaccount.h"
#include "interfaces/imucentry.h"
#include "interfaces/itransfermanager.h"
#include "interfaces/iconfigurablemuc.h"
#include "interfaces/ichatstyleresourcesource.h"
#include "interfaces/isupportmediacalls.h"
#include "interfaces/imediacall.h"
#include "interfaces/ihistoryplugin.h"
#include "core.h"
#include "textedit.h"
#include "chattabsmanager.h"
#include "xmlsettingsmanager.h"
#include "transferjobmanager.h"
#include "bookmarksmanagerdialog.h"
#include "simpledialog.h"
#include "zoomeventfilter.h"
#include "callmanager.h"
#include "callchatwidget.h"
#include "chattabwebview.h"
#include "msgformatterwidget.h"

namespace LeechCraft
{
namespace Azoth
{
	QObject *ChatTab::S_ParentMultiTabs_ = 0;

	void ChatTab::SetParentMultiTabs (QObject *obj)
	{
		S_ParentMultiTabs_ = obj;
	}

	ChatTab::ChatTab (const QString& entryId,
			QWidget *parent)
	: QWidget (parent)
	, TabToolbar_ (new QToolBar (tr ("Azoth chat window")))
	, ToggleRichText_ (0)
	, SendFile_ (0)
	, Call_ (0)
	, EntryID_ (entryId)
	, BgColor_ (QApplication::palette ().color (QPalette::Base))
	, CurrentHistoryPosition_ (-1)
	, CurrentNickIndex_ (0)
	, LastSpacePosition_(-1)
	, NumUnreadMsgs_ (0)
	, IsMUC_ (false)
	, PreviousTextHeight_ (0)
	, MsgFormatter_ (0)
	, XferManager_ (0)
	, TypeTimer_ (new QTimer (this))
	, PreviousState_ (CPSNone)
	{
		Ui_.setupUi (this);
		Ui_.View_->installEventFilter (new ZoomEventFilter (Ui_.View_));

		Ui_.SubjBox_->setVisible (false);
		Ui_.SubjChange_->setEnabled (false);
		
		Ui_.EventsButton_->setMenu (new QMenu (tr ("Events")));
		Ui_.EventsButton_->hide ();
		
		BuildBasicActions ();
		
		RequestLogs ();

		Core::Instance ().RegisterHookable (this);
		
		connect (Core::Instance ().GetTransferJobManager (),
				SIGNAL (jobNoLongerOffered (QObject*)),
				this,
				SLOT (handleFileNoLongerOffered (QObject*)));

		QSize ccSize = Ui_.CharCounter_->size ();
		ccSize.setWidth (QApplication::fontMetrics ().width (" 9999"));
		Ui_.CharCounter_->resize (ccSize);

		Ui_.View_->page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);

		connect (Ui_.View_->page (),
				SIGNAL (linkClicked (const QUrl&)),
				this,
				SLOT (handleViewLinkClicked (const QUrl&)));
		
		TypeTimer_->setInterval (2000);
		connect (TypeTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (typeTimeout ()));

		PrepareTheme ();
		InitEntry ();
		CheckMUC ();
		InitExtraActions ();
		InitMsgEdit ();
		
		emit hookChatTabCreated (IHookProxy_ptr (new Util::DefaultHookProxy),
				this,
				GetEntry<QObject> (),
				Ui_.View_);
		
		XmlSettingsManager::Instance ().RegisterObject ("FontSize",
				this, "handleFontSizeChanged");
		handleFontSizeChanged ();
		Ui_.View_->setFocusProxy (Ui_.MsgEdit_);
	}
	
	ChatTab::~ChatTab ()
	{
		SetChatPartState (CPSGone);
	}
	
	void ChatTab::PrepareTheme ()
	{
		QString data = Core::Instance ().GetSelectedChatTemplate (GetEntry<QObject> (),
				Ui_.View_->page ()->mainFrame ());
		if (data.isEmpty ())
			data = "<h1 style='color:red;'>" +
					tr ("Unable to load style, "
						"please check you've enabled at least one styles plugin.") +
					"</h1>";
		
		data.replace ("BACKGROUNDCOLOR",
				BgColor_.name ());
		data.replace ("FOREGROUNDCOLOR",
				QApplication::palette ().color (QPalette::Text).name ());
		data.replace ("LINKCOLOR",
				QApplication::palette ().color (QPalette::Link).name ());
		Ui_.View_->setHtml (data);
		
		Q_FOREACH (IMessage *msg, HistoryMessages_)
			AppendMessage (msg);

		ICLEntry *e = GetEntry<ICLEntry> ();
		Q_FOREACH (QObject *msgObj, e->GetAllMessages ())
		{
			IMessage *msg = qobject_cast<IMessage*> (msgObj);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast message to IMessage"
						<< msgObj;
				continue;
			}
			AppendMessage (msg);
		}
		
		QFile scrollerJS (":/plugins/azoth/resources/scripts/scrollers.js");
		if (!scrollerJS.open (QIODevice::ReadOnly))
			qWarning () << Q_FUNC_INFO
					<< "unable to open script file"
					<< scrollerJS.errorString ();
		else
		{
			Ui_.View_->page ()->mainFrame ()->evaluateJavaScript (scrollerJS.readAll ());
			Ui_.View_->page ()->mainFrame ()->evaluateJavaScript ("InstallEventListeners(); ScrollToBottom();");
		}
		
		emit hookThemeReloaded (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy),
				this, Ui_.View_, GetEntry<QObject> ());
	}

	void ChatTab::HasBeenAdded ()
	{
		UpdateStateIcon ();
	}
	
	TabClassInfo ChatTab::GetTabClassInfo () const
	{
		TabClassInfo chatTab =
		{
			"ChatTab",
			tr ("Chat"),
			tr ("A tab with a chat session"),
			QIcon (),
			0,
			TFEmpty
		};
		return chatTab;
	}

	QList<QAction*> ChatTab::GetTabBarContextMenuActions () const
	{
		QList<QAction*> allActions = Core::Instance ()
				.GetEntryActions (GetEntry<ICLEntry> ());
		QList<QAction*> result;
		Q_FOREACH (QAction *act, allActions)
		{
			if (Core::Instance ().GetAreasForAction (act)
					.contains (Core::CLEAATabCtxtMenu) ||
				act->isSeparator ())
				result << act;
		}
		return result;
	}

	QObject* ChatTab::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	QToolBar* ChatTab::GetToolBar () const
	{
		return TabToolbar_;
	}

	void ChatTab::Remove ()
	{
		emit needToClose (this);
	}

	void ChatTab::TabMadeCurrent ()
	{
		Core::Instance ().GetChatTabsManager ()->ChatMadeCurrent (this);
		Core::Instance ().FrameFocused (Ui_.View_->page ()->mainFrame ());

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookMadeCurrent (proxy, this);
		if (proxy->IsCancelled ())
			return;

		emit entryMadeCurrent (GetEntry<QObject> ());

		NumUnreadMsgs_ = 0;

		ReformatTitle ();
		Ui_.MsgEdit_->setFocus ();
	}
	
	void ChatTab::TabLostCurrent ()
	{
		TypeTimer_->stop ();
		SetChatPartState (CPSInactive);
	}
	
	QObject* ChatTab::GetCLEntry () const
	{
		return GetEntry<QObject> ();
	}

	void ChatTab::messageSend ()
	{
		QString text = Ui_.MsgEdit_->toPlainText ();
		if (text.isEmpty ())
			return;
		
		const QString& richText = MsgFormatter_->GetNormalizedRichText ();
		
		SetChatPartState (CPSActive);

		Ui_.MsgEdit_->clear ();
		Ui_.MsgEdit_->document ()->clear ();
		MsgFormatter_->Clear ();
		CurrentHistoryPosition_ = -1;
		MsgHistory_.prepend (text);

		QString variant = Ui_.VariantBox_->count () > 1 ?
				Ui_.VariantBox_->currentText () :
				QString ();

		ICLEntry *e = GetEntry<ICLEntry> ();
		IMessage::MessageType type =
				e->GetEntryType () == ICLEntry::ETMUC ?
						IMessage::MTMUCMessage :
						IMessage::MTChatMessage;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy ());
		emit hookMessageWillCreated (proxy, this, e->GetObject (), type, variant, text);
		if (proxy->IsCancelled ())
			return;

		int intType = type;
		proxy->FillValue ("type", intType);
		type = static_cast<IMessage::MessageType> (intType);
		proxy->FillValue ("variant", variant);
		proxy->FillValue ("text", text);

		if (ProcessOutgoingMsg (e, text))
			return;

		QObject *msgObj = e->CreateMessage (type, variant, text);

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast message from"
					<< e->GetEntryID ()
					<< "to IMessage"
					<< msgObj;
			return;
		}
		
		IRichTextMessage *richMsg = qobject_cast<IRichTextMessage*> (msgObj);
		if (richMsg &&
				!richText.isEmpty () &&
				ToggleRichText_->isChecked ())
			richMsg->SetRichBody (richText);

		proxy.reset (new Util::DefaultHookProxy ());
		emit hookMessageCreated (proxy, this, msg->GetObject ());
		if (proxy->IsCancelled ())
			return;

		msg->Send ();
	}

	void ChatTab::on_MsgEdit__textChanged ()
	{
		UpdateTextHeight ();

		SetChatPartState (CPSComposing);
		TypeTimer_->stop ();
		TypeTimer_->start ();
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

		/* TODO enable depending on whether we have enough rights to
		 * change the subject. And, if we don't, set the SubjEdit_ to
		 * readOnly() mode.
		 */
		Ui_.SubjEdit_->setText (me->GetMUCSubject ());
	}

	void ChatTab::on_SubjChange__released()
	{
		Ui_.SubjectButton_->setChecked (false);

		IMUCEntry *me = GetEntry<IMUCEntry> ();
		if (!me)
			return;

		me->SetMUCSubject (Ui_.SubjEdit_->toPlainText ());
	}

	void ChatTab::handleSendFile ()
	{
		if (!XferManager_)
		{
			qWarning () << Q_FUNC_INFO
					<< "called with null XferManager_";
			return;
		}

		const QString& filename = QFileDialog::getOpenFileName (this,
				tr ("Select file to send"));
		if (filename.isEmpty ())
			return;

		QObject *job = XferManager_->SendFile (EntryID_,
				Ui_.VariantBox_->currentText (), filename);
		Core::Instance ().HandleTransferJob (job);
	}
	
	void ChatTab::handleCallRequested ()
	{
		QObject *callObj = Core::Instance ().GetCallManager ()->
				Call (GetEntry<ICLEntry> (), Ui_.VariantBox_->currentText ());
		if (!callObj)
			return;
		handleCall (callObj);
	}
	
	void ChatTab::handleCall (QObject *callObj)
	{
		IMediaCall *call = qobject_cast<IMediaCall*> (callObj);
		if (!call || call->GetSourceID () != EntryID_)
			return;

		CallChatWidget *widget = new CallChatWidget (callObj);
		const int idx = Ui_.MainLayout_->indexOf (Ui_.View_);
		Ui_.MainLayout_->insertWidget (idx, widget);
	}
	
	void ChatTab::handleClearChat ()
	{
		ICLEntry *entry = GetEntry<ICLEntry> ();
		if (!entry)
			return;
		
		entry->PurgeMessages (QDateTime ());
		PrepareTheme ();
	}
	
	void ChatTab::handleRichTextToggled ()
	{
		PrepareTheme ();
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
	}
	
	void ChatTab::handleFileNoLongerOffered (QObject *jobObj)
	{
		Q_FOREACH (QAction *action, Ui_.EventsButton_->menu ()->actions ())
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
		
		if (QMessageBox::question (this,
					tr ("File transfer request"),
					tr ("Would you like to accept or deny file transfer "
						"request for file %1?")
						.arg (job->GetName ()),
					QMessageBox::Save | QMessageBox::Abort) == QMessageBox::Abort)
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

		if (Core::Instance ().ShouldCountUnread (GetEntry<ICLEntry> (), msg))
		{
			++NumUnreadMsgs_;
			ReformatTitle ();
		}

		const int idx = Ui_.VariantBox_->findText (msg->GetOtherVariant ());
		if (idx != -1)
			Ui_.VariantBox_->setCurrentIndex (idx);

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

		Q_FOREACH (const QString& variant, variants)
		{
			const State& st = GetEntry<ICLEntry> ()->GetStatus (variant).State_;
			const QIcon& icon = Core::Instance ().GetIconForState (st);
			Ui_.VariantBox_->addItem (icon, variant);
		}

		if (!variants.isEmpty ())
		{
			const int pos = std::max (0, Ui_.VariantBox_->findText (current));
			Ui_.VariantBox_->setCurrentIndex (pos);
		}
		
		Ui_.VariantBox_->setVisible (variants.size () > 1);
	}

	void ChatTab::handleStatusChanged (const EntryStatus& status,
			const QString& variant)
	{
		const QStringList& vars = GetEntry<ICLEntry> ()->Variants ();

		const QIcon& icon = Core::Instance ().GetIconForState (status.State_);

		if (status.State_ == SOffline)
			handleVariantsChanged (vars);
		else
			for (int i = 0; i < Ui_.VariantBox_->count (); ++i)
				if (variant == Ui_.VariantBox_->itemText (i))
				{
					Ui_.VariantBox_->setItemIcon (i, icon);
					break;
				}

		if (!variant.isEmpty () &&
				vars.size () &&
				vars.value (0) != variant)
			return;

		TabIcon_ = icon;
		UpdateStateIcon ();
	}

	void ChatTab::handleChatPartStateChanged (const ChatPartState& state, const QString&)
	{
		QString text = GetEntry<ICLEntry> ()->GetEntryName ();

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

	void ChatTab::handleViewLinkClicked (const QUrl& url)
	{
		if (url.scheme () != "azoth")
		{
			Entity e = Util::MakeEntity (url,
					QString (), static_cast<TaskParameter> (FromUserInitiated | OnlyHandle));
			Core::Instance ().SendEntity (e);
			return;
		}

		if (url.host () == "msgeditreplace")
		{
			Ui_.MsgEdit_->setText (url.path ().mid (1));
			Ui_.MsgEdit_->moveCursor (QTextCursor::End);
			Ui_.MsgEdit_->setFocus ();
		}
		else if (url.host () == "insertnick")
		{
			const QByteArray& encoded = url.encodedQueryItemValue ("nick");
			InsertNick (QUrl::fromPercentEncoding (encoded));
		}
	}

	void ChatTab::InsertNick (const QString& nicknameHtml)
	{
		QTextCursor cursor = Ui_.MsgEdit_->textCursor ();
		cursor.insertHtml (cursor.atStart () ?
				nicknameHtml + ": " :
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
	
	void ChatTab::handleAddToBookmarks ()
	{
		BookmarksManagerDialog *dia = new BookmarksManagerDialog (this);
		dia->SuggestSaving (GetEntry<QObject> ());
		dia->setAttribute (Qt::WA_DeleteOnClose, true);
		dia->show ();
	}
	
	void ChatTab::handleConfigureMUC ()
	{
		IConfigurableMUC *confMUC = GetEntry<IConfigurableMUC> ();
		if (!confMUC)
			return;

		QWidget *w = confMUC->GetConfigurationWidget ();
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty conf widget"
					<< GetEntry<QObject> ();
			return;
		}

		SimpleDialog *dia = new SimpleDialog ();
		dia->setWindowTitle (tr ("Room configuration"));
		dia->SetWidget (w);
		connect (dia,
				SIGNAL (accepted ()),
				dia,
				SLOT (deleteLater ()),
				Qt::QueuedConnection);
		dia->show ();
	}
	
	void ChatTab::typeTimeout ()
	{
		SetChatPartState (CPSPaused);
	}
	
	namespace
	{
		struct PredSimilarMessage
		{
			IMessage *To_;

			PredSimilarMessage (IMessage *to)
			: To_ (to)
			{
			}
			
			bool operator() (QObject *msgObj)
			{
				IMessage *msg = qobject_cast<IMessage*> (msgObj);
				if (!msg)
				{
					qWarning () << Q_FUNC_INFO
							<< msgObj
							<< "doesn't implement IMessage";
					return false;
				}
				
				return msg->GetDirection () == To_->GetDirection () &&
						msg->GetBody () == To_->GetBody () &&
						std::abs (msg->GetDateTime ().secsTo (To_->GetDateTime ())) < 5;
			}
		};
	}
	
	void ChatTab::handleGotLastMessages (QObject *entryObj, const QList<QObject*>& messages)
	{
		if (entryObj != GetEntry<QObject> ())
			return;
		
		ICLEntry *entry = GetEntry<ICLEntry> ();
		QList<QObject*> rMsgs = entry->GetAllMessages ();
		std::reverse (rMsgs.begin (), rMsgs.end ());

		Q_FOREACH (QObject *msgObj, messages)
		{
			IMessage *msg = qobject_cast<IMessage*> (msgObj);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< msgObj
						<< "doesn't implement IMessage";
				continue;
			}
			
			const QDateTime& dt = msg->GetDateTime ();
			
			if (std::find_if (rMsgs.begin (), rMsgs.end (),
						PredSimilarMessage (msg)) != rMsgs.end ())
				continue;
			
			if (HistoryMessages_.isEmpty () ||
					HistoryMessages_.last ()->GetDateTime () <= dt)
				HistoryMessages_ << msg;
			else
			{
				QList<IMessage*>::iterator pos =
						std::find_if (HistoryMessages_.begin (), HistoryMessages_.end (),
								boost::bind (std::greater<QDateTime> (),
										boost::bind (&IMessage::GetDateTime, _1),
										dt));
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
	
	void ChatTab::handleFontSizeChanged ()
	{
		const int size = XmlSettingsManager::Instance ()
				.property ("FontSize").toInt ();
		Ui_.View_->settings ()->setFontSize (QWebSettings::DefaultFontSize, size);
	}

	template<typename T>
	T* ChatTab::GetEntry () const
	{
		QObject *obj = Core::Instance ().GetEntry (EntryID_);
		T *entry = qobject_cast<T*> (obj);
		if (!entry)
			qWarning () << Q_FUNC_INFO
					<< "object"
					<< obj
					<< "doesn't implement the required interface";
		return entry;
	}
	
	void ChatTab::BuildBasicActions ()
	{
		QAction *clearAction = new QAction (tr ("Clear chat window"), this);
		clearAction->setProperty ("ActionIcon", "clear");
		connect (clearAction,
				SIGNAL (triggered ()),
				this,
				SLOT (handleClearChat ()));
		TabToolbar_->addAction (clearAction);

		ToggleRichText_ = new QAction (tr ("Enable rich text"), this);
		ToggleRichText_->setProperty ("ActionIcon", "richtext");
		ToggleRichText_->setCheckable (true);
		ToggleRichText_->setChecked (XmlSettingsManager::Instance ()
					.property ("ShowRichTextMessageBody").toBool ());
		connect (ToggleRichText_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleRichTextToggled ()));
		TabToolbar_->addAction (ToggleRichText_);
		TabToolbar_->addSeparator ();
	}
	
	void ChatTab::InitEntry()
	{
		connect (GetEntry<QObject> (),
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleEntryMessage (QObject*)));
		connect (GetEntry<QObject> (),
				SIGNAL (statusChanged (const EntryStatus&, const QString&)),
				this,
				SLOT (handleStatusChanged (const EntryStatus&, const QString&)));
		connect (GetEntry<QObject> (),
				SIGNAL (availableVariantsChanged (const QStringList&)),
				this,
				SLOT (handleVariantsChanged (QStringList)));

		ICLEntry *e = GetEntry<ICLEntry> ();
		handleVariantsChanged (e->Variants ());
		Ui_.EntryInfo_->setText (e->GetEntryName ());

		const QString& accName =
				qobject_cast<IAccount*> (e->GetParentAccount ())->
						GetAccountName ();
		Ui_.AccountName_->setText (accName);
	}

	void ChatTab::CheckMUC ()
	{
		ICLEntry *e = GetEntry<ICLEntry> ();

		bool claimsMUC = e->GetEntryType () == ICLEntry::ETMUC;
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
			TabIcon_ = Core::Instance ()
					.GetIconForState (e->GetStatus ().State_);

			connect (GetEntry<QObject> (),
					SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
					this,
					SLOT (handleChatPartStateChanged (const ChatPartState&, const QString&)));
		}
	}

	void ChatTab::HandleMUC ()
	{
		TabIcon_ = QIcon (":/plugins/azoth/resources/images/azoth.svg");
		
		QAction *bookmarks = new QAction (tr ("Add to bookmarks..."), this);
		bookmarks->setProperty ("ActionIcon", "favorites");
		connect (bookmarks,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddToBookmarks ()));
		TabToolbar_->addAction (bookmarks);
		
		IConfigurableMUC *confmuc = GetEntry<IConfigurableMUC> ();
		if (confmuc)
		{
			QAction *configureMUC = new QAction (tr ("Configure MUC..."), this);
			configureMUC->setProperty ("ActionIcon", "preferences");
			connect (configureMUC,
					SIGNAL (triggered ()),
					this,
					SLOT (handleConfigureMUC ()));
			TabToolbar_->addAction (configureMUC);
		}
	}
	
	void ChatTab::InitExtraActions ()
	{
		ICLEntry *e = GetEntry<ICLEntry> ();
		QObject *accObj = e->GetParentAccount ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		XferManager_ = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (XferManager_ &&
			!IsMUC_ &&
			acc->GetAccountFeatures () & IAccount::FMUCsSupportFileTransfers)
		{
			SendFile_ = new QAction (tr ("Send file..."), this);
			SendFile_->setProperty ("ActionIcon", "sendfile");
			connect (SendFile_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleSendFile ()));
			TabToolbar_->addAction (SendFile_);

			connect (acc->GetTransferManager (),
					SIGNAL (fileOffered (QObject*)),
					this,
					SLOT (handleFileOffered (QObject*)));
			
			Q_FOREACH (QObject *object,
					Core::Instance ().GetTransferJobManager ()->
							GetPendingIncomingJobsFor (EntryID_))
				handleFileOffered (object);
		}
		
		if (qobject_cast<ISupportMediaCalls*> (accObj) &&
				e->GetEntryType () == ICLEntry::ETChat)
		{
			Call_ = new QAction (tr ("Call..."), this);
			Call_->setProperty ("ActionIcon", "call");
			connect (Call_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleCallRequested ()));
			TabToolbar_->addAction (Call_);
			
			connect (accObj,
					SIGNAL (called (QObject*)),
					this,
					SLOT (handleCall (QObject*)));
			
			Q_FOREACH (QObject *object,
					Core::Instance ().GetCallManager ()->
							GetCallsForEntry (EntryID_))
				handleCall (object);
		}
	}
	
	void ChatTab::InitMsgEdit ()
	{
		QShortcut *histUp = new QShortcut (Qt::CTRL + Qt::Key_Up,
				Ui_.MsgEdit_, 0, 0, Qt::WidgetShortcut);
		connect (histUp,
				SIGNAL (activated ()),
				this,
				SLOT (handleHistoryUp ()));

		QShortcut *histDown = new QShortcut (Qt::CTRL + Qt::Key_Down,
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
				SIGNAL (keyTabPressed ()),
				this,
				SLOT (nickComplete ()));
		connect (Ui_.MsgEdit_,
				SIGNAL (scroll (int)),
				this,
				SLOT (handleEditScroll (int)));

		QTimer::singleShot (0,
				Ui_.MsgEdit_,
				SLOT (setFocus ()));

		connect (Ui_.MsgEdit_,
				SIGNAL (clearAvailableNicks ()),
				this,
				SLOT (clearAvailableNick ()));
		UpdateTextHeight ();
		
		const int pos = Ui_.MainLayout_->indexOf (Ui_.MsgEdit_);
		
		MsgFormatter_ = new MsgFormatterWidget (Ui_.MsgEdit_);
		Ui_.MainLayout_->insertWidget (pos, MsgFormatter_);
		connect (ToggleRichText_,
				SIGNAL (toggled (bool)),
				MsgFormatter_,
				SLOT (setVisible (bool)));
		MsgFormatter_->setVisible (ToggleRichText_->isChecked ());
	}
	
	void ChatTab::RequestLogs ()
	{
		ICLEntry *entry = GetEntry<ICLEntry> ();
		if (entry->GetAllMessages ().size () > 100 ||
				entry->GetEntryType () != ICLEntry::ETChat)
			return;
		
		const int num = XmlSettingsManager::Instance ()
				.property ("ShowLastNMessages").toInt ();
		if (!num)
			return;

		QObject *entryObj = entry->GetObject ();

		const QObjectList& histories = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableRoots<IHistoryPlugin*> ();
		
		Q_FOREACH (QObject *histObj, histories)
		{
			IHistoryPlugin *hist = qobject_cast<IHistoryPlugin*> (histObj);	
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

	void ChatTab::AppendMessage (IMessage *msg)
	{
		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		if (!other && msg->OtherPart ())
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part doesn't implement ICLEntry"
					<< msg->GetObject ()
					<< msg->OtherPart ();
			return;
		}
		
		ICLEntry *parent = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());

		if (msg->GetDirection () == IMessage::DOut &&
				other->GetEntryType () == ICLEntry::ETMUC)
			return;

		if (msg->GetMessageSubType () == IMessage::MSTParticipantStatusChange &&
				(!parent || parent->GetEntryType () == ICLEntry::ETMUC) &&
				!XmlSettingsManager::Instance ().property ("ShowStatusChangesEvents").toBool ())
			return;
		
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookGonnaAppendMsg (proxy, msg->GetObject ());
		if (proxy->IsCancelled ())
			return;

		QWebFrame *frame = Ui_.View_->page ()->mainFrame ();
		
		ChatMsgAppendInfo info =
		{
			Core::Instance ().IsHighlightMessage (msg),
			Core::Instance ().GetChatTabsManager ()->IsActiveChat (GetEntry<ICLEntry> ()),
			ToggleRichText_->isChecked ()
		};

		if (!Core::Instance ().AppendMessageByTemplate (frame,
				msg->GetObject (), info))
			qWarning () << Q_FUNC_INFO
					<< "unhandled append message :(";
	}

	bool ChatTab::ProcessOutgoingMsg (ICLEntry *entry, QString& text)
	{
		IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (entry->GetObject ());
		if (entry->GetEntryType () == ICLEntry::ETMUC &&
				mucEntry)
		{
			if (text.startsWith ("/nick "))
			{
				mucEntry->SetNick (text.mid (std::strlen ("/nick ")));
				return true;
			}
			else if (text == "/names")
			{
				QStringList names;
				Q_FOREACH (QObject *obj, mucEntry->GetParticipants ())
				{
					ICLEntry *entry = qobject_cast<ICLEntry*> (obj);
					if (!entry)
					{
						qWarning () << Q_FUNC_INFO
								<< obj
								<< "doesn't implement ICLEntry";
						continue;
					}
					const QString& name = entry->GetEntryName ();
					if (!name.isEmpty ())
						names << name;
				}
				names.sort ();
				QWebElement body = Ui_.View_->page ()->mainFrame ()->findFirstElement ("body");
				body.appendInside ("<div class='systemmsg'>" +
						tr ("MUC's participants: ") + "<ul><li>" +
						names.join ("</li><li>") + "</li></ul></div>");
				return true;
			}
		}

		return false;
	}

	void ChatTab::nickComplete ()
	{
		IMUCEntry *entry = GetEntry<IMUCEntry> ();
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "doesn't implement ICLEntry";

			return;
		}

		QStringList currentMUCParticipants = GetMUCParticipants ();
		currentMUCParticipants.removeAll (entry->GetNick ());
		if (currentMUCParticipants.isEmpty ())
			return;

		QTextCursor cursor = Ui_.MsgEdit_->textCursor ();

		int cursorPosition = cursor.position ();
		int pos = -1;
		int lastNickLen = -1;

		QString text = Ui_.MsgEdit_->toPlainText ();

		if (AvailableNickList_.isEmpty ())
		{
			if (cursorPosition)
				pos = text.lastIndexOf (' ', cursorPosition - 1);
			else
				pos = text.lastIndexOf (' ', cursorPosition);
			LastSpacePosition_ = pos;
		}
		else
			pos = LastSpacePosition_;

		if (NickFirstPart_.isNull ())
		{
			if (!cursorPosition)
				NickFirstPart_ = "";
			else
				NickFirstPart_ = text.mid (pos + 1, cursorPosition - pos -1);
		}

		if (AvailableNickList_.isEmpty ())
		{
			Q_FOREACH (const QString& item, currentMUCParticipants)
				if (item.startsWith (NickFirstPart_, Qt::CaseInsensitive))
					AvailableNickList_ << item + (pos == -1 ? ": " : " ");

			if (AvailableNickList_.isEmpty ())
				return;

			text.replace (pos + 1,
					NickFirstPart_.length (),
					AvailableNickList_ [CurrentNickIndex_]);
		}
		else
		{
			QStringList newAvailableNick;

			Q_FOREACH (const QString& item, currentMUCParticipants)
				if (item.startsWith (NickFirstPart_, Qt::CaseInsensitive))
					newAvailableNick << item + (pos == -1 ? ": " : " ");

			if ((newAvailableNick != AvailableNickList_) && (!newAvailableNick.isEmpty ()))
			{
				int newIndex = newAvailableNick.indexOf (AvailableNickList_ [CurrentNickIndex_ - 1]);
				lastNickLen = AvailableNickList_ [CurrentNickIndex_ - 1].length ();

				while (newIndex == -1 && CurrentNickIndex_ > 0)
					newIndex = newAvailableNick.indexOf (AvailableNickList_ [--CurrentNickIndex_]);

				CurrentNickIndex_ = (newIndex == -1 ? 0 : newIndex);
				AvailableNickList_ = newAvailableNick;
			}

			if (CurrentNickIndex_ < AvailableNickList_.count () && CurrentNickIndex_)
				text.replace (pos + 1,
						AvailableNickList_ [CurrentNickIndex_ - 1].length (),
						AvailableNickList_ [CurrentNickIndex_]);
			else if (CurrentNickIndex_)
			{
				CurrentNickIndex_ = 0;
				text.replace (pos + 1,
						AvailableNickList_.last ().length (),
						AvailableNickList_ [CurrentNickIndex_]);
			}
			else
				text.replace (pos + 1,
						lastNickLen,
						AvailableNickList_ [CurrentNickIndex_]);
		}
		++CurrentNickIndex_;

		Ui_.MsgEdit_->setPlainText (text);
		cursor.setPosition (pos + 1 + AvailableNickList_ [CurrentNickIndex_ - 1].length ());
		Ui_.MsgEdit_->setTextCursor (cursor);
	}

	QStringList ChatTab::GetMUCParticipants () const
	{
		IMUCEntry *entry = GetEntry<IMUCEntry> ();
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "doesn't implement IMUCEntry";

			return QStringList ();
		}

		QStringList participantsList;

		Q_FOREACH (QObject *item, entry->GetParticipants ())
		{
			ICLEntry *part = qobject_cast<ICLEntry*> (item);
			if (!part)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast item to ICLEntry"
						<< item;
				return QStringList ();
			}
			participantsList << part->GetEntryName ();
		}
		return participantsList;
	}

	void ChatTab::ReformatTitle ()
	{
		if (!GetEntry<ICLEntry> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "GetEntry<ICLEntry> returned NULL";
			return;
		}

		QString title = GetEntry<ICLEntry> ()->GetEntryName ();
		if (NumUnreadMsgs_)
			title.prepend (QString ("(%1) ")
					.arg (NumUnreadMsgs_));
		emit changeTabName (this, title);

		QStringList path ("Azoth");
		switch (GetEntry<ICLEntry> ()->GetEntryType ())
		{
		case ICLEntry::ETChat:
			path << tr ("Chat");
			break;
		case ICLEntry::ETMUC:
			path << tr ("Conference");
			break;
		case ICLEntry::ETPrivateChat:
			path << tr ("Private chat");
			break;
		case ICLEntry::ETUnauthEntry:
			path << tr ("Unauthorized user");
			break;
		}

		path << title;

		setProperty ("WidgetLogicalPath", path);
	}
	
	void ChatTab::UpdateTextHeight ()
	{
		Ui_.CharCounter_->setText (QString::number (Ui_.MsgEdit_->toPlainText ().size ()));

		const int docHeight = Ui_.MsgEdit_->document ()->size ().toSize ().height ();
		if (docHeight == PreviousTextHeight_)
			return;
		
		PreviousTextHeight_ = docHeight;
		const int fontHeight = Ui_.MsgEdit_->fontMetrics ().height ();
		const int resHeight = std::min (height () / 3, std::max (docHeight, fontHeight));
		Ui_.MsgEdit_->setMinimumHeight (resHeight);
		Ui_.MsgEdit_->setMaximumHeight (resHeight);
	}
	
	void ChatTab::SetChatPartState (ChatPartState state)
	{
		if (state == PreviousState_)
			return;

		if (IsMUC_)
			return;
		
		if (!XmlSettingsManager::Instance ()
				.property ("SendChatStates").toBool ())
			return;
		
		ICLEntry *entry = GetEntry<ICLEntry> ();
		if (!entry)
			return;
		
		PreviousState_ = state;
		entry->SetChatPartState (state, Ui_.VariantBox_->currentText ());
	}
	
	void ChatTab::prepareMessageText (const QString& text)
	{
		Ui_.MsgEdit_->setText (text);
	}
	
	void ChatTab::appendMessageText (const QString& text)
	{
		Ui_.MsgEdit_->setText (Ui_.MsgEdit_->toPlainText () + text);
	}
	
	QTextEdit* ChatTab::getMsgEdit ()
	{
		return Ui_.MsgEdit_;
	}

	void ChatTab::clearAvailableNick ()
	{
		NickFirstPart_.clear ();
		if (!AvailableNickList_.isEmpty ())
		{
			AvailableNickList_.clear ();
			CurrentNickIndex_ = 0;
		}
	}
	
	void ChatTab::handleEditScroll (int direction)
	{
		int distance = Ui_.View_->size ().height () / 2 - 5;
		Ui_.View_->page ()->mainFrame ()->scroll (0, distance * direction);
	}

	void ChatTab::UpdateStateIcon ()
	{
		emit changeTabIcon (this, TabIcon_);
	}
}
}
