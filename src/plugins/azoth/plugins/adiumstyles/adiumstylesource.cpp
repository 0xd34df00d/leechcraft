/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "adiumstylesource.h"
#include <QWebEnginePage>
#include <QtDebug>
#include <util/sys/resourceloader.h>
#include <util/sll/qtutil.h>
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

namespace LC
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
			const auto& split = srcPack.split ('/', Qt::SkipEmptyParts);
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

		void FormatTime (QString& templ, const QString& elem, const QDateTime& dt)
		{
			QStringMatcher timeMatcher ("%" + elem + "{");
			int pos = 0;
			while ((pos = timeMatcher.indexIn (templ, pos)) != -1)
			{
				const auto start = pos;
				pos += timeMatcher.pattern ().size ();
				const auto end = templ.indexOf ('}', pos);
				if (end == -1)
					break;

				const auto& formatStr = templ.mid (pos, end - pos);
				const auto& formatted = dt.toString (formatStr);
				templ.replace (start, end - start + 2, formatted);
			}
		}

		void ReplaceIcon (QString& result, const QString& pattern, const QString& entryId)
		{
			if (result.contains (pattern))
				result.replace (pattern, "azoth://avatar/" + entryId.toUtf8 ().toBase64 ());
		}

		void ParseGlobalTemplate (QString& result, ICLEntry *entry)
		{
			auto acc = entry->GetParentAccount ();
			auto extSelf = qobject_cast<IExtSelfInfoAccount*> (acc->GetQObject ());

			ICLEntry *selfEntry = extSelf ?
					qobject_cast<ICLEntry*> (extSelf->GetSelfContact ()) :
					0;

			result.replace ("%chatName%", entry->GetEntryName ());
			result.replace ("%sourceName%", acc->GetOurNick ());
			result.replace ("%destinationName%", entry->GetHumanReadableID ());
			result.replace ("%destinationDisplayName%", entry->GetEntryName ());

			ReplaceIcon (result, "%incomingIconPath%", entry->GetEntryID ());
			ReplaceIcon (result, "%outgoingIconPath%", selfEntry ? selfEntry->GetEntryID () : "");

			const auto& now = QDateTime::currentDateTime ();
			QLocale loc;
			result.replace ("%timeOpened%", loc.toString (now.time (), QLocale::LongFormat));
			result.replace ("%dateOpened%", loc.toString (now.date (), QLocale::LongFormat));
			FormatTime (result, "timeOpened", now);
		}
	}

	QString AdiumStyleSource::GetHTMLTemplate (const QString& srcPack,
			const QString& varCss, QObject *entryObj, QWebEnginePage *frame) const
	{
		if (srcPack.contains ('/'))
		{
			const auto& split = srcPack.split ('/', Qt::SkipEmptyParts);
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

		auto insensitive = [&prefix] (const QString& name)
		{
			return QStringList { prefix + name, prefix + name.toLower () };
		};
		const auto& header = StylesLoader_->Load (insensitive ("Header.html"));
		const auto& footer = StylesLoader_->Load (insensitive ("Footer.html"));
		const auto& css = StylesLoader_->Load ({ prefix + "main.css" });
		const auto& tmpl = StylesLoader_->Load (insensitive ("Template.html"));

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
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "failed to open resource file"
						<< file.fileName ()
						<< file.errorString ();
				return {};
			}
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

		auto colors = StylesLoader_->Load (insensitive ("Incoming/SenderColors.txt"));
		if (colors && colors->open (QIODevice::ReadOnly))
			for (const auto& colorName : QString::fromUtf8 (colors->readAll ()).split (":"))
				Coloring2Colors_ ["hash"] << QColor (colorName);

		ParseGlobalTemplate (result, entry);
		FixSelfClosing (result);

		return result;
	}

	namespace
	{
		IMessage::Direction GetMsgDirection (IMessage *msg)
		{
			if (msg->GetMessageType () != IMessage::Type::MUCMessage)
				return msg->GetDirection ();

			const auto muc = qobject_cast<IMUCEntry*> (msg->ParentCLEntry ());
			const auto part = qobject_cast<ICLEntry*> (msg->OtherPart ());

			if (!muc || !part)
				return IMessage::Direction::In;

			return muc->GetNick () == part->GetEntryName () ?
					IMessage::Direction::Out :
					IMessage::Direction::In;
		}
	}

	namespace
	{
		QString GetMessageID (const QObject *msgObj)
		{
			return QString::number (reinterpret_cast<uintptr_t> (msgObj));
		}

		QString MakeStateSetterJS (Util::ResourceLoader& loader,
				const QObject *msgObj, const QString& prefix, const QString& stateFile)
		{
			const auto& stateContent = loader.Load ({ prefix + stateFile });
			QString replacement;
			if (stateContent && stateContent->open (QIODevice::ReadOnly))
				replacement = QString::fromUtf8 (stateContent->readAll ());
			return QStringLiteral (R"(
					(() => {
						let elem = document.querySelector("*[id='delivery_state_%1']");
						if (elem)
							elem.innerHTML = "%2";
					}) ();
			)").arg (GetMessageID (msgObj), replacement);
		}
	}

	bool AdiumStyleSource::AppendMessage (QWebEnginePage *frame, QObject *msgObj, const ChatMsgAppendInfo& info)
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

		const bool in = GetMsgDirection (msg) == IMessage::Direction::In;

		QObject *kindaSender = in ? msg->OtherPart () : reinterpret_cast<QObject*> (42);

		const bool isSlashMe = msg->GetBody ().trimmed ().startsWith ("/me ");
		const bool alwaysNotNext = isSlashMe ||
				!(msg->GetMessageType () == IMessage::Type::ChatMessage || msg->GetMessageType () == IMessage::Type::MUCMessage);
		const bool isNextMsg = !alwaysNotNext &&
				Frame2LastContact_.contains (frame) &&
				kindaSender == Frame2LastContact_ [frame];

		const QString& root = pack + "/Contents/Resources/";
		const QString& prefix = root +
				(in || isSlashMe ? "Incoming" : "Outgoing") +
				'/';

		QString filename;
		if ((msg->GetMessageType () == IMessage::Type::ChatMessage ||
					msg->GetMessageType () == IMessage::Type::MUCMessage) &&
				!isSlashMe)
			filename = isNextMsg ?
					"NextContent.html" :
					"Content.html";
		else
			filename = "Action.html";

		if (msg->GetMessageType () != IMessage::Type::MUCMessage &&
				msg->GetMessageType () != IMessage::Type::ChatMessage)
			Frame2LastContact_.remove (frame);
		else if (!isNextMsg && !alwaysNotNext)
			Frame2LastContact_ [frame] = kindaSender;
		else if (alwaysNotNext)
			Frame2LastContact_.remove (frame);

		QStringList templCands;
		templCands << (prefix + filename);
		if (filename == "Action.html")
		{
			templCands << (root + "Status.html");
			templCands << (root + "status.html");
		}
		if (isNextMsg)
			templCands << (root + "NextContent.html");
		templCands << (root + "Content.html");

		Util::QIODevice_ptr content;
		while (!content && !templCands.isEmpty ())
			content = StylesLoader_->Load (templCands.takeFirst ());
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

		const auto& command = isNextMsg ?
				QStringLiteral ("appendNextMessage(\"%1\");") :
				QStringLiteral ("appendMessage(\"%1\");");
		auto js = command.arg (body);

		if (templ.contains ("%stateElementId%"))
		{
			const auto advMsg = qobject_cast<IAdvancedMessage*> (msgObj);
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

			js += MakeStateSetterJS (*StylesLoader_, msgObj, prefix, fname);
		}

		frame->runJavaScript (js);

		return true;
	}

	void AdiumStyleSource::FrameFocused (QWebEnginePage*)
	{
	}

	QStringList AdiumStyleSource::GetVariantsForPack (const QString& pack)
	{
		QStringList result;

		const auto& origName = PackProxyModel_->GetOrigName (pack);
		if (!StylesLoader_->GetPath ({ origName + "/Contents/Resources/main.css" }).isEmpty ())
			result << "";

		const auto& suff = origName + "/Contents/Resources/Variants/";
		const auto& path = StylesLoader_->GetPath ({ suff });
		if (!path.isEmpty ())
			for (auto&& variant : QDir { path }.entryList ({ "*.css" }))
			{
				variant.chop (4);
				result << variant;
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

	void AdiumStyleSource::SubstituteUserIcon (QString& templ,
			const QString& base, bool in, ICLEntry *other, IAccount *acc)
	{
		const auto iha = qobject_cast<IHaveAvatars*> (other->GetQObject ());
		if (in && iha && iha->HasAvatar ())
		{
			ReplaceIcon (templ, "%userIconPath", other->GetEntryID ());
			return;
		}

		QImage image;
		if (!in && acc)
		{
			const auto self = qobject_cast<IExtSelfInfoAccount*> (acc->GetQObject ());
			if (const auto selfEntry = self ? self->GetSelfContact () : nullptr)
				ReplaceIcon (templ, "%userIconPath%",
						qobject_cast<ICLEntry*> (selfEntry)->GetEntryID ());
		}

		if (image.isNull ())
			image = QImage (StylesLoader_->GetPath (QStringList (base + "buddy_icon.png")));
		if (image.isNull ())
			image = Proxy_->GetDefaultAvatar ();
		if (image.isNull ())
			qWarning () << Q_FUNC_INFO
					<< "image is still null, though tried"
					<< base + "buddy_icon.png";

		templ.replace ("%userIconPath%", Util::GetAsBase64Src (image));
	}

	QString AdiumStyleSource::ParseMsgTemplate (QString templ, const QString& base,
			QWebEnginePage*, QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		const bool isHighlightMsg = info.IsHighlightMsg_;
		auto& formatter = Proxy_->GetFormatterProxy ();

		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		const bool in = msg->GetDirection () == IMessage::Direction::In;

		ICLEntry *other = 0;
		switch (msg->GetMessageType ())
		{
		case IMessage::Type::ChatMessage:
		case IMessage::Type::MUCMessage:
		case IMessage::Type::StatusMessage:
			other = qobject_cast<ICLEntry*> (msg->OtherPart ());
			break;
		case IMessage::Type::EventMessage:
		case IMessage::Type::ServiceMessage:
			other = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());
			break;
		}

		if (!other)
		{
			qWarning () << Q_FUNC_INFO
					<< "null other part, gonna fail:"
					<< static_cast<int> (msg->GetMessageType ())
					<< msg->GetBody ()
					<< msg->OtherPart ()
					<< msg->ParentCLEntry ();
			return templ;
		}

		auto acc = other->GetParentAccount ();

		if (!acc && msg->ParentCLEntry ())
		{
			if (const auto entry = qobject_cast<ICLEntry*> (msg->ParentCLEntry ()))
				acc = entry->GetParentAccount ();
		}

		if (!acc && !in)
		{
			qWarning () << Q_FUNC_INFO
					<< "no account for outgoing message, that sucks"
					<< static_cast<int> (msg->GetMessageType ())
					<< msg->OtherPart ()
					<< msg->ParentCLEntry ();
			return templ;
		}

		QString senderNick = in ? other->GetEntryName () : acc->GetOurNick ();
		if (in &&
				msg->GetMessageType () == IMessage::Type::ChatMessage &&
				Proxy_->GetSettingsManager ()->property ("ShowNormalChatResources").toBool ())
		{
			const auto& resource = msg->GetOtherVariant ();
			if (!resource.isEmpty ())
				senderNick += '/' + resource;
		}

		// %time%
		templ.replace ("%time%", msg->GetDateTime ().time ().toString ());

		// %time{X}%
		FormatTime (templ, "time", msg->GetDateTime ());

		// %messageDirection%
		templ.replace ("%messageDirection%", "ltr");

		// %userIconPath%
		if (templ.contains ("%userIconPath%"))
			SubstituteUserIcon (templ, base, in, other, acc);

		// %senderScreenName%
		templ.replace ("%senderScreenName%",
				in ? other->GetHumanReadableID () : acc->GetAccountName ());

		// %sender%
		templ.replace ("%sender%",
				formatter.FormatNickname (senderNick, msgObj, "%senderColor%"));

		// %service%
		templ.replace ("%service%",
				acc ?
					qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolName () :
					QString ());

		// %textbackgroundcolor{X}%
		QRegExp bgColorRx ("%textbackgroundcolor\\{([^}]*)\\}%");
		int pos = 0;
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
			Coloring2Colors_ ["hash"] = formatter.GenerateColors ("hash", {});

		// %senderColor%
		const auto& colors = Coloring2Colors_ ["hash"];
		const auto& nickColor = formatter.GetNickColor (senderNick, colors);
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
			body = Proxy_->GetFormatterProxy ().EscapeBody (msg->GetBody (), msg->GetEscapePolicy ());

		if (body.startsWith ("/me "))
			body = QString ("* %1 %2")
					.arg (senderNick)
					.arg (body.mid (4));

		body = formatter.FormatBody (body, msgObj, colors);

		if (isHighlightMsg && !hasHighBackground)
			body = "<span style=\"color:" + highColor +
					"\">" + body + "</span>";

		templ.replace ("%message%", body);

		return templ;
	}

	void AdiumStyleSource::handleMessageDelivered ()
	{
		const auto frame = Msg2Frame_.take (sender ());
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

		frame->runJavaScript (MakeStateSetterJS (*StylesLoader_, sender (), prefix, "StateSent.html"));

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
		for (auto i = Msg2Frame_.begin (); i != Msg2Frame_.end (); )
			if (i.value () == snd)
				i = Msg2Frame_.erase (i);
			else
				++i;

		Frame2LastContact_.remove (static_cast<QWebEnginePage*> (sender ()));
		Frame2Pack_.remove (static_cast<QWebEnginePage*> (sender ()));
	}
}
}
}
