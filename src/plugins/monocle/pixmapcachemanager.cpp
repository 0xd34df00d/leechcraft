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

#include "pixmapcachemanager.h"
#include <numeric>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "pagegraphicsitem.h"

namespace LeechCraft
{
namespace Monocle
{
	PixmapCacheManager::PixmapCacheManager (QObject *parent)
	: QObject (parent)
	, CurrentSize_ (0)
	, MaxSize_ (0)
	{
		XmlSettingsManager::Instance ().RegisterObject ("PixmapCacheSize",
				this, "handleCacheSizeChanged");
		handleCacheSizeChanged ();
	}

	namespace
	{
		quint64 GetPixmapSize (const QPixmap& px)
		{
			return px.width () * px.height () * px.defaultDepth () / 8 * 1.5;
		}
	}

	void PixmapCacheManager::PixmapPainted (PageGraphicsItem *item)
	{
		RecentlyUsed_.removeAll (item);
		RecentlyUsed_ << item;
	}

	void PixmapCacheManager::PixmapChanged (PageGraphicsItem *item)
	{
		if (RecentlyUsed_.removeAll (item))
			CurrentSize_ = std::accumulate (RecentlyUsed_.begin (), RecentlyUsed_.end (), 0,
					[] (qint64 size, decltype (RecentlyUsed_.front ()) item)
						{ return size + GetPixmapSize (item->pixmap ()); });

		RecentlyUsed_ << item;
		CurrentSize_ += GetPixmapSize (item->pixmap ());
		CheckCache ();
	}

	void PixmapCacheManager::PixmapDeleted (PageGraphicsItem *item)
	{
		CurrentSize_ -= GetPixmapSize (item->pixmap ());
		RecentlyUsed_.removeAll (item);
	}

	void PixmapCacheManager::CheckCache ()
	{
		while (MaxSize_ < CurrentSize_ && RecentlyUsed_.size () > 2)
		{
			auto page = RecentlyUsed_.takeFirst ();
			const quint64 pxSize = GetPixmapSize (page->pixmap ());
			CurrentSize_ -= pxSize;
			page->ClearPixmap ();
		}
	}

	void PixmapCacheManager::handleCacheSizeChanged ()
	{
		MaxSize_ = XmlSettingsManager::Instance ().property ("PixmapCacheSize").value<qint64> () * 1024 * 1024;

		CheckCache ();
	}
}
}
