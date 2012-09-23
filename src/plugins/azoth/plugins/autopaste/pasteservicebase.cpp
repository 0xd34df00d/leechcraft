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

#include "pasteservicebase.h"
#include <QApplication>
#include <QClipboard>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Autopaste
{
	PasteServiceBase::PasteServiceBase (QObject *entry, QObject *parent)
	: QObject (parent)
	, Entry_ (entry)
	{
	}

	void PasteServiceBase::InitReply (QNetworkReply *reply)
	{
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleFinished ()));
		connect (reply,
				SIGNAL (metaDataChanged ()),
				this,
				SLOT (handleMetadata ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleError ()));
	}

	void PasteServiceBase::FeedURL (const QString& pasteUrl)
	{
		if (!Entry_)
		{
			QApplication::clipboard ()->setText (pasteUrl, QClipboard::Clipboard);
			QApplication::clipboard ()->setText (pasteUrl, QClipboard::Selection);
			const Entity& e = Util::MakeNotification (tr ("Text pasted"),
					tr ("Your text has been pasted: %1. The URL has "
						"been copied to the clipboard."),
					PInfo_);
			emit gotEntity (e);
			return;
		}

		auto entry = qobject_cast<ICLEntry*> (Entry_);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< Entry_
					<< "to ICLEntry";
			return;
		}

		auto type = entry->GetEntryType () == ICLEntry::ETMUC ?
				IMessage::MTMUCMessage :
				IMessage::MTChatMessage;
		QObject *msgObj = entry->CreateMessage (type, QString (), pasteUrl);
		auto msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< msgObj
					<< "to IMessage";
			return;
		}
		msg->Send ();
	}

	void PasteServiceBase::handleError ()
	{
		sender ()->deleteLater ();
		deleteLater ();
	}

	void PasteServiceBase::handleFinished ()
	{
		sender ()->deleteLater ();
		deleteLater ();
	}

	void PasteServiceBase::handleMetadata ()
	{
	}
}
}
}
