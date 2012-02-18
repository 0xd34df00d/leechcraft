/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2011 Minh Ngo
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

#include "p100q.h"
#include <QIcon>
#include <QString>
#include <QShortcut>
#include <QTranslator>
#include <QTextEdit>
#include <util/util.h>
#include <interfaces/imessage.h>
#include <interfaces/iclentry.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace p100q
{
	const int PstoCommentPos = 6;

	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("azoth_p100q");

		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothp100qsettings.xml");

		PstoCommentRX_ =  QRegExp ("#[a-z]+/[0-9]+[:]", Qt::CaseInsensitive);
		UserRX_ = QRegExp ("(?:[^>/]|<br />)@([\\w\\-]+)[ :]", Qt::CaseInsensitive);
		PostAuthorRX_ = QRegExp ("<br />@([\\w\\-]+)[ :]", Qt::CaseInsensitive);
		PostRX_ = QRegExp ("#([a-zA-Z0-9]+)[ :]", Qt::CaseInsensitive);
		PostByUserRX_ = QRegExp ("\\s#([a-zA-Z0-9]+)", Qt::CaseInsensitive);
		CommentRX_ = QRegExp ("#([a-zA-Z0-9]+)/([0-9]+)", Qt::CaseInsensitive);
		TagRX_ = QRegExp ("<br />[*] ([^*,<]+(, [^*,<]+)*)");
		ImgRX_ = QRegExp ("<br /><a href=\"(http://[^\"]+[.](png|gif|jpe?g))\">[^<]*</a>", Qt::CaseInsensitive);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.p100q";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth p100q";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth p100q enhances experience with the psto.net microblogging service.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/plugins/azoth/plugins/p100q/resources/images/p100q.svg");
	}

	QStringList Plugin::Provides () const
	{
		return QStringList ();
	}

	QStringList Plugin::Needs () const
	{
		return QStringList ();
	}

	QStringList Plugin::Uses () const
	{
		return QStringList ();
	}

	void Plugin::SetProvider (QObject*, const QString&)
	{
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	QString Plugin::FormatBody (QString body)
	{
		if (body.indexOf (PstoCommentRX_, 0) != PstoCommentPos)
		{
			QString tags, tag;
			int pos = 0;
			int delta = 0;
			while ((pos = TagRX_.indexIn (body, pos)) != -1)
			{
				tags = " * ";
				tag = TagRX_.cap (0);
				QStringList tagslist = TagRX_.cap (1).split (", ");

				Q_FOREACH (const QString& tagval, tagslist)
				{
					QString t = QString (QUrl::toPercentEncoding (tagval)).replace ("%2F", "/");
					tags += QString (" <a href=\"azoth://msgeditreplace/S *%1\" title=\"" + tr ("Subscribe to tag") + "\">%2</a> ")
							.arg (t)
							.arg (tagval);
				}
				delta = body.length ();
				body.replace (tag, tags);
				pos += body.length () - delta;
			}
		}
		const bool showImg = XmlSettingsManager::Instance ().property ("ShowImage").toBool ();
		if (showImg)
			body.replace (ImgRX_,
					"<p><a href=\"\\1\"><img style='max-height: 300px; max-width:300px;' src=\"\\1\"/></a><p/>");

		body.replace (PostRX_,
				"<a href=\"azoth://msgeditreplace/%23\\1%20\">#\\1</a> "
				"("
				"<a href=\"azoth://msgeditreplace/S%20%23\\1\" title=\"" + tr ("Subscribe") + "\">S</a> "
				"<a href=\"azoth://msgeditreplace/U%20%23\\1\" title=\"" + tr ("Unsubscribe") + "\">U</a> "
				"<a href=\"azoth://msgeditreplace/%23\\1+\" title=\"" + tr ("View") + "\">+</a> "
				"<a href=\"azoth://msgeditreplace/!%20%23\\1%20\" title=\"" + tr ("Recommend") + "\">!</a> "
				"<a href=\"azoth://msgeditreplace/~%20%23\\1%20\" title=\"" + tr ("Add to bookmarks") + "\">~</a>"
				") "
				);

		if (XmlSettingsManager::Instance ().property ("ShowAvatars").toBool ())
			body.replace (PostAuthorRX_,
					"<img style='float:left;margin-right:4px' "
							"width='32px' "
							"height='32px' "
							"src='http://psto.net/img/a/40/\\1.png'>"
					" <a href=\"azoth://msgeditreplace/@\\1+\" title=\"" + tr ("View user's posts") + "\">@\\1</a> ");
		else
			body.replace (PostAuthorRX_,
					" <a href=\"azoth://msgeditreplace/@\\1+\" title=\"" + tr ("View user's posts") + "\">@\\1</a> ");

		body.replace(UserRX_,
				" <a href=\"azoth://msgeditreplace/@\\1+\" title=\"" + tr ("View user's posts") + "\">@\\1</a> ");

		body.replace (CommentRX_,
				"<a href=\"azoth://msgeditreplace/%23\\1/\\2%20\" title=\"" + tr ("Reply") + "\">#\\1/\\2</a> "
				"(<a href=\"azoth://msgeditreplace/U%20%23\\1\" title=\"" + tr ("Unsubscribe from post") + "\">U</a> "
				" <a href=\"azoth://msgeditreplace/!%20%23\\1/\\2%20\" title=\"" + tr ("Recommend this comment") + "\">!</a> "
				" <a href=\"azoth://msgeditreplace/~%20%23\\1/\\2%20\" title=\"" + tr ("Add this comment to bookmarks") + "\">~</a>) ");

		body.replace (PostByUserRX_,
				" <a href=\"azoth://msgeditreplace/%23\\1+\" title=\"" + tr ("View post") + "\">#\\1</a> ");
		while (body.startsWith ("<br />"))
			body = body.mid (6);
		body.prepend ("<div style=\"width:100%;overflow:auto;\">");
		body += "</div>";
		return body;
	}

	void Plugin::hookChatTabCreated (IHookProxy_ptr proxy,
			QObject *chatTab, QObject *entry, QWebView*)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("BindLastID").toBool ())
			return;

		ICLEntry *other = qobject_cast<ICLEntry*> (entry);
		if (!other ||
				!other->GetEntryID ().contains ("psto@psto.net"))
			return;

		QTextEdit *edit;
		QMetaObject::invokeMethod (chatTab,
				"getMsgEdit",
				Q_RETURN_ARG (QTextEdit*, edit));
		connect (chatTab,
				SIGNAL (destroyed ()),
				this,
				SLOT (handleChatDestroyed ()));

		QShortcut *sh = new QShortcut (QString ("Ctrl+L"), edit);
		sh->setProperty ("Azoth/p100q/Tab", QVariant::fromValue<QObject*> (chatTab));
		connect (sh,
				SIGNAL (activated ()),
				this,
				SLOT (handleShortcutActivated ()));

		Entry2Tab_ [entry] = chatTab;
	}

	void Plugin::hookFormatBodyEnd (IHookProxy_ptr proxy,
			QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (msg->GetDirection () != IMessage::DIn ||
				msg->GetMessageType () != IMessage::MTChatMessage)
			return;

		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());
		if (!other)
		{
			qWarning () << Q_FUNC_INFO
					<< "NULL other part for message"
					<< msgObj
					<< msg->GetBody ();
			return;
		}

		if (!other->GetEntryID ().contains ("psto@psto.net"))
			return;

		const QString& prevBody = proxy->GetValue ("body").toString ();
		proxy->SetValue ("body", FormatBody (prevBody));

		if (!XmlSettingsManager::Instance ()
				.property ("BindLastID").toBool ())
			return;

		const int aPos = PostByUserRX_.lastIndexIn (prevBody);
		const int pPos = PostRX_.lastIndexIn (prevBody);
		const int cPos = CommentRX_.lastIndexIn (prevBody);
		QRegExp rx;
		if (aPos > pPos && aPos > cPos)
			rx = PostByUserRX_;
		else if (pPos > cPos && pPos > aPos)
			rx = PostRX_;
		else
			rx = CommentRX_;

		QObject *tab = Entry2Tab_ [msg->OtherPart ()];
		if (rx.capturedTexts ().size () == 2)
			LastPostInTab_ [tab] = rx.cap (1);
		else if (rx.capturedTexts ().size () == 3)
			LastPostInTab_ [tab] = rx.cap (1) + '/' + rx.cap (2);
	}

	void Plugin::handleShortcutActivated ()
	{
		QObject *chat = sender ()->property ("Azoth/p100q/Tab").value<QObject*> ();

		QMetaObject::invokeMethod (chat,
				"prepareMessageText",
				Q_ARG (QString, "#" + LastPostInTab_ [chat] + " "));
	}

	void Plugin::handleChatDestroyed ()
	{
		LastPostInTab_.remove (sender ());
		Entry2Tab_.remove (Entry2Tab_.key (sender ()));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_p100q, LeechCraft::Azoth::p100q::Plugin);
