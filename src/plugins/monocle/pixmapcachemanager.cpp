/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pixmapcachemanager.h"
#include <numeric>
#include <QtDebug>
#include "components/viewitems/pagegraphicsitem.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Monocle
{
	PixmapCacheManager::PixmapCacheManager (QObject *parent)
	: QObject (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("PixmapCacheSize",
				this, "handleCacheSizeChanged");
		handleCacheSizeChanged ();
	}

	namespace
	{
		quint64 GetPixmapSize (const QPixmap& px)
		{
			if (px.isNull ())
				return 0;

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
					[] (qint64 size, const PageGraphicsItem *item)
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
		for (auto i = RecentlyUsed_.begin (); i != RecentlyUsed_.end () && MaxSize_ < CurrentSize_; )
		{
			const auto page = *i;
			if (page->IsDisplayed ())
			{
				++i;
				continue;
			}

			const quint64 pxSize = GetPixmapSize (page->pixmap ());
			CurrentSize_ -= pxSize;
			page->ClearPixmap ();
			i = RecentlyUsed_.erase (i);
		}

		if (MaxSize_ < CurrentSize_)
			qWarning () << Q_FUNC_INFO
					<< "cache overflow:"
					<< CurrentSize_
					<< "instead of"
					<< MaxSize_
					<< "for"
					<< RecentlyUsed_.size ()
					<< "pages";
	}

	void PixmapCacheManager::handleCacheSizeChanged ()
	{
		MaxSize_ = XmlSettingsManager::Instance ().property ("PixmapCacheSize").value<qint64> () * 1024 * 1024;

		CheckCache ();
	}
}
}
