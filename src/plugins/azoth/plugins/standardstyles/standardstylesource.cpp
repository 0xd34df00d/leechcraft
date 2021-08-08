/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "standardstylesource.h"
#include <QTextDocument>
#include <QApplication>
#include <QPalette>
#include <QtDebug>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <util/sys/resourceloader.h>
#include <util/sll/qtutil.h>
#include <util/util.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iadvancedmessage.h>
#include <interfaces/azoth/irichtextmessage.h>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/imucentry.h>
#include <interfaces/azoth/iproxyobject.h>

namespace LC::Azoth::StandardStyles
{
	StandardStyleSource::StandardStyleSource (IProxyObject *proxy, QObject *parent)
	: QObject (parent)
	, StylesLoader_ (new Util::ResourceLoader (QStringLiteral ("azoth/styles/standard/"), this))
	, Proxy_ (proxy)
	{
		StylesLoader_->AddGlobalPrefix ();
		StylesLoader_->AddLocalPrefix ();

		const auto stylesCacheSizeKb = 256;
		StylesLoader_->SetCacheParams (stylesCacheSizeKb, 0);
	}

	QAbstractItemModel* StandardStyleSource::GetOptionsModel () const
	{
		return StylesLoader_->GetSubElemModel ();
	}

	QUrl StandardStyleSource::GetBaseURL (const QString& pack) const
	{
		const auto& path = StylesLoader_->GetPath (QStringList (pack + "/viewcontents.html"));
		if (path.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty base URL for"
					<< pack;
			return {};
		}

		return QUrl::fromLocalFile (QFileInfo (path).absolutePath ());
	}

	QString StandardStyleSource::GetHTMLTemplate (const QString& pack,
			const QString&, QObject *entryObj, QWebEnginePage*) const
	{
		if (pack != LastPack_)
		{
			Coloring2Colors_.clear ();
			Frame2Colors_.clear ();
			LastPack_ = pack;
			StylesLoader_->FlushCache ();
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		Util::QIODevice_ptr dev;
		if (entry && entry->GetEntryType () == ICLEntry::EntryType::MUC)
			dev = StylesLoader_->Load (QStringList (pack + "/viewcontents.muc.html"));
		if (!dev)
			dev = StylesLoader_->Load (QStringList (pack + "/viewcontents.html"));

		if (!dev)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not load HTML template for pack"
					<< pack;
			return {};
		}

		if (!dev->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open source file for"
					<< pack + "/viewcontents.html"
					<< dev->errorString ();
			return {};
		}

		QString data = QString::fromUtf8 (dev->readAll ());
		data.replace ("BACKGROUNDCOLOR"_ql,
				QApplication::palette ().color (QPalette::Base).name ());
		data.replace ("FOREGROUNDCOLOR"_ql,
				QApplication::palette ().color (QPalette::Text).name ());
		data.replace ("LINKCOLOR"_ql,
				QApplication::palette ().color (QPalette::Link).name ());
		data.replace ("HIGHLIGHTCOLOR"_ql,
				Proxy_->GetSettingsManager ()->
						property ("HighlightColor").toString ());
		return data;
	}

	namespace
	{
		QString WrapNickPart (const QString& part,
				const QString& color, IMessage::Type type)
		{
			const QString& pre = type == IMessage::Type::MUCMessage ?
					"<span class='nickname' style='color: " + color + "'>" :
					QStringLiteral ("<span class='nickname'>");
			return pre + part.toHtmlEscaped () + "</span>";
		}

		QString GetMessageID (QObject *msgObj)
		{
			return QString::number (std::bit_cast<uintptr_t> (msgObj));
		}
	}

