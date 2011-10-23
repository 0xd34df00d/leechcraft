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

#include "standardstylesource.h"
#include <QTextDocument>
#include <QWebElement>
#include <QWebFrame>
#include <QApplication>
#include <QtDebug>
#include <util/resourceloader.h>
#include <util/util.h>
#include <interfaces/imessage.h>
#include <interfaces/iadvancedmessage.h>
#include <interfaces/irichtextmessage.h>
#include <interfaces/iaccount.h>
#include <interfaces/imucentry.h>
#include <interfaces/iproxyobject.h>

namespace LeechCraft
{
namespace Azoth
{
namespace StandardStyles
{
	StandardStyleSource::StandardStyleSource (IProxyObject *proxy, QObject *parent)
	: QObject (parent)
	, StylesLoader_ (new Util::ResourceLoader ("azoth/styles/standard/", this))
	, Proxy_ (proxy)
	{
		StylesLoader_->AddGlobalPrefix ();
		StylesLoader_->AddLocalPrefix ();
	}

	QAbstractItemModel* StandardStyleSource::GetOptionsModel() const
	{
		return StylesLoader_->GetSubElemModel ();
	}

	QUrl StandardStyleSource::GetBaseURL (const QString& pack) const
	{
		return QUrl ();
	}

	QString StandardStyleSource::GetHTMLTemplate (const QString& pack, QObject *entryObj, QWebFrame*) const
	{
		if (pack != LastPack_)
		{
			Coloring2Colors_.clear ();
			LastPack_ = pack;
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		Util::QIODevice_ptr dev;
		if (entry && entry->GetEntryType () == ICLEntry::ETMUC)
			dev = StylesLoader_->Load (QStringList (pack + "/viewcontents.muc.html"));
		if (!dev)
			dev = StylesLoader_->Load (QStringList (pack + "/viewcontents.html"));

		if (!dev)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not load HTML template for pack"
					<< pack;
			return QString ();
		}

		if (!dev->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open source file for"
					<< pack + "/viewcontents.html"
					<< dev->errorString ();
			return QString ();
		}

		QString data = QString::fromUtf8 (dev->readAll ());
		data.replace ("BACKGROUNDCOLOR",
				QApplication::palette ().color (QPalette::Base).name ());
		data.replace ("FOREGROUNDCOLOR",
				QApplication::palette ().color (QPalette::Text).name ());
		data.replace ("LINKCOLOR",
				QApplication::palette ().color (QPalette::Link).name ());
		data.replace ("HIGHLIGHTCOLOR",
				Proxy_->GetSettingsManager ()->
						property ("HighlightColor").toString ());
		return data;
	}

	bool StandardStyleSource::AppendMessage (QWebFrame *frame,
			QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		QObject *azothSettings = Proxy_->GetSettingsManager ();
		const QList<QColor>& colors = CreateColors (frame->metaData ().value ("coloring"));

		const bool isHighlightMsg = info.IsHighlightMsg_;
		const bool isActiveChat = info.IsActiveChat_;

		const QString& msgId = GetMessageID (msgObj);

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		QString entryName = other ?
				Qt::escape (other->GetEntryName ()) :
				QString ();

		IAdvancedMessage *advMsg = qobject_cast<IAdvancedMessage*> (msgObj);
		if (msg->GetDirection () == IMessage::DOut &&
				advMsg &&
				!advMsg->IsDelivered ())
		{
			connect (msgObj,
					SIGNAL (messageDelivered ()),
					this,
					SLOT (handleMessageDelivered ()));
			connect (frame,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (handleFrameDestroyed ()),
					Qt::UniqueConnection);
			Msg2Frame_ [msgObj] = frame;
		}

		const QString& nickColor = Proxy_->GetNickColor (entryName, colors);

		IRichTextMessage *richMsg = qobject_cast<IRichTextMessage*> (msgObj);
		QString body;
		if (richMsg && info.UseRichTextBody_)
			body = richMsg->GetRichBody ();
		if (body.isEmpty ())
			body = msg->GetBody ();

		body = Proxy_->FormatBody (body, msg->GetObject ());

		const QString dateBegin ("<span class='datetime'>");
		const QString dateEnd ("</span>");

		const QString& preNick = dateBegin +
				azothSettings->property ("PreNickText").toString () +
				dateEnd;
		const QString& postNick = dateBegin +
				azothSettings->property ("PostNickText").toString () +
				dateEnd;

		QString divClass;
		QString statusIconName;

		QString string = dateBegin + '[' +
				Proxy_->FormatDate (msg->GetDateTime (), msg->GetObject ()) +
				']' + dateEnd;
		string.append (' ');
		switch (msg->GetDirection ())
		{
		case IMessage::DIn:
		{
			switch (msg->GetMessageType ())
			{
			case IMessage::MTChatMessage:
				statusIconName = "notification_chat_receive";
				divClass = msg->GetDirection () == IMessage::DIn ?
					"msgin" :
					"msgout";
			case IMessage::MTMUCMessage:
			{
				statusIconName = "notification_chat_receive";
				entryName = Proxy_->FormatNickname (entryName, msg->GetObject (), nickColor);

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
					string.append (preNick);
					string.append (entryName);
					string.append (postNick);
					string.append (' ');
					if (divClass.isEmpty ())
						divClass = isHighlightMsg ?
								"highlightchatmsg" :
								"chatmsg";
				}
				break;
			}
			case IMessage::MTEventMessage:
				statusIconName = "notification_chat_info";
				string.append ("! ");
				divClass = "eventmsg";
				break;
			case IMessage::MTStatusMessage:
				statusIconName = "notification_chat_info";
				string.append ("* ");
				divClass = "statusmsg";
				break;
			case IMessage::MTServiceMessage:
				qWarning () << Q_FUNC_INFO
						<< "service message";
				break;
			}
			break;
		}
		case IMessage::DOut:
		{
			statusIconName = "notification_chat_send";
			if (advMsg && advMsg->IsDelivered ())
				statusIconName = "notification_chat_delivery_ok";

			IMUCEntry *entry = qobject_cast<IMUCEntry*> (other->GetParentCLEntry ());
			IAccount *acc = qobject_cast<IAccount*> (other->GetParentAccount ());
			const QString& nick = entry ?
					entry->GetNick () :
					acc->GetOurNick ();
			if (body.startsWith ("/leechcraft "))
			{
				body = body.mid (12);
				string.append ("* ");
			}
			else if (body.startsWith ("/me ") &&
					msg->GetMessageType () != IMessage::MTMUCMessage)
			{
				body = body.mid (3);
				string.append ("* ");
				string.append (nick);
				string.append (' ');
				divClass = "slashmechatmsg";
			}
			else
			{
				string.append (preNick);
				string.append (Proxy_->FormatNickname (nick, msg->GetObject (), nickColor));
				string.append (postNick);
				string.append (' ');
			}
			if (divClass.isEmpty ())
				divClass = "msgout";
			break;
		}
		}

