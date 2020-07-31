/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "appinfomanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include "vkconnection.h"

namespace LC
{
namespace Azoth
{
namespace Murm
{
	AppInfoManager::AppInfoManager (QNetworkAccessManager *nam, VkConnection *conn, QObject *parent)
	: QObject { parent }
	, NAM_ { nam }
	, Conn_ { conn }
	{
	}

	bool AppInfoManager::HasAppInfo (qulonglong appId) const
	{
		return AppId2Info_.contains (appId);
	}

	AppInfo AppInfoManager::GetAppInfo (qulonglong appId) const
	{
		return AppId2Info_.value (appId);
	}

	void AppInfoManager::PerformWithAppInfo (qulonglong appId,
			const std::function<void (AppInfo)>& contFound,
			const std::function<void ()>& contNotFound)
	{
		const auto pos = AppId2Info_.find (appId);
		if (pos == AppId2Info_.end ())
		{
			contNotFound ();
			CacheAppInfo (appId);
		}
		else
			contFound (*pos);
	}

	void AppInfoManager::CacheAppInfo (const QList<AppInfo>& infos)
	{
		for (const auto& info : infos)
			CacheAppInfo (info.AppId_);
	}

	QImage AppInfoManager::GetAppImage (const AppInfo& info) const
	{
		return Url2Image_ [info.Icon25_];
	}

	void AppInfoManager::CacheAppInfo (qulonglong appId)
	{
		if (!appId ||
				PendingAppInfos_.contains (appId))
			return;

		if (AppId2Info_.contains (appId))
		{
			emit gotAppInfo (AppId2Info_.value (appId));
			return;
		}

		PendingAppInfos_ << appId;

		Conn_->GetAppInfo (appId,
				[this, appId] (const AppInfo& info)
				{
					AppId2Info_ [info.AppId_] = info;
					PendingAppInfos_.remove (appId);

					if (info.Icon25_.isValid ())
						CacheImage (info.Icon25_, info.AppId_);
					else
						emit gotAppInfo (info);
				});
	}

	void AppInfoManager::CacheImage (const QUrl& url, qulonglong id)
	{
		if (!url.isValid ())
			return;

		if (PendingUrls_.contains (url) ||
				Url2Image_.contains (url))
			return;

		PendingUrls_ << url;

		const auto reply = NAM_->get (QNetworkRequest { url });
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, reply, url, id]
			{
				reply->deleteLater ();
				if (reply->error () != QNetworkReply::NoError)
				{
					qWarning () << Q_FUNC_INFO
							<< reply->errorString ();
					return;
				}

				PendingUrls_.remove (url);

				const auto& img = QImage::fromData (reply->readAll ())
						.scaled (24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				Url2Image_ [url] = img;

				emit gotAppInfo (AppId2Info_ [id]);
			},
			reply,
			SIGNAL (finished ()),
			this
		};
	}
}
}
}