	bool StandardStyleSource::AppendMessage (QWebEnginePage *frame,
			QObject *msgObj, const ChatMsgAppendInfo& info)
	{
		auto& colors = Frame2Colors_ [frame];
		if (colors.isEmpty ())
			colors = CreateColors (frame);

		auto& formatter = Proxy_->GetFormatterProxy ();

		const QString& msgId = GetMessageID (msgObj);

		const auto msg = qobject_cast<IMessage*> (msgObj);
		const auto other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		QString entryName = other ?
				other->GetEntryName ().toHtmlEscaped () :
				QString ();
		if (msg->GetMessageType () == IMessage::Type::ChatMessage &&
				Proxy_->GetSettingsManager ()->property ("ShowNormalChatResources").toBool () &&
				!msg->GetOtherVariant ().isEmpty ())
			entryName += '/' + msg->GetOtherVariant ();

		connect (msgObj,
				&QObject::destroyed,
				this,
				&StandardStyleSource::HandleMessageDestroyed,
				Qt::UniqueConnection);

		const auto advMsg = qobject_cast<IAdvancedMessage*> (msgObj);
		if (msg->GetDirection () == IMessage::Direction::Out &&
				advMsg &&
				!advMsg->IsDelivered ())
		{
			connect (msgObj,
					SIGNAL (messageDelivered ()),
					this,
					SLOT (handleMessageDelivered ()));
			Msg2Frame_ [msgObj] = frame;
		}

		const auto& nickColor = formatter.GetNickColor (entryName, colors);

		const auto richMsg = qobject_cast<IRichTextMessage*> (msgObj);
		QString body;
		if (richMsg && info.UseRichTextBody_)
			body = richMsg->GetRichBody ();
		if (body.isEmpty ())
			body = msg->GetEscapedBody ();

		body = formatter.FormatBody (body, msg->GetQObject (), colors);

		const auto dateBegin = QStringLiteral ("<span class='datetime'>");
		const auto dateEnd = QStringLiteral ("</span>");

		const auto slashMeMarker = "/me "_ql;
		const auto leechcraftMarker = "/leechcraft "_ql;

		const auto azothSettings = Proxy_->GetSettingsManager ();
		const auto& preNick = WrapNickPart (azothSettings->property ("PreNickText").toString (),
				nickColor, msg->GetMessageType ());
		const auto& postNick = WrapNickPart (azothSettings->property ("PostNickText").toString (),
				nickColor, msg->GetMessageType ());

		QString divClass;
		QString statusIconName;

		QString string = dateBegin + '[' +
				formatter.FormatDate (msg->GetDateTime (), msg->GetQObject ()) +
				']' + dateEnd;
		string.append (' ');
		switch (msg->GetDirection ())
		{
		case IMessage::Direction::In:
		{
			switch (msg->GetMessageType ())
			{
			case IMessage::Type::ChatMessage:
				divClass = msg->GetDirection () == IMessage::Direction::In ?
						"msgin"_ql :
						"msgout"_ql;
				[[fallthrough]];
			case IMessage::Type::MUCMessage:
			{
				statusIconName = "notification_chat_receive"_ql;

				if (body.startsWith (slashMeMarker))
				{
					body = body.mid (slashMeMarker.size ());
					string.append ("* ");
					string.append (formatter.FormatNickname (entryName, msg->GetQObject (), nickColor));
					string.append (' ');
					divClass = "slashmechatmsg"_ql;
				}
				else
				{
					string.append (preNick);
					string.append (formatter.FormatNickname (entryName, msg->GetQObject (), nickColor));
					string.append (postNick);
					string.append (' ');
					if (divClass.isEmpty ())
						divClass = info.IsHighlightMsg_ ?
								"highlightchatmsg"_ql :
								"chatmsg"_ql;
				}
				break;
			}
			case IMessage::Type::EventMessage:
				statusIconName = "notification_chat_info"_ql;
				string.append ("! ");
				divClass = "eventmsg"_ql;
				break;
			case IMessage::Type::StatusMessage:
				statusIconName = "notification_chat_info"_ql;
				string.append ("* ");
				divClass = "statusmsg"_ql;
				break;
			case IMessage::Type::ServiceMessage:
				statusIconName = "notification_chat_info"_ql;
				string.append ("* ");
				divClass = "servicemsg"_ql;
				break;
			}
			break;
		}
		case IMessage::Direction::Out:
		{
			statusIconName = "notification_chat_send"_ql;
			if (advMsg && advMsg->IsDelivered ())
				statusIconName = "notification_chat_delivery_ok"_ql;

			const auto entry = other ? qobject_cast<IMUCEntry*> (other->GetParentCLEntryObject ()) : nullptr;
			const auto& nick = entry ?
					entry->GetNick () :
					(other ?
							other->GetParentAccount ()->GetOurNick () :
							QString {});
			if (body.startsWith (leechcraftMarker))
			{
				body = body.mid (leechcraftMarker.size ());
				string.append ("* ");
			}
			else if (body.startsWith (slashMeMarker) &&
					msg->GetMessageType () != IMessage::Type::MUCMessage)
			{
				body = body.mid (slashMeMarker.size ());
				string.append ("* ");
				string.append (formatter.FormatNickname (nick, msg->GetQObject (), nickColor));
				string.append (' ');
				divClass = "slashmechatmsg"_ql;
			}
			else
			{
				string.append (preNick);
				string.append (formatter.FormatNickname (nick, msg->GetQObject (), nickColor));
				string.append (postNick);
				string.append (' ');
			}
			if (divClass.isEmpty ())
				divClass = "msgout"_ql;
			break;
		}
		}

		string.prepend (QStringLiteral ("<img src='%1' style='max-width: 1em; max-height: 1em;' id='%2' class='deliveryStatusIcon' />")
				.arg (GetStatusImage (statusIconName), msgId));
		string.append (body);

		QString js;
		if (msg->GetMessageType () == IMessage::Type::ChatMessage ||
				msg->GetMessageType () == IMessage::Type::MUCMessage)
		{
			const auto isRead = Proxy_->IsMessageRead (msgObj);
			if (!info.IsActiveChat_ &&
				!isRead && IsLastMsgRead_.value (frame, false))
			{
				js += R"(
						(() => {
							let hr = document.querySelector("hr[class='lastSeparator']");
							if (hr)
								document.body.appendChild(hr);
							else
								document.body.innerHTML += "<hr class='lastSeparator' />";
						}) ();
						)"_ql;
			}
			IsLastMsgRead_ [frame] = isRead;
		}

