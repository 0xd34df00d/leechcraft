/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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
#include <plugininterface/defaulthookproxy.h>
#include <plugininterface/util.h>
#include "interfaces/iclentry.h"
#include "interfaces/imessage.h"
#include "interfaces/iaccount.h"
#include "interfaces/imucentry.h"
#include "interfaces/itransfermanager.h"
#include "core.h"
#include "textedit.h"
#include "chattabsmanager.h"
#include "xmlsettingsmanager.h"
#include "transferjobmanager.h"

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
			const QString& variant, QWidget *parent)
	: QWidget (parent)
	, EntryID_ (entryId)
	, Variant_ (variant)
	, BgColor_ (QApplication::palette ().color (QPalette::Base))
	, CurrentHistoryPosition_ (-1)
	, CurrentNickIndex_ (0)
	, LastSpacePosition_(-1)
	, NumUnreadMsgs_ (0)
	, IsMUC_ (false)
	, XferManager_ (0)
	{
		Ui_.setupUi (this);

		Ui_.SubjBox_->setVisible (false);
		Ui_.SubjChange_->setEnabled (false);
		
		Ui_.EventsButton_->setMenu (new QMenu (tr ("Events")));
		Ui_.EventsButton_->hide ();

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
				SLOT (handleVariantsChanged (const QStringList&)));

		PrepareTheme ();

		ICLEntry *e = GetEntry<ICLEntry> ();
		handleVariantsChanged (e->Variants ());

		const QString& accName =
				qobject_cast<IAccount*> (e->GetParentAccount ())->
						GetAccountName ();
		Ui_.AccountName_->setText (accName);

		CheckMUC ();

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

		Ui_.EntryInfo_->setText (e->GetEntryName ());

		QTimer::singleShot (0, Ui_.MsgEdit_, SLOT (setFocus ()));

		connect (Ui_.MsgEdit_,
				SIGNAL (clearAvailableNicks ()),
				this,
				SLOT (clearAvailableNick ()));
		on_MsgEdit__textChanged ();
	}
	
	void ChatTab::PrepareTheme ()
	{
		QString data = Core::Instance ().GetSelectedChatTemplate (GetEntry<QObject> ());
		if (data.isEmpty ())
		{
			QFile file (":/plugins/azoth/resources/html/viewcontents.html");
			if (!file.open (QIODevice::ReadOnly))
				qWarning () << Q_FUNC_INFO
						<< "could not open resource file"
						<< file.errorString ();
			else
				data = file.readAll ();
		}
		
		if (!data.isEmpty ())
		{
			data.replace ("BACKGROUNDCOLOR",
					BgColor_.name ());
			data.replace ("FOREGROUNDCOLOR",
					QApplication::palette ().color (QPalette::Text).name ());
			data.replace ("LINKCOLOR",
					QApplication::palette ().color (QPalette::Link).name ());
			Ui_.View_->setHtml (data);
		}
		
		GenerateColors ();

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
	}

	void ChatTab::HasBeenAdded ()
	{
		UpdateStateIcon ();
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

	QObject* ChatTab::ParentMultiTabs () const
	{
		return S_ParentMultiTabs_;
	}

	void ChatTab::NewTabRequested ()
	{
	}

	QToolBar* ChatTab::GetToolBar () const
	{
		return 0;
	}

	void ChatTab::Remove ()
	{
		emit needToClose (this);
	}

	void ChatTab::TabMadeCurrent ()
	{
		Core::Instance ().FrameFocused (Ui_.View_->page ()->mainFrame ());

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookMadeCurrent (proxy, this);
		if (proxy->IsCancelled ())
			return;

		emit clearUnreadMsgCount (GetEntry<QObject> ());

		NumUnreadMsgs_ = 0;

		ReformatTitle ();
		Ui_.MsgEdit_->setFocus ();
	}

	void ChatTab::messageSend ()
	{
		QString text = Ui_.MsgEdit_->toPlainText ();
		if (text.isEmpty ())
			return;

		Ui_.MsgEdit_->clear ();
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
		emit hookMessageWillCreated (proxy, this, type, variant, text);
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

		proxy.reset (new Util::DefaultHookProxy ());
		emit hookMessageCreated (proxy, this, msg->GetObject ());
		if (proxy->IsCancelled ())
			return;

		msg->Send ();
		AppendMessage (msg);
	}

	void ChatTab::on_MsgEdit__textChanged ()
	{
		Ui_.CharCounter_->setText (QString::number (Ui_.MsgEdit_->toPlainText ().size ()));

		const int docHeight = Ui_.MsgEdit_->document ()->size ().toSize ().height ();
		const int fontHeight = Ui_.MsgEdit_->fontMetrics ().height ();
		const int resHeight = std::min (height () / 3, std::max (docHeight, fontHeight));
		Ui_.MsgEdit_->setMinimumHeight (resHeight);
		Ui_.MsgEdit_->setMaximumHeight (resHeight);
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

	void ChatTab::on_SendFileButton__released ()
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

		AppendMessage (msg);
	}

	void ChatTab::handleVariantsChanged (const QStringList& variants)
	{
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

		Ui_.VariantBox_->addItems (variants);
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

		if (status.State_ == SOffline)
			handleVariantsChanged (vars);

		if (variant != Variant_ &&
				!variant.isEmpty () &&
				vars.size () &&
				vars.value (0) == variant)
			return;

		TabIcon_ = Core::Instance ().GetIconForState (status.State_);
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
					.GetIconForState (e->GetStatus (Variant_).State_);

			connect (GetEntry<QObject> (),
					SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
					this,
					SLOT (handleChatPartStateChanged (const ChatPartState&, const QString&)));
		}

		IAccount *acc = qobject_cast<IAccount*> (GetEntry<ICLEntry> ()->GetParentAccount ());
		XferManager_ = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (!XferManager_ ||
			(IsMUC_ &&
			 !(acc->GetAccountFeatures () & IAccount::FMUCsSupportFileTransfers)))
			Ui_.SendFileButton_->hide ();
		else
		{
			connect (acc->GetTransferManager (),
					SIGNAL (fileOffered (QObject*)),
					this,
					SLOT (handleFileOffered (QObject*)));
			
			Q_FOREACH (QObject *object,
					Core::Instance ().GetTransferJobManager ()->
							GetPendingIncomingJobsFor (EntryID_))
				handleFileOffered (object);
		}
	}

	void ChatTab::HandleMUC ()
	{
		TabIcon_ = QIcon (":/plugins/azoth/resources/images/azoth.svg");
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

		if (msg->GetDirection () == IMessage::DOut &&
				other->GetEntryType () == ICLEntry::ETMUC)
			return;

		if (msg->GetMessageSubType () == IMessage::MSTParticipantStatusChange &&
				!XmlSettingsManager::Instance ().property ("ShowStatusChangesEvents").toBool ())
			return;

		QWebFrame *frame = Ui_.View_->page ()->mainFrame ();
		
		QString entryName = other ?
				Qt::escape (other->GetEntryName ()) :
				QString ();

		if (!Core::Instance ().AppendMessageByTemplate (frame, msg->GetObject (),
					Core::Instance ().GetNickColor (entryName, NickColors_),
					Core::Instance ().IsHighlightMessage (msg),
					Core::Instance ().GetChatTabsManager ()->IsActiveChat (GetEntry<ICLEntry> ())))
			qWarning () << Q_FUNC_INFO
					<< "unhandled append message :(";
	}

	bool ChatTab::ProcessOutgoingMsg (ICLEntry *entry, QString& text)
	{
		IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (entry->GetObject ());
		if (entry->GetEntryType () == ICLEntry::ETMUC &&
				mucEntry &&
				text.startsWith ("/nick "))
		{
			mucEntry->SetNick (text.mid (std::strlen ("/nick ")));
			return true;
		}

		return false;
	}

	void ChatTab::GenerateColors ()
	{
		const QMultiMap<QString, QString>& metadata =
				Ui_.View_->page ()->mainFrame ()->metaData ();
		const QString& coloring = metadata.value ("coloring");
		NickColors_ = Core::Instance ().GenerateColors (metadata.value ("coloring"));
	}

	void ChatTab::nickComplete ()
	{
		ICLEntry *entry = GetEntry<ICLEntry> ();
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "doesn't implement ICLEntry";

			return;
		}
		if (entry->GetEntryType() != ICLEntry::ETMUC)
			return;

		QStringList currentMUCParticipants = GetMUCParticipants ();
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
						<< "unable to cast message from"
						<< part->GetEntryID ()
						<< "to ICLEntry"
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

	void ChatTab::clearAvailableNick ()
	{
		NickFirstPart_.clear ();
		if (!AvailableNickList_.isEmpty ())
		{
			AvailableNickList_.clear ();
			CurrentNickIndex_ = 0;
		}
	}

	void ChatTab::UpdateStateIcon ()
	{
		emit changeTabIcon (this, TabIcon_);
	}
}
}
