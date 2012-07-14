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

#include "useravatarmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include "pubsubmanager.h"
#include "useravatardata.h"
#include "useravatarmetadata.h"
#include "core.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	UserAvatarManager::UserAvatarManager (ClientConnection *conn)
	: QObject (conn)
	, Manager_ (conn->GetPubSubManager ())
	, Conn_ (conn)
	{
		connect (Manager_,
				SIGNAL (gotEvent (QString, PEPEventBase*)),
				this,
				SLOT (handleEvent (QString, PEPEventBase*)));

		Manager_->RegisterCreator<UserAvatarData> ();
		Manager_->RegisterCreator<UserAvatarMetadata> ();
		Manager_->SetAutosubscribe<UserAvatarMetadata> (true);
	}

	void UserAvatarManager::PublishAvatar (const QImage& avatar)
	{
		if (!avatar.isNull ())
		{
			UserAvatarData data (avatar);

			Manager_->PublishEvent (&data);
		}

		UserAvatarMetadata metadata (avatar);
		Manager_->PublishEvent (&metadata);
	}

	void UserAvatarManager::handleEvent (const QString& from, PEPEventBase *event)
	{
		UserAvatarMetadata *mdEvent = dynamic_cast<UserAvatarMetadata*> (event);
		if (mdEvent)
		{
			if (mdEvent->GetID ().isEmpty ())
			{
				emit avatarUpdated (from, QImage ());
				return;
			}

			QString bare;
			QString resource;
			ClientConnection::Split (from, &bare, &resource);

			ICLEntry *entry = qobject_cast<ICLEntry*> (Conn_->GetCLEntry (bare, resource));
			if (entry && !entry->GetAvatar ().isNull ())
			{
				UserAvatarMetadata md (entry->GetAvatar ());
				if (mdEvent->GetID () == md.GetID ())
					return;
			}

			if (mdEvent->GetURL ().isValid ())
			{
				QNetworkAccessManager *mgr = Core::Instance ()
						.GetProxy ()->GetNetworkAccessManager ();

				QNetworkReply *rep = mgr->get (QNetworkRequest (mdEvent->GetURL ()));
				rep->setProperty ("Azoth/From", from);
				connect (rep,
						SIGNAL (finished ()),
						this,
						SLOT (handleHTTPFinished ()));
			}
			else
				Manager_->RequestItem (bare,
						UserAvatarData::GetNodeString (),
						mdEvent->GetID ());

			return;
		}

		UserAvatarData *dEvent = dynamic_cast<UserAvatarData*> (event);
		if (dEvent)
			emit avatarUpdated (from, dEvent->GetImage ());
	}

	void UserAvatarManager::handleHTTPFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "isn't a QNetworkReply";
			return;
		}

		reply->deleteLater ();

		const QString& from = reply->property ("Azoth/From").toString ();
		if (from.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty from";
			return;
		}

		emit avatarUpdated (from, QImage::fromData (reply->readAll ()));
	}
}
}
}
