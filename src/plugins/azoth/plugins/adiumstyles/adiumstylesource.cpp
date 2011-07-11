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
	{
		StylesLoader_->AddGlobalPrefix ();
		StylesLoader_->AddLocalPrefix ();
	}
	
	QAbstractItemModel* AdiumStyleSource::GetOptionsModel () const
	{
		return StylesLoader_->GetSubElemModel ();
	}
	
	QString AdiumStyleSource::GetHTMLTemplate (const QString& pack,
			QObject *entryObj, QWebFrame *frame) const
	{
		if (pack != LastPack_)
		{
			Coloring2Colors_.clear ();
			LastPack_ = pack;
		}
		
		Frame2LastContact_.remove (frame);
		
		const QString& prefix = pack + "/Contents/Resources/";

		Util::QIODevice_ptr header = StylesLoader_->
				Load (QStringList (prefix + "Header.html"));
		Util::QIODevice_ptr footer = StylesLoader_->
				Load (QStringList (prefix + "Footer.html"));
		Util::QIODevice_ptr css = StylesLoader_->
				Load (QStringList (prefix + "main.css"));

		if (!header || !footer || !css)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not load HTML template for pack"
					<< pack;
			return QString ();
		}

		if (!header->open (QIODevice::ReadOnly) ||
				!footer->open (QIODevice::ReadOnly) ||
				!css->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open source files for"
					<< pack
					<< header->errorString ()
					<< footer->errorString ()
					<< css->errorString ();
			return QString ();
		}
		
		Entry2Pack_ [entryObj] = pack;
		
		QString cssStr = QString::fromUtf8 (css->readAll ());
		int pos = 0;
		QRegExp cssUrlRx ("url\\s*\\((.*)\\)");
		cssUrlRx.setMinimal (true);
		while ((pos = cssUrlRx.indexIn (cssStr, pos)) != -1)
		{
			QString url = cssUrlRx.cap (1);
			if (url.contains ("://"))
				continue;

			if (url.at (url.size () - 1) == '"')
				url.chop (1);
			if (url.at (0) == '"')
				url = url.mid (1);

			url.prepend (pack + "/Contents/Resources/");
			Util::QIODevice_ptr dev = StylesLoader_->Load (QStringList (url));
			if (dev && dev->open (QIODevice::ReadOnly))
			{
				const QString replacement = "data:image/png;base64," +
						QString::fromUtf8 (dev->readAll ().toBase64 ());
				cssStr.replace (cssUrlRx.cap (1), replacement);
			}
			pos += cssUrlRx.matchedLength ();
		}
		
		QString result;
		result = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">";
		result += "<html xmlns=\"http://www.w3.org/1999/xhtml\"><head><style type=\"text/css\">";
		result += cssStr;
		result += "</style><title/></head><body>";
		result += QString::fromUtf8 (header->readAll ());
		result += "<div id=\"Chat\"><div id=\"insert\"></div></div>";
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
		
		if (entry->GetEntryType () == ICLEntry::ETMUC)
		{
			IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (entryObj);
			if (!mucEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< entryObj
						<< "claims to be a MUC but doesn't implement IMUCEntry";
				return result;
			}

			result.replace ("%chatName%", mucEntry->GetMUCSubject ());
		}
		else
			result.replace ("%chatName%", entry->GetEntryName ());
		
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

		const QString& pack = Entry2Pack_ [msg->OtherPart ()];
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
		const QString& filename = isNextMsg ?
				"NextContent.html" :
				"Content.html";

		if (!isNextMsg)
			Frame2LastContact_ [frame] = kindaSender;

		Util::QIODevice_ptr content = StylesLoader_->
				Load (QStringList (prefix + filename));
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
		const QString& nextSelector = QString ("div[id=\"insert\"]");
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
		
		FixSrcs (frame, pack);
		
		return true;
	}
	
	void AdiumStyleSource::FrameFocused (QWebFrame*)
	{
	}
	
	void AdiumStyleSource::FixSrcs (QWebFrame *frame, const QString& pack)
	{
		QWebElementCollection col = frame->findAllElements ("*[src]");
		for (int i = 0; i < col.count (); ++i)
		{
			QString attr = col.at (i).attribute ("src");
			if (attr.contains ("://") || attr.startsWith ("data:"))
				continue;
			
			attr.prepend (pack + "/Contents/Resources/");
			
			Util::QIODevice_ptr dev = StylesLoader_->Load (QStringList (attr));
			if (dev && dev->open (QIODevice::ReadOnly))
			{
				const QString replacement = "data:image/png;base64," +
						QString::fromUtf8 (dev->readAll ().toBase64 ());
				col.at (i).setAttribute ("src", replacement);
			}
		}
	}
	
	QString AdiumStyleSource::ParseTemplate (QString templ, const QString& base,
			QWebFrame*, QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		const bool isHighlightMsg = info.IsHighlightMsg_;
		
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		const bool in = msg->GetDirection () == IMessage::DIn;

		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		IAccount *acc = qobject_cast<IAccount*> (other->GetParentAccount ());
		
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
		QImage image = in ? other->GetAvatar () : QImage ();
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
				qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolName ());
		
		// TODO have a setting for highlights.
		// %backgroundcolor{X}%
		QRegExp bgColorRx ("%textbackgroundcolor\\{([^}]*)\\}%");
		pos = 0;
		while ((pos = bgColorRx.indexIn (templ, pos)) != -1)
			templ.replace (pos, bgColorRx.matchedLength (),
					"inherit");
			
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
		const QString& nickColor = isHighlightMsg ?
				"red" :
				Proxy_->GetNickColor (senderNick, Coloring2Colors_ ["hash"]);
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
		{
			// damn ugly workaround for a bug that makes things like
			// "<div/>ping" parsed as "<div>ping</div>".
			// QtWebKit sucks.
			const int endPos = templ.indexOf ('>', templ.indexOf ("%stateElementId%"));
			if (templ.at (endPos - 1) == '/')
				templ.replace (endPos - 1, 2, "></div>");

			templ.replace ("%stateElementId%", "delivery_state_" + GetMessageID (msgObj));
		}
		
		// %message%
		IRichTextMessage *richMsg = qobject_cast<IRichTextMessage*> (msgObj);
		QString body;
		if (richMsg && info.UseRichTextBody_)
			body = richMsg->GetRichBody ();
		if (body.isEmpty ())
			body = msg->GetBody ();
		
		body = Proxy_->FormatBody (body, msgObj);
		
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

		const QString& pack = Entry2Pack_ [msg->OtherPart ()];
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
		
		FixSrcs (frame, pack);
		
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
	}
}
}
}
