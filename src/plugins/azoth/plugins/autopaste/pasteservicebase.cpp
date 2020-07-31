/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pasteservicebase.h"
#include <QApplication>
#include <QClipboard>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/xpc/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/core/ientitymanager.h>

namespace LC::Azoth::Autopaste
{
	PasteServiceBase::PasteServiceBase (QObject *entry, const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Entry_ (entry)
	{
	}

	void PasteServiceBase::InitReply (QNetworkReply *reply)
	{
		reply->setParent (this);

		connect (reply,
				&QNetworkReply::finished,
				this,
				[this, reply]
				{
					HandleFinished (reply);
					deleteLater ();
				});
		connect (reply,
				&QNetworkReply::redirected,
				this,
				[this] (const QUrl& url)
				{
					FeedURL (url.toString ());
					deleteLater ();
				});
		connect (reply,
				&QNetworkReply::metaDataChanged,
				this,
				[this, reply] { HandleMetadata (reply); });
		connect (reply,
				qOverload<QNetworkReply::NetworkError> (&QNetworkReply::error),
				this,
				[this, reply] (QNetworkReply::NetworkError error)
				{
					qWarning () << "Azoth Autopaste"
							<< "base network error:"
							<< reply->request ().url ()
							<< error;
					HandleError (error, reply);
				});
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
					Priority::Info);
			Proxy_->GetEntityManager ()->HandleEntity (e);

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

		auto type = entry->GetEntryType () == ICLEntry::EntryType::MUC ?
				IMessage::Type::MUCMessage :
				IMessage::Type::ChatMessage;
		const auto msg = entry->CreateMessage (type, QString (), pasteUrl);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to create message for"
					<< entry->GetEntryID ();
			return;
		}
		msg->Send ();
	}

	void PasteServiceBase::HandleError (QNetworkReply::NetworkError error, QNetworkReply *reply)
	{
		auto message = error == QNetworkReply::ProtocolFailure ?
				tr ("Unexpected reply from the service. Maybe its support in Autopaste got oudated.") :
				reply->errorString ();

		const Entity& e = Util::MakeNotification (tr ("Text paste failure"),
				tr ("Couldn't paste text. %1").arg (message),
				Priority::Critical);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		deleteLater ();
	}

	void PasteServiceBase::HandleFinished (QNetworkReply*)
	{
	}

	void PasteServiceBase::HandleMetadata (QNetworkReply*)
	{
	}
}
