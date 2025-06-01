/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "requesthandler.h"
#include <cmath>
#include <QUrlQuery>
#include <util/sll/channeldevice.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/poshuku/istoragebackend.h>
#include <interfaces/poshuku/iproxyobject.h>
#include "constants.h"
#include "customsitesmanager.h"
#include "imagecache.h"
#include "pagerenderer.h"
#include "xmlsettingsmanager.h"

namespace LC::Poshuku::SpeedDial
{
	using HandleResult = IInternalSchemeHandler::HandleResult;
	using ReplyContents = IInternalSchemeHandler::ReplyContents;

	namespace
	{
		double GetScore (const QDateTime& then, const QDateTime& now)
		{
			return std::log (std::max<int> (then.daysTo (now) + 1, 1));
		}

		template<typename K, typename V>
		auto GetSortedVec (const QHash<K, V>& hash)
		{
			std::vector<std::pair<K, V>> vec { hash.keyValueBegin (), hash.keyValueEnd () };
			std::sort (vec.rbegin (), vec.rend (), Util::ComparingBy (Util::Snd));
			return vec;
		}

		struct LoadResult
		{
			TopList_t TopPages_;
			TopList_t TopHosts_;
		};

		LoadResult GetTopUrls (const IStorageBackend_ptr& sb, size_t count)
		{
			history_items_t items;
			sb->LoadHistory (items);

			const auto& now = QDateTime::currentDateTime ();

			QHash<QString, double> url2score;
			QHash<QStringView, double> host2score;
			for (const auto& item : items)
			{
				const auto score = GetScore (item.DateTime_, now);
				url2score [item.URL_] += score;

				const auto startPos = item.URL_.indexOf ("//") + 2;
				const auto endPos = item.URL_.indexOf ('/', startPos);
				if (startPos >= 0 && endPos > startPos)
					host2score [QStringView { item.URL_ }.left (endPos + 1)] += score;
			}
			const auto& hostsVec = GetSortedVec (host2score);

			TopList_t topSites;
			for (size_t i = 0; i < std::min (hostsVec.size (), count); ++i)
			{
				const auto& url = hostsVec [i].first.toString ();
				topSites.append ({ url, url });

				url2score.remove (url);
			}

			const auto& vec = GetSortedVec (url2score);

			TopList_t topPages;
			for (size_t i = 0; i < std::min (vec.size (), count); ++i)
			{
				const auto& url = vec [i].first;

				const auto& item = std::find_if (items.begin (), items.end (),
						[&url] (const HistoryItem& item) { return item.URL_ == url; });

				topPages.append ({ url, item->Title_ });
			}

			return { topPages, topSites };
		}

		QByteArray MakeRootPage (const RootPageDeps& deps)
		{
			if (XmlSettingsManager::Instance ().property ("UseStaticList").toBool ())
			{
				const auto& topList = deps.CustomSites_.GetTopList ();
				return topList.isEmpty () ?
						MakeEmptyTopList () :
						MakePage (deps.ImageCache_, { { {}, topList } });
			}
			else
			{
				const auto sb = deps.PoshukuProxy_.CreateStorageBackend ();
				const auto& result = GetTopUrls (sb, Rows * Cols);
				return MakePage (deps.ImageCache_,
					{
						{ QObject::tr ("Top pages"), result.TopPages_ },
						{ QObject::tr ("Top sites"), result.TopHosts_ }
					});
			}
		}

		ReplyContents GetThumbnail (ImageCache& cache, const QUrl& pageUrl)
		{
			if (auto file = cache.GetSnapshotFile (pageUrl))
				return file;

			auto device = std::make_shared<Util::ChannelDevice> ();
			device->open (QIODevice::ReadWrite);
			QObject::connect (&cache,
					&ImageCache::gotSnapshot,
					device.get (),
					[pageUrl, devPtr = device.get ()] (const QUrl& url, const QByteArray& data)
					{
						if (pageUrl == url)
						{
							devPtr->write (data);
							devPtr->FinishWrite ();
						}
					});
			return device;
		}
	}

	HandleResult HandleRequest (const QString& path, const QUrlQuery& query, const RootPageDeps& deps)
	{
		if (path.isEmpty ())
			return
			{
				{
					.ContentType_ = "text/html",
					.Contents_ = MakeRootPage (deps)
				}
			};

		if (path == ThumbPath)
		{
			const auto& pageUrlStr = query.queryItemValue (ThumbUrlKey);
			const auto& pageUrl = QUrl::fromEncoded (pageUrlStr.toUtf8 ());
			if (pageUrl.isValid ())
				return
				{
					{
						.ContentType_ = "image/png",
						.Contents_ = GetThumbnail (deps.ImageCache_, pageUrl),
					}
				};
			qWarning () << "invalid thumb URL:" << query.toString ();
		}

		return { Util::AsLeft, IInternalSchemeHandler::Error::Unsupported };
	}
}
