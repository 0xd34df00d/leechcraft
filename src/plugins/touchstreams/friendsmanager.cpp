/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "friendsmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <QIcon>
#include <QTimer>
#include <QtDebug>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/svcauth/vkauthmanager.h>
#include <util/svcauth/vkcaptchadialog.h>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include "albumsmanager.h"
#include "xmlsettingsmanager.h"
#include "recsmanager.h"

namespace LC
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
	, RequestQueueGuard_ (AuthMgr_->ManageQueue (&RequestQueue_))
	, Root_ (new QStandardItem (tr ("VKontakte: friends")))
	{
		Root_->setIcon (QIcon (":/touchstreams/resources/images/vk.svg"));
		Root_->setEditable (false);
		Root_->setData (Media::RadioType::TracksRoot, Media::RadioItemRole::ItemType);

		QTimer::singleShot (1000,
				this,
				SLOT (refetchFriends ()));

		XmlSettingsManager::Instance ().RegisterObject ("RequestFriendsData",
				this, "refetchFriends");
	}

	QStandardItem* FriendsManager::GetRootItem () const
	{
		return Root_;
	}

	void FriendsManager::RefreshItems (QList<QStandardItem*>& items)
	{
		if (items.contains (Root_))
		{
			if (auto rc = Root_->rowCount ())
				Root_->removeRows (0, rc);

			Friend2Item_.clear ();
			Friend2AlbumsManager_.clear ();
			Friend2RecsManager_.clear ();
			Queue_->Clear ();
			RequestQueue_.clear ();

			refetchFriends ();

			items.removeOne (Root_);

			return;
		}

		for (const auto& mgr : Friend2AlbumsManager_)
		{
			mgr->RefreshItems (items);
			if (items.isEmpty ())
				break;
		}

		for (const auto& mgr : Friend2RecsManager_)
		{
			mgr->RefreshItems (items);
			if (items.isEmpty ())
				break;
		}
	}

	void FriendsManager::ShowFriendsList (const QList<qlonglong>& ids, const QMap<qlonglong, QVariantMap>& map)
	{
		for (const auto& id : ids)
			MakeFriendItem (id, map [id], {}, {});
	}

	void FriendsManager::MakeFriendItem (qlonglong id, const QVariantMap& map, const QVariant& albums, const QVariant& tracks)
	{
		const auto& mgr = std::make_shared<AlbumsManager> (id,
				albums, tracks, AuthMgr_, Proxy_);
		Friend2AlbumsManager_ [id] = mgr;

		const auto& name = map ["first_name"].toString () + " " + map ["last_name"].toString ();
		const auto& userItem = new QStandardItem { name };
		userItem->setText (name);
		userItem->setData (QUrl::fromEncoded (map ["photo"].toByteArray ()), PhotoUrlRole);
		userItem->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("user-identity"));
		Root_->appendRow (userItem);
		Friend2Item_ [id] = userItem;

		const auto albumItem = mgr->GetRootItem ();
		albumItem->setText (tr ("Albums"));
		albumItem->setIcon (Proxy_->GetIconThemeManager ()->GetIcon ("media-optical"));
		handleAlbumsFinished (mgr.get ());
		userItem->appendRow (albumItem);

		const auto& recsMgr = std::make_shared<RecsManager> (id, AuthMgr_, Queue_, Proxy_);
		Friend2RecsManager_ [id] = recsMgr;

		const auto recsItem = recsMgr->GetRootItem ();
		recsItem->setText (tr ("Recommendations"));
		userItem->appendRow (recsItem);
	}

	void FriendsManager::refetchFriends ()
	{
		auto nam = Proxy_->GetNetworkAccessManager ();
		RequestQueue_.push_back ([this, nam] (const QString& key) -> void
			{
				QUrl friendsUrl ("https://api.vk.com/method/friends.get");
				Util::UrlOperator { friendsUrl }
						("access_token", key)
						("order", "name")
						("fields", "uid,first_name,last_name,photo");
				auto reply = nam->get (QNetworkRequest (friendsUrl));
				connect (reply,
						SIGNAL (finished ()),
						this,
						SLOT (handleGotFriends ()));
			});
		AuthMgr_->GetAuthKey ();
	}

	void FriendsManager::handleGotFriends ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();
		auto usersList = data ["response"].toList ();

		QList<qlonglong> ids;
		QMap<qlonglong, QVariantMap> user2info;
		for (const auto& userVar : usersList)
		{
			const auto& map = userVar.toMap ();
			const auto id = map ["user_id"].toLongLong ();
			ids << id;
			user2info [id] = map;
		}

		if (!XmlSettingsManager::Instance ()
				.property ("RequestFriendsData").toBool ())
			ShowFriendsList (ids, user2info);
		else
			ScheduleTracksRequests (ids, user2info);
	}

	void FriendsManager::ScheduleTracksRequests (const QList<qlonglong>& ids,
			const QMap<qlonglong, QVariantMap>& user2info)
	{
		const auto portion = 10;
		for (int i = 0; i < ids.size (); i += portion)
		{
			const QStringList sub { Util::Map (ids.mid (i, portion),
						[] (qulonglong num) { return QString::number (num); }) };

			const auto& code = QString (R"d(
					var ids = [%1];
					var i = 0;
					var res = [];
					while (i < %2)
					{
						var id = ids [i];
						var albs = API.audio.getAlbums ({ "uid": id, "count": 100 });
						var trs = API.audio.get ({ "uid": id, "count": 1000 });
						res = res + [{ "id": id, "albums": albs, "tracks": trs }];
						i = i + 1;
					};
					return res;
				)d")
					.arg (sub.join (","))
					.arg (sub.size ());

			auto nam = Proxy_->GetNetworkAccessManager ();
			RequestQueue_.push_back ([this, nam, code, user2info] (const QString& key) -> void
				{
					auto f = [=, this] (const QMap<QString, QString>& map)
					{
						QUrl url ("https://api.vk.com/method/execute");

						auto query = "access_token=" + QUrl::toPercentEncoding (key.toUtf8 ());
						query += '&';
						query += "code=" + QUrl::toPercentEncoding (code.toUtf8 ());

						for (const auto& [key, val] : Util::Stlize (map))
							query += '&' + key.toUtf8 () + '=' + QUrl::toPercentEncoding (val.toUtf8 ());

						QNetworkRequest req (url);
						req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
						auto reply = nam->post (req, query);
						connect (reply,
								SIGNAL (finished ()),
								this,
								SLOT (handleExecuted ()));

						Reply2Users_ [reply] = user2info;

						return reply;
					};
					Reply2Func_ [f ({})] = f;
				});
		}
		AuthMgr_->GetAuthKey ();
	}

	void FriendsManager::handleCaptchaEntered (const QString& cid, const QString& value)
	{
		if (Queue_->IsPaused ())
			Queue_->Resume ();

		if (!CaptchaReplyMaker_)
			return;

		if (value.isEmpty ())
			return;

		decltype (CaptchaReplyMaker_) maker;
		using std::swap;
		swap (maker, CaptchaReplyMaker_);
		Queue_->Schedule ([cid, value, maker, this]
				{
					const QMap<QString, QString> map
					{
						{ "captcha_sid", cid },
						{ "captcha_key", value }
					};
					Reply2Func_ [maker (map)] = maker;
				},
				nullptr,
				Util::QueuePriority::High);
	}

	void FriendsManager::handleExecuted ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& usersMap = Reply2Users_.take (reply);
		const auto& reqFunc = Reply2Func_.take (reply);

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();

		if (data.contains ("error"))
		{
			const auto& errMap = data ["error"].toMap ();
			if (errMap ["error_code"].toULongLong () == 14)
			{
				qDebug () << Q_FUNC_INFO
						<< "captcha requested";
				if (Queue_->IsPaused ())
					return;

				Queue_->Pause ();

				auto captchaDialog = new Util::SvcAuth::VkCaptchaDialog (errMap,
						Proxy_->GetNetworkAccessManager ());
				captchaDialog->SetContextName ("TouchStreams");
				captchaDialog->show ();
				connect (captchaDialog,
						SIGNAL (gotCaptcha (QString, QString)),
						this,
						SLOT (handleCaptchaEntered (QString, QString)));
				CaptchaReplyMaker_ = std::move (reqFunc);
			}
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "error"
						<< errMap;
				return;
			}
		}

		for (const auto& userDataVar : data ["response"].toList ())
		{
			const auto& userData = userDataVar.toMap ();
			const auto id = userData ["id"].toLongLong ();
			const auto& map = usersMap [id];

			MakeFriendItem (id, map, userData ["albums"], userData ["tracks"]);
		}
	}

	void FriendsManager::handleAlbumsFinished (AlbumsManager *mgr)
	{
		const auto uid = mgr->GetUserID ();
		if (!Friend2Item_.contains (uid))
			return;

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
		if (!Friend2Item_.contains (uid))
			return;

		Friend2Item_ [uid]->setIcon (px);
	}
}
}