		if (!statusIconName.isEmpty ())
			string.prepend (QString ("<img src='%1' style='max-width: 1em; max-height: 1em;' id='%2'/>")
					.arg (GetStatusImage (statusIconName))
					.arg (msgId));
		string.append (body);

		QWebElement elem = frame->findFirstElement ("body");

		if (!isActiveChat &&
				!HasBeenAppended_ [frame])
		{
			QWebElement hr = elem.findFirst ("hr[class=\"lastSeparator\"]");
			if (hr.isNull ())
				elem.appendInside ("<hr class=\"lastSeparator\" />");
			else
				elem.appendInside (hr.takeFromDocument ());
			HasBeenAppended_ [frame] = true;
		}

		elem.appendInside (QString ("<div class='%1'>%2</div>")
					.arg (divClass)
					.arg (string));
		return true;
	}

	void StandardStyleSource::FrameFocused (QWebFrame *frame)
	{
		HasBeenAppended_ [frame] = false;
	}

	QList<QColor> StandardStyleSource::CreateColors (const QString& scheme)
	{
		if (!Coloring2Colors_.contains (scheme))
			Coloring2Colors_ [scheme] = Proxy_->GenerateColors (scheme);

		return Coloring2Colors_ [scheme];
	}

	QString StandardStyleSource::GetMessageID (QObject *msgObj)
	{
		return QString::number (reinterpret_cast<long int> (msgObj));
	}

	QString StandardStyleSource::GetStatusImage (const QString& statusIconName)
	{
		const QString& fullName = Proxy_->GetSettingsManager ()->
				property ("SystemIcons").toString () + '/' + statusIconName;
		const QString& statusIconPath = Proxy_->
				GetResourceLoader (IProxyObject::PRLSystemIcons)->GetIconPath (fullName);
		const QImage& img = QImage (statusIconPath);
		return Util::GetAsBase64Src (img);
	}

	void StandardStyleSource::handleMessageDelivered ()
	{
		QWebFrame *frame = Msg2Frame_.take (sender ());
		if (!frame)
			return;

		const QString& msgId = GetMessageID (sender ());
		QWebElement elem = frame->findFirstElement ("img[id=\"" + msgId + "\"]");
		elem.setAttribute ("src", GetStatusImage ("notification_chat_delivery_ok"));

		disconnect (sender (),
				SIGNAL (messageDelivered ()),
				this,
				SLOT (handleMessageDelivered ()));
	}

	void StandardStyleSource::handleFrameDestroyed ()
	{
		const QObject *snd = sender ();
		for (QHash<QObject*, QWebFrame*>::iterator i = Msg2Frame_.begin ();
				i != Msg2Frame_.end (); )
			if (i.value () == snd)
				i = Msg2Frame_.erase (i);
			else
				++i;
	}
}
}
}
