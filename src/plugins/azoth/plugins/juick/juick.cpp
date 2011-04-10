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

#include "juick.h"
#include <QIcon>
#include <interfaces/imessage.h>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Juick
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		UserRX_ = QRegExp ("(@[\\w\\-\\.@\\|]*)\\b", Qt::CaseInsensitive);
		PostRX_ = QRegExp ("<br />#(\\d+)\\s", Qt::CaseInsensitive);
		IdRX_ = QRegExp ("#(\\d+)\\s", Qt::CaseInsensitive);
		ReplyRX_ = QRegExp ("#(\\d+/\\d+)\\s?", Qt::CaseInsensitive);
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.juick";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "juick";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("juick is plugin for nicer support of the juick.com microblogging service.");
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
		body.replace (UserRX_, "<a href=\"azoth://msgeditreplace/\\1+\">\\1</a>");
		body.replace (PostRX_, 
				"<br /> <a href=\"azoth://msgeditreplace/%23\\1%20\">#\\1</a> "
				"("
				"<a href=\"azoth://msgeditreplace/S%20%23\\1\">S</a> "
				"<a href=\"azoth://msgeditreplace/%23\\1+\">+</a> "
				"<a href=\"azoth://msgeditreplace/!%20%23\\1\">!</a> "
				")");
		body.replace (IdRX_, "<a href=\"azoth://msgeditreplace/%23\\1+\">#\\1</a> ");
		body.replace (ReplyRX_, "<a href=\"azoth://msgeditreplace/%23\\1%20\">#\\1</a> ");	
	
		return body;
	}

	void Plugin::hookFormatBodyEnd (IHookProxy_ptr proxy,
			QObject*, QString body, QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);

		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< msgObj
				<< "to IMessage";
			return;
		}

		if (msg->GetDirection () != IMessage::DIn ||
				msg->GetMessageType () != IMessage::MTChatMessage)
			return;

		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());

		if (!other)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< msg
				<< "to ICLEntry";
			return;
		}

		if (!other->GetEntryID ().contains ("juick@juick.com"))
			return;

		proxy->SetValue ("body", FormatBody (body));
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_juick, LeechCraft::Azoth::Juick::Plugin);
