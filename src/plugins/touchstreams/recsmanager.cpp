/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recsmanager.h"
#include <QStandardItem>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QtDebug>
#include <util/svcauth/vkauthmanager.h>
#include <util/sll/queuemanager.h>
#include <util/sll/urloperator.h>
#include <util/sll/parsejson.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/media/iradiostationprovider.h>

namespace LC
{
namespace TouchStreams
{
	RecsManager::RecsManager (std::optional<qulonglong> uid,
			Util::SvcAuth::VkAuthManager *authMgr,
			Util::QueueManager *queueMgr,
			const ICoreProxy_ptr& proxy,
			QObject *parent)
	: QObject { parent }
	, UID_ { uid }
	, AuthMgr_ { authMgr }
	, QueueMgr_ { queueMgr }
	, RequestQueueGuard_ { AuthMgr_->ManageQueue (&RequestQueue_) }
	, Proxy_ { proxy }
	, RootItem_ { new QStandardItem { tr ("VKontakte: recommendations") } }
	{
		static QIcon vkIcon { ":/touchstreams/resources/images/vk.svg" };
		RootItem_->setIcon (vkIcon);
		RootItem_->setEditable (false);
		RootItem_->setData (Media::RadioType::TracksRoot, Media::RadioItemRole::ItemType);

		if (!UID_)
		{
			if (AuthMgr_->HadAuthentication ())
				QTimer::singleShot (1000,
						this,
						SLOT (refetchRecs ()));
			connect (AuthMgr_,
					SIGNAL (justAuthenticated ()),
					this,
					SLOT (refetchRecs ()));
		}
	}

	QStandardItem* RecsManager::GetRootItem () const
	{
		return RootItem_;
	}

	void RecsManager::RefreshItems (QList<QStandardItem*>& items)
	{
		if (!items.removeOne (RootItem_))
			return;

		if (const auto rc = RootItem_->rowCount ())
			RootItem_->removeRows (0, rc);

		refetchRecs ();
	}

	void RecsManager::refetchRecs ()
	{
		RequestQueue_.append ({
				[this] (const QString& key) -> void
				{
					QUrl url ("https://api.vk.com/method/audio.getRecommendations");
					Util::UrlOperator { url }
							("access_token", key)
							("shuffle", "0")
							("count", "100");
					if (UID_)
						Util::UrlOperator { url }
								("user_id", QString::number (*UID_));

					auto nam = Proxy_->GetNetworkAccessManager ();
					connect (nam->get (QNetworkRequest { url }),
							SIGNAL (finished ()),
							this,
							SLOT (handleRecsFetched ()));
				},
				Util::QueuePriority::Normal
			});
		AuthMgr_->GetAuthKey ();
	}

	void RecsManager::handleRecsFetched ()
	{
		if (const auto rc = RootItem_->rowCount ())
			RootItem_->removeRows (0, rc);

		const auto reply = qobject_cast<QNetworkReply*> (sender ());

		const auto& data = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();;
		reply->deleteLater ();

		for (const auto& songVar : data ["response"].toList ())
		{
			const auto& map = songVar.toMap ();

			const auto& url = QUrl::fromEncoded (map ["url"].toString ().toUtf8 ());
			if (!url.isValid ())
				continue;

			Media::AudioInfo info {};
			info.Title_ = map ["title"].toString ();
			info.Artist_ = map ["artist"].toString ();
			info.Length_ = map ["duration"].toInt ();
			info.Other_ ["URL"] = url;

			auto trackItem = new QStandardItem (QString::fromUtf8 ("%1 — %2")
						.arg (info.Artist_)
						.arg (info.Title_));
			trackItem->setEditable (false);
			trackItem->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);
			trackItem->setData (QVariant::fromValue<QList<Media::AudioInfo>> ({ info }),
					Media::RadioItemRole::TracksInfos);
			RootItem_->appendRow (trackItem);
		}
	}
}
}
