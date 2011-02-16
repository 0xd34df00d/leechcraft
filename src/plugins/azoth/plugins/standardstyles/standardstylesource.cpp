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
#include <interfaces/imessage.h>
#include <interfaces/iaccount.h>
#include <interfaces/iproxyobject.h>

namespace LeechCraft
{
namespace Azoth
{
namespace StandardStyles
{
	StandardStyleSource::StandardStyleSource (IProxyObject *proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, StylesLoader_ (new Util::ResourceLoader ("azoth/styles/standard/", this))
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
	
	bool StandardStyleSource::AppendMessage (QWebFrame *frame, QObject *msgObj,
			const QString& nickColor, bool isHighlightMsg, bool isActiveChat)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		QString entryName = other ?
				Qt::escape (other->GetEntryName ()) :
				QString ();
		
		QString body = Proxy_->FormatBody (msg->GetBody (), msg->GetObject ());

		QString divClass;
		QString string = QString ("%1 ")
				.arg (Proxy_->FormatDate (msg->GetDateTime (), msg->GetObject ()));
		string.append (' ');
		switch (msg->GetDirection ())
		{
		case IMessage::DIn:
		{
			switch (msg->GetMessageType ())
			{
			case IMessage::MTChatMessage:
			case IMessage::MTMUCMessage:
			{
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
					divClass = isHighlightMsg ?
							"highlightchatmsg" :
							"chatmsg";
				}
				break;
			}
			case IMessage::MTEventMessage:
				string.append ("! ");
				divClass = "eventmsg";
				break;
			case IMessage::MTStatusMessage:
				string.append ("* ");
				divClass = "statusmsg";
				break;
			}
			break;
		}
		case IMessage::DOut:
			if (body.startsWith ("/leechcraft "))
			{
				body = body.mid (12);
				string.append ("* ");
			}
			else if (body.startsWith ("/me ") &&
					msg->GetMessageType () != IMessage::MTMUCMessage)
			{
				IAccount *acc = qobject_cast<IAccount*> (other->GetParentAccount ());
				body = body.mid (3);
				string.append ("* ");
				string.append (acc->GetOurNick ());
				string.append (' ');
				divClass = "slashmechatmsg";
			}
			else
			{
				IAccount *acc = qobject_cast<IAccount*> (other->GetParentAccount ());
				string.append (Proxy_->FormatNickname (acc->GetOurNick (), msg->GetObject (), nickColor));
				string.append (": ");
			}
			divClass = "chatmsg";
			break;
		}

		string.append (body);

		QWebElement elem = frame->findFirstElement ("body");

		if (!isActiveChat &&
				!HasBeenAppended_ [frame])
		{
			QWebElement elem = frame->findFirstElement ("body");
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
}
}
}
