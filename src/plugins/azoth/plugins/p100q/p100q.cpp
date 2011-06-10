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

#include "p100q.h"
#include <QIcon>
#include <QString>
#include <interfaces/imessage.h>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace p100q
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		PstoCommentRX_ =  QRegExp ("#[a-z]+/[0-9]+[:]", Qt::CaseInsensitive);
		UserRX_ = QRegExp ("(?:[^>/]|<br />)@([\\w\\-]+)[ :]", Qt::CaseInsensitive);
		PostAuthorRX_ = QRegExp ("<br />@([\\w\\-]+)[ :]", Qt::CaseInsensitive);
		PostRX_ = QRegExp ("#([a-zA-Z0-9]+) ", Qt::CaseInsensitive);
		PostByUserRX_ = QRegExp ("\\s#([a-zA-Z0-9]+)", Qt::CaseInsensitive);
		CommentRX_ = QRegExp ("#([a-zA-Z0-9]+)/([0-9]+)", Qt::CaseInsensitive);
		TagRX_ = QRegExp ("<br />[*] ([^*,<]+(, [^*,<]+)*)");
		ImgRX_ = QRegExp ("<a href=\"(http://[^<>\"]+[.](png|jpg|gif|jpeg))\">http://[^<>\"]+[.](png|jpg|gif|jpeg)</a>", Qt::CaseInsensitive);
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
		return "p100q";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("p100q is plugin for nicer support of the psto.net microblogging service.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
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
				tags = "* ";
				tag = TagRX_.cap (0);
				QStringList tagslist = TagRX_.cap (1).split (", ");
				
				Q_FOREACH (const QString& tagval, tagslist)
				{
					tags += QString (" <a href=\"azoth://msgeditreplace/S *%1\">%2</a> ")
							.arg (QString (QUrl::toPercentEncoding (tagval)))
							.arg (tagval);
				}
				delta = body.length ();
				body.replace (tag, tags);
				pos += body.length () - delta;
			}
		}
		
		body.replace (ImgRX_,
				"<p><a href=\"\\1\"><img style='max-height: 300px; max-width:300px;' src=\"\\1\"/></a><p/>");
				
		body.replace (PostRX_,
				" <a href=\"azoth://msgeditreplace/%23\\1\">#\\1</a> "
				"("
				"<a href=\"azoth://msgeditreplace/S%20%23\\1\">S</a> "
				"<a href=\"azoth://msgeditreplace/%23\\1+\">+</a> "
				"<a href=\"azoth://msgeditreplace/!%20%23\\1\">!</a> "
				") ");
		
		body.replace (PostAuthorRX_,
				"<img style='float:left;margin-right:4px' "
						"width='32px' "
						"height='32px' "
						"src='http://psto.net/img/a/40/\\1.png'>"
				" <a href=\"azoth://msgeditreplace/@\\1+\">@\\1</a> ");
		
		body.replace(UserRX_,
				" <a href=\"azoth://msgeditreplace/@\\1+\">@\\1</a> ");
				
		body.replace (CommentRX_,
				" <a href=\"azoth://msgeditreplace/%23\\1/\\2%20\">#\\1/\\2</a>");
				
		body.replace (PostByUserRX_,
				" <a href=\"azoth://msgeditreplace/%23\\1+\">#\\1</a> ");
		
		body.prepend("<div style=\"width:100%;overflow:auto;\">");
		body += "</div>";
		return body;
	}

	void Plugin::hookFormatBodyEnd (IHookProxy_ptr proxy,
			QObject*, QString body, QObject *msgObj)
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

		proxy->SetValue ("body", FormatBody (body));
	}
	
	void Plugin::hookThemeReloaded (LeechCraft::IHookProxy_ptr proxy,
			QObject *chatTab, QWebView *view, QObject *entry)
	{
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_p100q, LeechCraft::Azoth::p100q::Plugin);
