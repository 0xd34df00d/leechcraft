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
#include <plugininterface/defaulthookproxy.h>
#include <plugininterface/util.h>
#include "interfaces/iclentry.h"
#include "interfaces/imessage.h"
#include "interfaces/iaccount.h"
#include "interfaces/imucentry.h"
#include "core.h"
#include "mucsubjectdialog.h"
#include "textedit.h"
#include "chattabsmanager.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
	QObject *ChatTab::S_ParentMultiTabs_ = 0;

	void ChatTab::SetParentMultiTabs (QObject *obj)
	{
		S_ParentMultiTabs_ = obj;
	}

	ChatTab::ChatTab (const QByteArray& entryId,
			const QString& variant, QWidget *parent)
	: QWidget (parent)
	, EntryID_ (entryId)
	, Variant_ (variant)
	, LinkRegexp_ ("(\\b(?:(?:https?|ftp)://|www.|xmpp:)[\\w\\d/\\?.=:@&%#_;\\(?:\\)\\+\\-\\~\\*\\,]+)",
			Qt::CaseInsensitive, QRegExp::RegExp2)
	, ImageRegexp_ ("(\\b(?:data:image/)[\\w\\d/\\?.=:@&%#_;\\(?:\\)\\+\\-\\~\\*\\,]+)",
			Qt::CaseInsensitive, QRegExp::RegExp2)
	, BgColor_ (QApplication::palette ().color (QPalette::Base))
	, CurrentHistoryPosition_ (-1)
	, CurrentNickIndex_ (0)
	, LastSpacePosition_(-1)
	, NumUnreadMsgs_ (0)
	{
		Ui_.setupUi (this);

		Core::Instance ().RegisterHookable (this);

		QSize ccSize = Ui_.CharCounter_->size ();
		ccSize.setWidth (QApplication::fontMetrics ().width (" 9999"));
		Ui_.CharCounter_->resize (ccSize);

		Ui_.View_->page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);

		QFile file (":/plugins/azoth/resources/html/viewcontents.html");
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "could not open resource file"
					<< file.errorString ();
		}
		else
		{
			QString data = file.readAll ();
			data.replace ("BACKGROUNDCOLOR",
					BgColor_.name ());
			data.replace ("FOREGROUNDCOLOR",
					QApplication::palette ().color (QPalette::Text).name ());
			data.replace ("LINKCOLOR",
					QApplication::palette ().color (QPalette::Link).name ());
			Ui_.View_->setHtml (data);
		}

		connect (Ui_.View_->page (),
				SIGNAL (linkClicked (const QUrl&)),
				this,
				SLOT (handleViewLinkClicked (const QUrl&)));

		connect (GetEntry<QObject> (),
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleEntryMessage (QObject*)));
		connect (GetEntry<QObject> (),
				SIGNAL (statusChanged (const Plugins::EntryStatus&, const QString&)),
				this,
				SLOT (handleStatusChanged (const Plugins::EntryStatus&, const QString&)));

		Plugins::ICLEntry *e = GetEntry<Plugins::ICLEntry> ();
		Q_FOREACH (QObject *msgObj, e->GetAllMessages ())
		{
			Plugins::IMessage *msg = qobject_cast<Plugins::IMessage*> (msgObj);
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast message to IMessage"
						<< msgObj;
				continue;
			}
			AppendMessage (msg);
		}

		const QString& accName =
				qobject_cast<Plugins::IAccount*> (e->GetParentAccount ())->
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

	void ChatTab::HasBeenAdded ()
	{
		UpdateStateIcon ();
	}

	QList<QAction*> ChatTab::GetTabBarContextMenuActions () const
	{
		QList<QAction*> allActions = Core::Instance ()
				.GetEntryActions (GetEntry<Plugins::ICLEntry> ());
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
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookMadeCurrent (proxy, this);
		if (proxy->IsCancelled ())
			return;

		emit clearUnreadMsgCount (GetEntry<QObject> ());

		NumUnreadMsgs_ = 0;

		ReformatTitle ();
	}

	void ChatTab::messageSend ()
	{
		QString text = Ui_.MsgEdit_->toPlainText ();
		if (text.isEmpty ())
			return;

		Ui_.MsgEdit_->clear ();
		CurrentHistoryPosition_ = -1;
		MsgHistory_.prepend (text);

		Plugins::ICLEntry *e = GetEntry<Plugins::ICLEntry> ();
		QStringList currentVariants = e->Variants ();
		QString variant = currentVariants.contains (Variant_) ?
				Variant_ :
				currentVariants.first ();
		Plugins::IMessage::MessageType type =
				e->GetEntryType () == Plugins::ICLEntry::ETMUC ?
						Plugins::IMessage::MTMUCMessage :
						Plugins::IMessage::MTChatMessage;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy ());
		emit hookMessageWillCreated (proxy, this, type, variant, text);
		if (proxy->IsCancelled ())
			return;

		int intType = type;
		proxy->FillValue ("type", intType);
		type = static_cast<Plugins::IMessage::MessageType> (intType);
		proxy->FillValue ("variant", variant);
		proxy->FillValue ("text", text);

		if (ProcessOutgoingMsg (e, text))
			return;

		QObject *msgObj = e->CreateMessage (type, variant, text);

		Plugins::IMessage *msg = qobject_cast<Plugins::IMessage*> (msgObj);
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

	void ChatTab::on_SubjectButton__released ()
	{
		Plugins::IMUCEntry *me = GetEntry<Plugins::IMUCEntry> ();
		if (!me)
			return;

		const QString& subject = me->GetMUCSubject ();
		MUCSubjectDialog dia (subject, this);
		dia.exec ();
	}

	void ChatTab::handleEntryMessage (QObject *msgObj)
	{
		Plugins::IMessage *msg = qobject_cast<Plugins::IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage"
					<< sender ();
			return;
		}

		if (Core::Instance ().ShouldCountUnread (GetEntry<Plugins::ICLEntry> (), msg))
		{
			++NumUnreadMsgs_;
			ReformatTitle ();
		}

		AppendMessage (msg);
	}

	void ChatTab::handleStatusChanged (const Plugins::EntryStatus& status,
			const QString& variant)
	{
		if (variant != Variant_ &&
				!variant.isEmpty () &&
				GetEntry<Plugins::ICLEntry> ()->Variants ().value (0) == variant)
			return;

		TabIcon_ = Core::Instance ().GetIconForState (status.State_);
		UpdateStateIcon ();
	}

	void ChatTab::handleViewLinkClicked (const QUrl& url)
	{
		if (url.scheme () != "azoth")
		{
			Entity e = Util::MakeEntity (url,
					QString (), FromUserInitiated | OnlyHandle);
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

	void ChatTab::scrollToEnd ()
	{
		QWebFrame *frame = Ui_.View_->page ()->mainFrame ();
		int height = frame->contentsSize ().height ();
		frame->scroll (0, height);
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
		Plugins::ICLEntry *e = GetEntry<Plugins::ICLEntry> ();

		bool claimsMUC = e->GetEntryType () == Plugins::ICLEntry::ETMUC;
		bool isGoodMUC = true;
		if (!(claimsMUC))
			isGoodMUC = false;

		if (claimsMUC &&
				!GetEntry<Plugins::IMUCEntry> ())
		{
			qWarning () << Q_FUNC_INFO
				<< e->GetEntryName ()
				<< "declares itself to be a MUC, "
					"but doesn't implement IMUCEntry";
			isGoodMUC = false;
		}

		if (isGoodMUC)
			HandleMUC ();
		else
		{
			Ui_.SubjectButton_->hide ();
			TabIcon_ = Core::Instance ()
					.GetIconForState (e->GetStatus (Variant_).State_);
		}
	}

	void ChatTab::HandleMUC ()
	{
		TabIcon_ = QIcon (":/plugins/azoth/resources/images/azoth.svg");
	}

	void ChatTab::AppendMessage (Plugins::IMessage *msg)
	{
		Plugins::ICLEntry *other = qobject_cast<Plugins::ICLEntry*> (msg->OtherPart ());
		if (!other && msg->OtherPart ())
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part doesn't implement ICLEntry"
					<< msg->GetObject ()
					<< msg->OtherPart ();
			return;
		}

		if (msg->GetDirection () == Plugins::IMessage::DOut &&
				other->GetEntryType () == Plugins::ICLEntry::ETMUC)
			return;

		if (msg->GetMessageSubType () == Plugins::IMessage::MSTParticipantStatusChange &&
				!XmlSettingsManager::Instance ().property ("ShowStatusChangesEvents").toBool ())
			return;

		QWebFrame *frame = Ui_.View_->page ()->mainFrame ();
		bool shouldScrollFurther = (frame->scrollBarMaximum (Qt::Vertical) ==
						frame->scrollBarValue (Qt::Vertical));

		QString body = FormatBody (msg->GetBody (), msg);

		QString divClass;
		QString string = QString ("%1 ")
				.arg (FormatDate (msg->GetDateTime (), msg));
		string.append (' ');
		switch (msg->GetDirection ())
		{
		case Plugins::IMessage::DIn:
		{
			switch (msg->GetMessageType ())
			{
			case Plugins::IMessage::MTChatMessage:
			case Plugins::IMessage::MTMUCMessage:
			{
				QString entryName = Qt::escape (other->GetEntryName ());
				entryName = FormatNickname (entryName, msg);

				if (body.startsWith ("/me "))
				{
					body = body.mid (3);
					string.append ("* ");
					string.append (entryName);
					string.append (' ');
					divClass = "slashmechatmsg";
				}
				else
				{
					string.append (entryName);
					string.append (": ");
					divClass = Core::Instance ().IsHighlightMessage (msg) ?
							"highlightchatmsg" :
							"chatmsg";
				}
				break;
			}
			case Plugins::IMessage::MTEventMessage:
				string.append ("! ");
				divClass = "eventmsg";
				break;
			case Plugins::IMessage::MTStatusMessage:
				string.append ("* ");
				divClass = "statusmsg";
				break;
			}
			break;
		}
		case Plugins::IMessage::DOut:
			if (body.startsWith ("/leechcraft "))
			{
				body = body.mid (12);
				string.append ("* ");
			}
			else if (body.startsWith ("/me ") &&
					msg->GetMessageType () != Plugins::IMessage::MTMUCMessage)
			{
				Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (other->GetParentAccount ());
				body = body.mid (3);
				string.append ("* ");
				string.append (acc->GetOurNick ());
				string.append (' ');
				divClass = "slashmechatmsg";
			}
			else
			{
				Plugins::IAccount *acc = qobject_cast<Plugins::IAccount*> (other->GetParentAccount ());
				string.append (FormatNickname (acc->GetOurNick (), msg));
				string.append (": ");
			}
			divClass = "chatmsg";
			break;
		}

		string.append (body);

		QWebElement elem = frame->findFirstElement ("body");
		elem.appendInside (QString ("<div class='%2'>%1</div>")
					.arg (string)
					.arg (divClass));

		if (shouldScrollFurther)
			QTimer::singleShot (50,
					this,
					SLOT (scrollToEnd ()));
	}

	QString ChatTab::FormatDate (QDateTime dt, Plugins::IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatDateTime (proxy, this, dt, msg->GetObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("dateTime", dt);

		QString str = dt.time ().toString ();
		return QString ("<span class='datetime' style='color:green'>[" +
				str + "]</span>");
	}

	QString ChatTab::FormatNickname (QString nick, Plugins::IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatNickname (proxy, this, nick, msg->GetObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("nick", nick);

		QString string;

		QString color = "green";
		if (msg->GetMessageType () == Plugins::IMessage::MTMUCMessage)
		{
			if (NickColors_.isEmpty ())
				GenerateColors ();

			if (!NickColors_.isEmpty ())
			{
				int hash = 0;
				for (int i = 0; i < nick.length (); ++i)
				{
					const QChar& c = nick.at (i);
					hash += c.toLatin1 () ?
							c.toLatin1 () :
							c.unicode ();
					hash += nick.length ();
				}
				QColor nc = NickColors_.at (hash % NickColors_.size ());
				color = nc.name ();
			}

			QUrl url (QString ("azoth://msgeditreplace/%1")
					.arg (nick + ":"));

			string.append ("<span class='nickname'><a href='");
			string.append (url.toString () + "%20");
			string.append ("' class='nicklink' style='text-decoration:none; color:");
			string.append (color);
			string.append ("'>");
			string.append (nick);
			string.append ("</a></span>");
		}
		else
		{
			switch (msg->GetDirection ())
			{
				case Plugins::IMessage::DIn:
					color = "blue";
					break;
				case Plugins::IMessage::DOut:
					color = "red";
					break;
			}

			string = QString ("<span class='nickname' style='color:%2'>%1</span>")
					.arg (nick)
					.arg (color);
		}

		return string;
	}

	QString ChatTab::FormatBody (QString body, Plugins::IMessage *msg)
	{
		QObject *msgObj = msg->GetObject ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatBodyBegin (proxy, this, body, msgObj);
		if (!proxy->IsCancelled ())
		{
			proxy->FillValue ("body", body);
			body = Qt::escape (body);
			body.replace ('\n', "<br />");
			body.replace ("  ", "&nbsp; ");

			int pos = 0;
			while ((pos = LinkRegexp_.indexIn (body, pos)) != -1)
			{
				QString link = LinkRegexp_.cap (1);
				QString str = QString ("<a href=\"%1\">%1</a>")
						.arg (link);
				body.replace (pos, link.length (), str);

				pos += str.length ();
			}
			pos = 0;
			while ((pos = ImageRegexp_.indexIn (body, pos)) != -1)
			{
				QString image = ImageRegexp_.cap (1);
				// webkit not copy <img alt> to buffer with all text
				//  fix it in new <div ...
				QString str = QString ("<img src=\"%1\" alt=\"%1\" />\
						<div style='width: 1px; overflow: hidden;\
									height: 1px; float: left;\
									font-size: 1px;'>%1</div>")
						.arg (image);
				body.replace (pos, image.length (), str);
				pos += str.length ();
			}

			proxy.reset (new Util::DefaultHookProxy);
			emit hookFormatBodyEnd (proxy, this, body, msgObj);
			proxy->FillValue ("body", body);
		}

		return proxy->IsCancelled () ?
				proxy->GetReturnValue ().toString () :
				body;
	}

	bool ChatTab::ProcessOutgoingMsg (Plugins::ICLEntry *entry, QString& text)
	{
		Plugins::IMUCEntry *mucEntry = qobject_cast<Plugins::IMUCEntry*> (entry->GetObject ());
		if (entry->GetEntryType () == Plugins::ICLEntry::ETMUC &&
				mucEntry &&
				text.startsWith ("/nick "))
		{
			mucEntry->SetNick (text.mid (std::strlen ("/nick ")));
			return true;
		}

		return false;
	}

	namespace
	{
		qreal Fix (qreal h)
		{
			while (h < 0)
				h += 1;
			while (h >= 1)
				h -= 1;
			return h;
		}
	}

	void ChatTab::GenerateColors ()
	{
		const qreal lower = 25. / 360.;
		const qreal delta = 25. / 360.;
		const qreal higher = 180. / 360. - delta / 2;

		const qreal alpha = BgColor_.alphaF ();

		qreal s = BgColor_.saturationF ();
		s += 31 * (1 - s) / 32;
		qreal v = BgColor_.valueF ();
		v = 0.95 - 2 * v / 5;

		qreal h = BgColor_.hueF ();

		QColor color;
		for (qreal d = lower; d <= higher; d += delta)
		{
			color.setHsvF (Fix (h + d), s, v, alpha);
			NickColors_ << color;
			color.setHsvF (Fix (h - d), s, v, alpha);
			NickColors_ << color;
		}
	}

	void ChatTab::nickComplete ()
	{
		Plugins::ICLEntry *entry = GetEntry<Plugins::ICLEntry> ();
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "doesn't implement ICLEntry";

			return;
		}
		if (entry->GetEntryType() != Plugins::ICLEntry::ETMUC)
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
		Plugins::IMUCEntry *entry = GetEntry<Plugins::IMUCEntry> ();
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
			Plugins::ICLEntry *part = qobject_cast<Plugins::ICLEntry*> (item);
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
		if (!GetEntry<Plugins::ICLEntry> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "GetEntry<Plugins::ICLEntry> returned NULL";
			return;
		}

		QString title = GetEntry<Plugins::ICLEntry> ()->GetEntryName ();
		if (NumUnreadMsgs_)
			title.prepend (QString ("(%1) ")
					.arg (NumUnreadMsgs_));
		emit changeTabName (this, title);

		QStringList path ("Azoth");
		switch (GetEntry<Plugins::ICLEntry> ()->GetEntryType ())
		{
		case Plugins::ICLEntry::ETChat:
			path << tr ("Chat");
			break;
		case Plugins::ICLEntry::ETMUC:
			path << tr ("Conference");
			break;
		case Plugins::ICLEntry::ETPrivateChat:
			path << tr ("Private chat");
			break;
		case Plugins::ICLEntry::ETUnauthEntry:
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
}
