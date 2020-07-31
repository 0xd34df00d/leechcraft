/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QHash>
#include <QSet>
#include <QUrl>
#include "structures.h"

class QNetworkAccessManager;
class QImage;

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkConnection;

	class AppInfoManager : public QObject
	{
		Q_OBJECT

		QNetworkAccessManager * const NAM_;
		VkConnection * const Conn_;

		QHash<qulonglong, AppInfo> AppId2Info_;
		QSet<qulonglong> PendingAppInfos_;

		QHash<QUrl, QImage> Url2Image_;
		QSet<QUrl> PendingUrls_;
	public:
		AppInfoManager (QNetworkAccessManager*, VkConnection*, QObject* = nullptr);

		bool HasAppInfo (qulonglong) const;
		AppInfo GetAppInfo (qulonglong) const;

		void PerformWithAppInfo (qulonglong,
				const std::function<void (AppInfo)>&, const std::function<void ()>&);

		void CacheAppInfo (const QList<AppInfo>&);

		QImage GetAppImage (const AppInfo&) const;
	private:
		void CacheAppInfo (qulonglong);
		void CacheImage (const QUrl&, qulonglong);
	signals:
		void gotAppInfo (const AppInfo&);
	};
}
}
}
