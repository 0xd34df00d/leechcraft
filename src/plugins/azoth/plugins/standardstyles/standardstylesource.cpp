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
#include <QtDebug>
#include <plugininterface/resourceloader.h>
#include <plugininterface/util.h>
#include <interfaces/imessage.h>
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
	
	QString StandardStyleSource::GetHTMLTemplate (const QString& pack, QObject *entryObj) const
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
		
		return dev->readAll ();
	}
	
	bool StandardStyleSource::AppendMessage (QWebFrame *frame,
			QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		const QList<QColor>& colors = CreateColors (frame->metaData ().value ("coloring"));

		const bool isHighlightMsg = info.IsHighlightMsg_;
		const bool isActiveChat = info.IsActiveChat_;
		
		const QString& msgId = QString::number (reinterpret_cast<long int> (msgObj));

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		QString entryName = other ?
				Qt::escape (other->GetEntryName ()) :
				QString ();
				
		const QString& nickColor = Proxy_->GetNickColor (entryName, colors);
		
		QString body = Proxy_->FormatBody (msg->GetBody (), msg->GetObject ());

		QString divClass;
		QString statusIconName;
		QString string = Proxy_->FormatDate (msg->GetDateTime (), msg->GetObject ());
		string.append (' ');
		switch (msg->GetDirection ())
		{
		case IMessage::DIn:
		{
			statusIconName = "notification_chat_receive";
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
					string.append (entryName);
					string.append (": ");
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
				string.append (Proxy_->FormatNickname (nick, msg->GetObject (), nickColor));
				string.append (": ");
			}
			if (divClass.isEmpty ())
				divClass = "msgout";
			break;
		}
		}

		const QString& statusIconPath = Proxy_->
				GetResourceLoader (IProxyObject::PRLSystemIcons)->GetIconPath (statusIconName);
		const QImage& img = QImage (statusIconPath);
		string.prepend (QString ("<img src='%1' style='max-width: 1em; max-height: 1em;' id='%2'/>")
				.arg (Util::GetAsBase64Src (img))
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
}
}
}
