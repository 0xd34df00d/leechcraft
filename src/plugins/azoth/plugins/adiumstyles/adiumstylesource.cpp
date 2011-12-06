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

#include "adiumstylesource.h"
#include <QTextDocument>
#include <QWebElement>
#include <QWebFrame>
#include <QtDebug>
#include <util/resourceloader.h>
#include <util/util.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/imessage.h>
#include <interfaces/irichtextmessage.h>
#include <interfaces/iadvancedmessage.h>
#include <interfaces/iclentry.h>
#include <interfaces/imucentry.h>
#include <interfaces/iaccount.h>
#include <interfaces/iprotocol.h>
#include <interfaces/iextselfinfoaccount.h>
#include "packproxymodel.h"

namespace LeechCraft
{
namespace Azoth
{
namespace AdiumStyles
{
	AdiumStyleSource::AdiumStyleSource (IProxyObject *proxy, QObject *parent)
	: QObject (parent)
	, StylesLoader_ (new Util::ResourceLoader ("azoth/styles/adium/", this))
	, Proxy_ (proxy)
	, PackProxyModel_ (new PackProxyModel (StylesLoader_, this))
	{
		StylesLoader_->AddGlobalPrefix ();
		StylesLoader_->AddLocalPrefix ();

		StylesLoader_->SetCacheParams (2048, 0);
	}

	QAbstractItemModel* AdiumStyleSource::GetOptionsModel () const
	{
		return PackProxyModel_;
	}

	QUrl AdiumStyleSource::GetBaseURL (const QString& srcPack) const
	{
		const QString& pack = PackProxyModel_->GetOrigName (srcPack);
		const QString& prefix = pack + "/Contents/Resources/";

		QString path = StylesLoader_->
				GetPath (QStringList (prefix + "Header.html"));
		if (path.isEmpty ())
			path = StylesLoader_->
					GetPath (QStringList (prefix + "main.css"));
		if (path.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty base URL for"
					<< srcPack;
			return QUrl ();
		}

		return QUrl::fromLocalFile (path);
	}

	QString AdiumStyleSource::GetHTMLTemplate (const QString& srcPack,
			QObject *entryObj, QWebFrame *frame) const
	{
		if (srcPack != LastPack_)
		{
			Coloring2Colors_.clear ();
			LastPack_ = srcPack;

			StylesLoader_->FlushCache ();
		}

		const QString& pack = PackProxyModel_->GetOrigName (srcPack);
		const QString& varCss = PackProxyModel_->GetVariant (srcPack);

		Frame2LastContact_.remove (frame);

		const QString& prefix = pack + "/Contents/Resources/";

		Util::QIODevice_ptr header = StylesLoader_->
				Load (QStringList (prefix + "Header.html"));
		Util::QIODevice_ptr footer = StylesLoader_->
				Load (QStringList (prefix + "Footer.html"));
		Util::QIODevice_ptr css = StylesLoader_->
				Load (QStringList (prefix + "main.css"));

		if (!header)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not load HTML template for pack"
					<< pack;
			return QString ();
		}

		if (!header->open (QIODevice::ReadOnly) ||
				(footer && !footer->open (QIODevice::ReadOnly)) ||
				(css && !css->open (QIODevice::ReadOnly)))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open source files for"
					<< pack
					<< header->errorString ()
					<< (footer ? footer->errorString () : "empty footer")
					<< (css ? css->errorString () : "empty css");
			return QString ();
		}

		Frame2Pack_ [frame] = pack;

		QString cssStr = css ?
				QString::fromUtf8 (css->readAll ()) :
				QString ();

		QString varCssStr;
		if (!varCss.isEmpty ())
		{
			Util::QIODevice_ptr varCssDev = StylesLoader_->
					Load (QStringList (prefix + "Variants/" + varCss + ".css"));
			if (varCssDev && varCssDev->open (QIODevice::ReadOnly))
			{
				varCssStr = QString::fromUtf8 (varCssDev->readAll ());
				varCssStr.remove ("../");
			}
		}

