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

#include "playlistmanager.h"
#include <QStandardItem>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include "authmanager.h"
#include "accountsmanager.h"
#include "consts.h"

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	PlaylistManager::PlaylistManager (QNetworkAccessManager *nam,
			AuthManager *authMgr, AccountsManager *accMgr, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, AuthMgr_ (authMgr)
	, AccMgr_ (accMgr)
	, Root_ (new QStandardItem ("mp3tunes.com"))
	{
		connect (AuthMgr_,
				SIGNAL (sidReady (QString)),
				this,
				SLOT (requestPlaylists (QString)));

		connect (AccMgr_,
				SIGNAL (accountsChanged ()),
				this,
				SLOT (handleAccountsChanged ()),
				Qt::QueuedConnection);
	}

	QStandardItem* PlaylistManager::GetRoot () const
	{
		return Root_;
	}

	void PlaylistManager::Update ()
	{
		if (Root_->rowCount ())
			return;
		/*
		while (Root_->rowCount ())
			Root_->removeRow (0);*/

		const auto& accs = AccMgr_->GetAccounts ();
		for (const auto& acc : accs)
		{
			auto item = new QStandardItem (acc);
			AccItems_ [acc] = item;
			Root_->appendRow (item);

			requestPlaylists (acc);
		}
	}

	void PlaylistManager::requestPlaylists (const QString& accName)
	{
		const auto& sid = AuthMgr_->GetSID (accName);
		if (sid.isEmpty ())
			return;

		const auto& url = QString ("http://ws.mp3tunes.com/api/v1/lockerData?output=xml"
					"&sid=%1&partner_token=%2&type=playlist")
				.arg (sid)
				.arg (Consts::PartnerId);
		auto reply = NAM_->get (QNetworkRequest (url));
		reply->setProperty ("AccountName", accName);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotPlaylists ()));
	}

	void PlaylistManager::handleGotPlaylists ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& name = reply->property ("AccountName").toString ();
		const auto& data = reply->readAll ();
	}

	void PlaylistManager::handleAccountsChanged ()
	{
		Update ();
	}
}
}
}
