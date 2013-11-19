/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "friendsmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <QTimer>
#include <QtDebug>
#include <qjson/parser.h>
#include <interfaces/media/iradiostationprovider.h>
#include <util/svcauth/vkauthmanager.h>
#include <util/queuemanager.h>
#include "albumsmanager.h"

namespace LeechCraft
{
namespace TouchStreams
{
	namespace
	{
		enum FriendRole
		{
			PhotoUrlRole = Media::RadioItemRole::MaxRadioRole + 1
		};
	}

	FriendsManager::FriendsManager (Util::SvcAuth::VkAuthManager *authMgr,
			Util::QueueManager *queueMgr, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, AuthMgr_ (authMgr)
	, Queue_ (queueMgr)
	, Root_ (new QStandardItem (tr ("VKontakte: friends")))
	{
		Root_->setEditable (false);

		AuthMgr_->ManageQueue (&RequestQueue_);

		QTimer::singleShot (1000,
				this,
				SLOT (refetchFriends ()));
	}

	QStandardItem* FriendsManager::GetRootItem () const
	{
		return Root_;
	}

	void FriendsManager::RefreshItems (QList<QStandardItem*> items)
	{
		if (items.contains (Root_))
		{
			if (auto rc = Root_->rowCount ())
				Root_->removeRows (0, rc);

			Friend2Item_.clear ();
			Friend2AlbumsManager_.clear ();
			Queue_->Clear ();
			RequestQueue_.clear ();

			refetchFriends ();
			return;
		}

		const auto& mgrs = Friend2AlbumsManager_.values ();
		for (auto mgr : mgrs)
		{
			items.removeOne (mgr->RefreshItems (items));
			if (items.isEmpty ())
				break;
		}
	}

	void FriendsManager::refetchFriends ()
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		RequestQueue_.push_back ([this, nam] (const QString& key) -> void
			{
				QUrl friendsUrl ("https://api.vk.com/method/friends.get");
				friendsUrl.addQueryItem ("access_token", key);
				friendsUrl.addQueryItem ("order", "name");
				friendsUrl.addQueryItem ("fields", "uid,first_name,last_name,photo");
				auto reply = nam->get (QNetworkRequest (friendsUrl));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotFriends ()));
			});
	}

	FriendsManager::~FriendsManager ()
	{
		AuthMgr_->UnmanageQueue (&RequestQueue_);
	}

	void FriendsManager::handleGotFriends ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = QJson::Parser ().parse (reply).toMap ();
		auto usersList = data ["response"].toList ();
		for (const auto& userVar : usersList)
		{
			const auto& map = userVar.toMap ();

			const auto id = map ["user_id"].toLongLong ();

			auto mgr = new AlbumsManager (id, AuthMgr_, Queue_, Proxy_, this);
			Friend2AlbumsManager_ [id] = mgr;

			const auto& name = map ["first_name"].toString () + " " + map ["last_name"].toString ();

			auto userItem = mgr->GetRootItem ();
			userItem->setText (name);
			userItem->setData (QUrl::fromEncoded (map ["photo"].toByteArray ()), PhotoUrlRole);
			userItem->setIcon (Proxy_->GetIcon ("user-identity"));
			Root_->appendRow (userItem);
			Friend2Item_ [id] = userItem;

			connect (mgr,
					SIGNAL (finished (AlbumsManager*)),
					this,
					SLOT (handleAlbumsFinished (AlbumsManager*)));
		}
	}

	void FriendsManager::handleAlbumsFinished (AlbumsManager *mgr)
	{
		mgr->deleteLater ();

		const auto uid = mgr->GetUserID ();
		if (!Friend2Item_.contains (uid))
			return;

		if (!mgr->GetTracksCount ())
		{
			Root_->removeRow (Friend2Item_.take (uid)->row ());
			return;
		}

		const auto& url = Friend2Item_ [uid]->data (PhotoUrlRole).toUrl ();
		const auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (url));
		reply->setProperty ("TS/UID", uid);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handlePhotoFetched ()));
	}

	void FriendsManager::handlePhotoFetched ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		QPixmap px;
		if (!px.loadFromData (reply->readAll ()))
			return;

		const auto uid = reply->property ("TS/UID").toLongLong ();
		Friend2Item_ [uid]->setIcon (px);
	}
}
}