		QString result;
		result = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">";
		result += "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">";
		result += cssStr + varCssStr;
		result += "</style><title></title></head><body>";
		result += QString::fromUtf8 (header->readAll ());
		result += "<div id=\"Chat\"><div id=\"insert\"></div></div>";
		if (footer)
			result += QString::fromUtf8 (footer->readAll ());
		result += "</body></html>";

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry, but proceeding...";
			return result;
		}

		result.replace ("%chatName%", entry->GetEntryName ());
		if (result.contains ("%incomingIconPath%"))
			result.replace ("%incomingIconPath%",
					Util::GetAsBase64Src (entry->GetAvatar ()));

		return result;
	}

	bool AdiumStyleSource::AppendMessage (QWebFrame *frame,
			QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return false;
		}

		const QString& pack = Frame2Pack_ [frame];
		if (pack.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty pack for"
					<< msgObj
					<< msg->OtherPart ();
			return false;
		}

		const bool in = msg->GetDirection () == IMessage::DIn;
		const QString& prefix = pack + "/Contents/Resources/" +
				(in ? "Incoming" : "Outgoing") +
				'/';

		QObject *kindaSender = in ? msg->OtherPart () : reinterpret_cast<QObject*> (42);
		const bool isNextMsg = Frame2LastContact_.contains (frame) &&
				kindaSender == Frame2LastContact_ [frame];
		const bool isSlashMe = msg->GetBody ()
				.trimmed ().startsWith ("/me ");
		QString filename;
		if ((msg->GetMessageType () == IMessage::MTChatMessage ||
					msg->GetMessageType () == IMessage::MTMUCMessage) &&
				!isSlashMe)
			filename = isNextMsg ?
					"NextContent.html" :
					"Content.html";
		else
			filename = "Action.html";

		if (msg->GetMessageType () != IMessage::MTMUCMessage &&
				msg->GetMessageType () != IMessage::MTChatMessage)
			Frame2LastContact_.remove (frame);
		else if (!isNextMsg)
			Frame2LastContact_ [frame] = kindaSender;

		Util::QIODevice_ptr content = StylesLoader_->
				Load (QStringList (prefix + filename));
		if (!content && filename == "Action.html")
			content = StylesLoader_->
					Load (QStringList (prefix + "../Status.html"));
		if (!content)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to load content template for"
					<< pack
					<< prefix;
			return false;
		}

		if (!content->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open contents for"
					<< pack
					<< prefix
					<< content->errorString ();
			return false;
		}

		const QString& newSelector = QString ("div[id=\"Chat\"]");
		const QString& nextSelector = QString ("*[id=\"insert\"]");
		QWebElement chat = frame->findFirstElement (isNextMsg ? nextSelector : newSelector);
		if (chat.isNull ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no div for insertion could be found";
			return false;
		}

		const QString& templ = QString::fromUtf8 (content->readAll ());
		const QString& body = ParseTemplate (templ, prefix, frame, msgObj, info);
		if (isNextMsg)
			chat.setOuterXml (body);
		else
		{
			QWebElement next = frame->findFirstElement (nextSelector);
			if (!next.isNull ())
				next.removeFromDocument ();

			chat.appendInside (body);
		}

		if (templ.contains ("%stateElementId%"))
		{
			IAdvancedMessage *advMsg = qobject_cast<IAdvancedMessage*> (msgObj);
			QString fname;
			if (!advMsg || advMsg->IsDelivered () || in)
				fname = "StateSent.html";
			else
			{
				fname = "StateSending.html";
				connect (msgObj,
						SIGNAL (messageDelivered ()),
						this,
						SLOT (handleMessageDelivered ()),
						Qt::UniqueConnection);
				Msg2Frame_ [msgObj] = frame;
			}

			Util::QIODevice_ptr content =
					StylesLoader_->Load (QStringList (prefix + fname));
			QString replacement;
			if (content && content->open (QIODevice::ReadOnly))
				replacement = QString::fromUtf8 (content->readAll ());

			const QString& selector = QString ("*[id=\"delivery_state_%1\"]")
					.arg (GetMessageID (msgObj));
			QWebElement elem = frame->findFirstElement (selector);
			elem.setInnerXml (replacement);
		}

		return true;
	}

	void AdiumStyleSource::FrameFocused (QWebFrame*)
	{
	}

	QString AdiumStyleSource::ParseTemplate (QString templ, const QString& base,
			QWebFrame*, QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		const bool isHighlightMsg = info.IsHighlightMsg_;

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		const bool in = msg->GetDirection () == IMessage::DIn;

		ICLEntry *other = 0;
		switch (msg->GetMessageType ())
		{
		case IMessage::MTChatMessage:
		case IMessage::MTMUCMessage:
		case IMessage::MTStatusMessage:
			other = qobject_cast<ICLEntry*> (msg->OtherPart ());
			break;
		case IMessage::MTEventMessage:
		case IMessage::MTServiceMessage:
			other = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
			break;
		}

		if (!other)
		{
			qWarning () << Q_FUNC_INFO
					<< "null other part, gonna fail:"
					<< msg->GetMessageType ()
					<< msg->GetBody ()
					<< msg->OtherPart ()
					<< msg->ParentCLEntry ();
		}

		IAccount *acc = other ?
				qobject_cast<IAccount*> (other->GetParentAccount ()) :
				0;

		if (!acc && msg->ParentCLEntry ())
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
			acc = entry ?
					qobject_cast<IAccount*> (entry->GetParentAccount ()) :
					0;
		}

		if (!acc && !in)
		{
			qWarning () << Q_FUNC_INFO
					<< "no account for outgoing message, that sucks"
					<< msg->GetMessageType ()
					<< msg->OtherPart ()
					<< msg->ParentCLEntry ();
			return templ;
		}

		const QString& senderNick = in ?
				other->GetEntryName () :
				acc->GetOurNick ();

		// %time%
		templ.replace ("%time%", msg->GetDateTime ().toString ());

		// %time{X}%
		QRegExp timeRx ("%time\\{(.*?)\\}%");
		int pos = 0;
		while ((pos = timeRx.indexIn (templ, pos)) != -1)
			templ.replace (pos, timeRx.matchedLength (),
					msg->GetDateTime ().toString (timeRx.cap (1)));

		// %messageDirection%
		templ.replace ("%messageDirection%", "ltr");

		// TODO show our avatar
		// %userIconPath%
		QImage image;
		if (in)
			image = other->GetAvatar ();
		else if (acc)
		{
			IExtSelfInfoAccount *self = qobject_cast<IExtSelfInfoAccount*> (acc->GetObject ());
			if (self)
				image = qobject_cast<ICLEntry*> (self->GetSelfContact ())->GetAvatar ();
		}

		if (image.isNull ())
			image = QImage (StylesLoader_->GetPath (QStringList (base + "buddy_icon.png")));
		if (image.isNull ())
			qWarning () << Q_FUNC_INFO
					<< "image is still null, though tried"
					<< base + "buddy_icon.png";
		templ.replace ("%userIconPath%", Util::GetAsBase64Src (image));

		// %senderScreenName%
		templ.replace ("%senderScreenName%",
				in ? other->GetHumanReadableID () : acc->GetAccountName ());

		// %sender%
		templ.replace ("%sender%",
				Proxy_->FormatNickname (senderNick, msgObj, "%senderColor%"));

		// %service%
		templ.replace ("%service%",
				acc ?
					qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolName () :
					QString ());

		// %textbackgroundcolor{X}%
		QRegExp bgColorRx ("%textbackgroundcolor\\{([^}]*)\\}%");
		pos = 0;
		const QString& highColor = isHighlightMsg ?
				Proxy_->GetSettingsManager ()->
						property ("HighlightColor").toString () :
				"inherit";
		bool hasHighBackground = false;
		while ((pos = bgColorRx.indexIn (templ, pos)) != -1)
		{
			templ.replace (pos, bgColorRx.matchedLength (), highColor);
			hasHighBackground = true;
		}

		// %senderStatusIcon%
		if (templ.contains ("%senderStatusIcon%"))
		{
			const State state = in ?
					other->GetStatus (msg->GetOtherVariant ()).State_ :
					acc->GetState ().State_;
			const QIcon& icon = Proxy_->GetIconForState (state);

			const QPixmap& px = icon.pixmap (icon.actualSize (QSize (256, 256)));

			templ.replace ("%senderStatusIcon%", Util::GetAsBase64Src (px.toImage ()));
		}

		// First, prepare colors
		if (templ.contains ("%senderColor") && !Coloring2Colors_.contains ("hash"))
			Coloring2Colors_ ["hash"] = Proxy_->GenerateColors ("hash");

		// %senderColor%
		const QString& nickColor = Proxy_->
				GetNickColor (senderNick, Coloring2Colors_ ["hash"]);
		templ.replace ("%senderColor%", nickColor);

		// %senderColor{N}%
		QRegExp senderColorRx ("%senderColor(?:\\{([^}]*)\\})?%");
		pos = 0;
		while ((pos = senderColorRx.indexIn (templ, pos)) != -1)
		{
			QColor color (nickColor);
			color = color.lighter (senderColorRx.cap (1).toInt ());
			templ.replace (pos, senderColorRx.matchedLength (), color.name ());
		}

		// %stateElementId%
		if (templ.contains ("%stateElementId%"))
			templ.replace ("%stateElementId%", "delivery_state_" + GetMessageID (msgObj));

		// %message%
		IRichTextMessage *richMsg = qobject_cast<IRichTextMessage*> (msgObj);
		QString body;
		if (richMsg && info.UseRichTextBody_)
			body = richMsg->GetRichBody ();
		if (body.isEmpty ())
			body = msg->GetBody ();

		if (body.startsWith ("/me "))
			body = QString ("* %1 %2")
					.arg (senderNick)
					.arg (body.mid (4));

		body = Proxy_->FormatBody (body, msgObj);

		if (isHighlightMsg && !hasHighBackground)
			body = "<span style=\"color:" + highColor +
					"\">" + body + "</span>";

		templ.replace ("%message%", body);

		return templ;
	}

	QList<QColor> AdiumStyleSource::CreateColors (const QString& scheme)
	{
		if (!Coloring2Colors_.contains (scheme))
			Coloring2Colors_ [scheme] = Proxy_->GenerateColors (scheme);

		return Coloring2Colors_ [scheme];
	}

	QString AdiumStyleSource::GetMessageID (QObject *msgObj)
	{
		return QString::number (reinterpret_cast<long int> (msgObj));
	}

	void AdiumStyleSource::handleMessageDelivered ()
	{
		QWebFrame *frame = Msg2Frame_.take (sender ());
		if (!frame)
			return;

		IMessage *msg = qobject_cast<IMessage*> (sender ());
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement IMessage";
			return;
		}

		const QString& pack = Frame2Pack_ [frame];
		const QString& prefix = pack + "/Contents/Resources/Outgoing/";

		Util::QIODevice_ptr content =
				StylesLoader_->Load (QStringList (prefix + "StateSent.html"));
		QString replacement;
		if (content && content->open (QIODevice::ReadOnly))
			replacement = QString::fromUtf8 (content->readAll ());

		const QString& selector = QString ("*[id=\"delivery_state_%1\"]")
				.arg (GetMessageID (sender ()));
		QWebElement elem = frame->findFirstElement (selector);
		elem.setInnerXml (replacement);

		disconnect (sender (),
				SIGNAL (messageDelivered ()),
				this,
				SLOT (handleMessageDelivered ()));
	}

	void AdiumStyleSource::handleFrameDestroyed ()
	{
		const QObject *snd = sender ();
		for (QHash<QObject*, QWebFrame*>::iterator i = Msg2Frame_.begin ();
				i != Msg2Frame_.end (); )
			if (i.value () == snd)
				i = Msg2Frame_.erase (i);
			else
				++i;

		Frame2LastContact_.remove (static_cast<QWebFrame*> (sender ()));
		Frame2Pack_.remove (static_cast<QWebFrame*> (sender ()));
	}
}
}
}