		js += R"(
				document.body.insertAdjacentHTML("beforeend", "<div class='%1' style='word-wrap: break-word;'>%2</div>");
				true;
				)"_ql
				.arg (divClass, string);

		frame->runJavaScript (js);

		return true;
	}

	void StandardStyleSource::FrameFocused (QWebEnginePage *frame)
	{
		IsLastMsgRead_ [frame] = true;
	}

	QStringList StandardStyleSource::GetVariantsForPack (const QString&)
	{
		return {};
	}

	void StandardStyleSource::PrepareColors (QWebEngineView *view)
	{
		const auto page = view->page ();
		Frame2Colors_ [page] = CreateColors (page);
		connect (page,
				&QObject::destroyed,
				this,
				&StandardStyleSource::HandleFrameDestroyed,
				Qt::UniqueConnection);
	}

	namespace
	{
		QColor UnRgb (QString str)
		{
			str.remove (' ');
			str.remove (QStringLiteral ("rgb("));
			str.remove (')');
			const auto& vals = str.splitRef (',', Qt::SkipEmptyParts);

			QColor color;
			if (vals.size () == 3)
				color.setRgb (vals.value (0).toInt (),
						vals.value (1).toInt (), vals.value (2).toInt ());
			return color;
		}
	}

	QList<QColor> StandardStyleSource::CreateColors (QWebEnginePage *frame)
	{
		QEventLoop loop;

		static const auto js = QStringLiteral (R"(
				(() => {
					let coloring = document.querySelector("meta[name=coloring]")?.content;
					let bgColor = window.getComputedStyle(document.body)['background-color'];
					return { coloring, bgColor };
				}) ();
				)");

		QString scheme;
		QColor bgColor;
		frame->runJavaScript (js,
				[&] (const QVariant& var)
				{
					const auto& varMap = var.toMap ();
					scheme = varMap [QStringLiteral ("coloring")].toString ();
					bgColor = UnRgb (varMap [QStringLiteral ("bgColor")].toString ());
					loop.quit ();
				});

		loop.exec (QEventLoop::ExcludeUserInputEvents);

		const auto& mangledScheme = scheme + bgColor.name ();

		if (!Coloring2Colors_.contains (mangledScheme))
			Coloring2Colors_ [mangledScheme] = Proxy_->GetFormatterProxy ().GenerateColors (scheme, bgColor);

		return Coloring2Colors_ [mangledScheme];
	}

	QString StandardStyleSource::GetStatusImage (const QString& iconName)
	{
		const auto& fullName = Proxy_->GetSettingsManager ()->property ("SystemIcons").toString () + '/' + iconName;
		const auto& statusIconPath = Proxy_->GetResourceLoader (IProxyObject::PRLSystemIcons)->GetIconPath (fullName);
		return Util::GetAsBase64Src (QImage { statusIconPath });
	}

	void StandardStyleSource::HandleMessageDestroyed (QObject *msgObj)
	{
		Msg2Frame_.remove (msgObj);
	}

	void StandardStyleSource::handleMessageDelivered ()
	{
		const auto frame = Msg2Frame_.take (sender ());
		if (!frame)
			return;

		const auto& msgId = GetMessageID (sender ());
		static const auto js = QStringLiteral (R"(
				(() => {
					let img = document.querySelector("img[id='%1']");
					if (img)
						img.src = "%2";
				}) ();
				)");
		frame->runJavaScript (js.arg (msgId, GetStatusImage (QStringLiteral ("notification_chat_delivery_ok"))));

		disconnect (sender (),
				SIGNAL (messageDelivered ()),
				this,
				SLOT (handleMessageDelivered ()));
	}

	void StandardStyleSource::HandleFrameDestroyed (QObject *frameObj)
	{
		Frame2Colors_.remove (frameObj);
		IsLastMsgRead_.remove (frameObj);
		for (auto i = Msg2Frame_.begin (); i != Msg2Frame_.end (); )
			if (i.value () == frameObj)
				i = Msg2Frame_.erase (i);
			else
				++i;
	}
}
