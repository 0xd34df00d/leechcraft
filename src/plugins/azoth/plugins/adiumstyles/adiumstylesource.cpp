/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/irichtextmessage.h>
#include <interfaces/azoth/iadvancedmessage.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/iextselfinfoaccount.h>
#include "packproxymodel.h"
#include "plistparser.h"

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
		if (srcPack.contains ('/'))
		{
			const auto& split = srcPack.split ('/', QString::SkipEmptyParts);
			return GetBaseURL (split.value (0));
		}

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

		return QUrl::fromLocalFile (QFileInfo (path).absolutePath () + '/');
	}

	namespace
	{
		void FixSelfClosing (QString& str)
		{
			QRegExp rx ("<div([^>]*)/>");
			rx.setMinimal (true);
			str.replace (rx, "<div\\1></div>");
		}
	}

	QString AdiumStyleSource::GetHTMLTemplate (const QString& srcPack,
			const QString& varCss, QObject *entryObj, QWebFrame *frame) const
	{
		if (srcPack.contains ('/'))
		{
			const auto& split = srcPack.split ('/', QString::SkipEmptyParts);
			return GetHTMLTemplate (split.value (0), split.value (1), entryObj, frame);
		}

		if (srcPack != LastPack_)
		{
			Coloring2Colors_.clear ();
			Frame2LastContact_.clear ();
			LastPack_ = srcPack;

			StylesLoader_->FlushCache ();
		}

		connect (frame,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleFrameDestroyed ()),
				Qt::UniqueConnection);

		const QString& pack = PackProxyModel_->GetOrigName (srcPack);

		Frame2Pack_ [frame] = pack;
		Frame2LastContact_.remove (frame);

		const QString& prefix = pack + "/Contents/Resources/";

		Util::QIODevice_ptr header = StylesLoader_->
				Load (QStringList (prefix + "Header.html"));
		Util::QIODevice_ptr footer = StylesLoader_->
				Load (QStringList (prefix + "Footer.html"));
		Util::QIODevice_ptr css = StylesLoader_->
				Load (QStringList (prefix + "main.css"));
		Util::QIODevice_ptr tmpl = StylesLoader_->
				Load (QStringList (prefix + "Template.html"));

		if ((header && !header->open (QIODevice::ReadOnly)) ||
				(footer && !footer->open (QIODevice::ReadOnly)) ||
				(css && !css->open (QIODevice::ReadOnly)))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open source files for"
					<< pack
					<< (header ? header->errorString () : "empty header")
					<< (footer ? footer->errorString () : "empty footer")
					<< (css ? css->errorString () : "empty css");
			return QString ();
		}

		const QUrl& baseUrl = GetBaseURL (srcPack);
		QString varCssStr;
		if (!varCss.isEmpty ())
			varCssStr = "Variants/" + varCss + ".css";
		else
			varCssStr = "main.css";

		QString result;
		if (tmpl && tmpl->open (QIODevice::ReadOnly))
			result = QString::fromUtf8 (tmpl->readAll ());
		else
		{
			QFile file (":/plugins/azoth/plugins/adiumstyles/resources/html/Template.html");
			file.open (QIODevice::ReadOnly);
			result = QString::fromUtf8 (file.readAll ());
		}

		QMap<QString, QString> map;
		map ["Path"] = baseUrl.toString ();
		map ["CSS"] = "@import url( \"main.css\" );";
		if (!varCssStr.isEmpty ())
			map ["VariantCSS"] = baseUrl.resolved (QUrl (varCssStr)).toString ();
		map ["Header"] = header ? header->readAll () : QString ();
		map ["Footer"] = footer ? footer->readAll () : QString ();
		PercentTemplate (result, map);

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entryObj
					<< "doesn't implement ICLEntry, but proceeding...";
			return result;
		}

		ParseGlobalTemplate (result, entry);
		FixSelfClosing (result);

		return result;
	}

	namespace
	{
		IMessage::Direction GetMsgDirection (IMessage *msg)
		{
			if (msg->GetMessageType () != IMessage::MTMUCMessage)
				return msg->GetDirection ();

			IMUCEntry *muc = qobject_cast<IMUCEntry*> (msg->ParentCLEntry ());
			ICLEntry *part = qobject_cast<ICLEntry*> (msg->OtherPart ());
			return muc->GetNick () == part->GetEntryName () ?
					IMessage::DOut :
					IMessage::DIn;
		}
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

		connect (msgObj,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleMessageDestroyed ()));

		const bool in = GetMsgDirection (msg) == IMessage::DIn;

		QObject *kindaSender = in ? msg->OtherPart () : reinterpret_cast<QObject*> (42);

		const bool isSlashMe = msg->GetBody ()
				.trimmed ().startsWith ("/me ");
		const bool alwaysNotNext = isSlashMe ||
				!(msg->GetMessageType () == IMessage::MTChatMessage || msg->GetMessageType () == IMessage::MTMUCMessage);
		const bool isNextMsg = !alwaysNotNext &&
				Frame2LastContact_.contains (frame) &&
				kindaSender == Frame2LastContact_ [frame];

		const QString& prefix = pack + "/Contents/Resources/" +
				(in || isSlashMe ? "Incoming" : "Outgoing") +
				'/';

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
		else if (!isNextMsg && !alwaysNotNext)
			Frame2LastContact_ [frame] = kindaSender;
		else if (alwaysNotNext)
			Frame2LastContact_.remove (frame);

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

		QString templ = QString::fromUtf8 (content->readAll ());
		FixSelfClosing (templ);
		QString bodyS = ParseMsgTemplate (templ, prefix, frame, msgObj, info);
		QString body;
		body.reserve (bodyS.size () * 1.2);
		for (int i = 0, size = bodyS.size (); i < size; ++i)
		{
			switch (bodyS.at (i).unicode ())
			{
			case L'\"':
				body += "\\\"";
				break;
			case L'\n':
				body += "\\n";
				break;
			case L'\t':
				body += "\\t";
				break;
			case L'\\':
				body += "\\\\";
				break;
			case L'\r':
				body += "\\r";
				break;
			default:
				body += bodyS.at (i);
				break;
			}
		}

		const QString& command = isNextMsg ? "appendNextMessage(\"%1\");" : "appendMessage(\"%1\");";
		frame->evaluateJavaScript (command.arg (body));

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

	QStringList AdiumStyleSource::GetVariantsForPack (const QString& pack)
	{
		QStringList result;

		const QString& origName = PackProxyModel_->GetOrigName (pack);
		if (!StylesLoader_->GetPath (QStringList (origName + "/Contents/Resources/main.css")).isEmpty ())
			result << "";

		const QString& suff = origName + "/Contents/Resources/Variants/";
		const QString& path = StylesLoader_->GetPath (QStringList (suff));
		if (!path.isEmpty ())
			Q_FOREACH (const QString& variant, QDir (path).entryList (QStringList ("*.css")))
			{
				QString hrVar = variant;
				hrVar.chop (4);
				result << hrVar;
			}

		return result;
	}

	void AdiumStyleSource::PercentTemplate (QString& result, const QMap<QString, QString>& map) const
	{
		QRegExp rx ("(?:%@){1}");
		const int count = result.count (rx);

		QStringList rpls (map ["Path"]);
		if (count == 5)
			rpls << map ["CSS"];
		rpls << map ["VariantCSS"]
			<< map ["Header"]
			<< map ["Footer"];

		int i = 0;
		int pos = 0;
		while ((pos = rx.indexIn (result, pos)) != -1 && i < rpls.size ())
		{
			result.replace (pos, 2, rpls [i]);
			pos += rpls [i].length ();
			i++;
		}
	}

	void AdiumStyleSource::ParseGlobalTemplate (QString& result, ICLEntry *entry) const
	{
		auto acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		auto extSelf = qobject_cast<IExtSelfInfoAccount*> (entry->GetParentAccount ());

		ICLEntry *selfEntry = extSelf ?
				qobject_cast<ICLEntry*> (extSelf->GetSelfContact ()) :
				0;

		result.replace ("%chatName%", entry->GetEntryName ());
		result.replace ("%sourceName%", acc->GetOurNick ());
		result.replace ("%destinationName%", entry->GetHumanReadableID ());
		result.replace ("%destinationDisplayName%", entry->GetEntryName ());

		const QImage& defAvatar = GetDefaultAvatar ();

		auto safeIconReplace = [&result] (const QString& pattern,
				QImage px, const QImage& def = QImage ())
		{
			if (result.contains (pattern))
				result.replace (pattern, Util::GetAsBase64Src (px.isNull () ? def : px));
		};
		safeIconReplace ("%incomingIconPath%",
				entry->GetAvatar (), defAvatar);
		safeIconReplace ("%outgoingIconPath%",
				selfEntry ? selfEntry->GetAvatar () : defAvatar, defAvatar);

		result.replace ("%timeOpened%",
				QDateTime::currentDateTime ().toString (Qt::SystemLocaleLongDate));

		QRegExp openedRx ("%timeOpened\\{(.*?)\\}%");
		int pos = 0;
		while ((pos = openedRx.indexIn (result, pos)) != -1)
			result.replace (pos, openedRx.matchedLength (),
					QDateTime::currentDateTime ().toString (openedRx.cap (1)));

		result.replace ("%dateOpened%", QDate::currentDate ().toString (Qt::SystemLocaleLongDate));
	}

	QString AdiumStyleSource::ParseMsgTemplate (QString templ, const QString& base,
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
				image = self->GetSelfAvatar ();
		}

		if (image.isNull ())
			image = QImage (StylesLoader_->GetPath (QStringList (base + "buddy_icon.png")));
		if (image.isNull ())
			image = GetDefaultAvatar ();
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

	QImage AdiumStyleSource::GetDefaultAvatar () const
	{
		const QString& defAvatarName = Proxy_->GetSettingsManager ()->
				property ("SystemIcons").toString () + "/default_avatar";
		auto sysLdr = Proxy_->GetResourceLoader (IProxyObject::PRLSystemIcons);
		return sysLdr->LoadPixmap (defAvatarName).toImage ();
	}

	PListParser_ptr AdiumStyleSource::GetPListParser (const QString& pack) const
	{
		if (PListParsers_.contains (pack))
			return PListParsers_ [pack];

		auto plist = std::make_shared<PListParser> ();
		try
		{
			const QString& name = pack + "/Contents/Info.plist";
			const auto& path = StylesLoader_->GetPath (QStringList (name));
			plist->Parse (path);
		}
		catch (const PListParseError& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing PList for"
					<< pack
					<< e.GetStr ();
			return PListParser_ptr ();
		}
		PListParsers_ [pack] = plist;
		return plist;
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

	void AdiumStyleSource::handleMessageDestroyed ()
	{
		Msg2Frame_.remove (sender ());
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
