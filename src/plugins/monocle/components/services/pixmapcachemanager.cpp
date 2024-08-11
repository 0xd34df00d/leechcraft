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

namespace LC::Monocle
{
	PixmapCacheManager::PixmapCacheManager (QObject *parent)
	: QObject { parent }
	{
		constexpr auto megabytes = 1024 * 1024;

		XmlSettingsManager::Instance ().RegisterObject ("PixmapCacheSize",
				this,
				[this] (qint64 size)
				{
					MaxSize_ = size * megabytes;
					CheckCache ();
				});
	}

	void PixmapCacheManager::PixmapPainted (PageGraphicsItem *item)
	{
		RecentlyUsed_.removeOne (item);
		RecentlyUsed_ << item;
	}

	void PixmapCacheManager::PixmapChanged (PageGraphicsItem *item)
	{
		if (RecentlyUsed_.removeOne (item))
			CurrentSize_ = std::accumulate (RecentlyUsed_.begin (), RecentlyUsed_.end (), 0,
					[] (qint64 size, const PageGraphicsItem *item) { return size + item->GetMemorySize (); });

		RecentlyUsed_ << item;
		CurrentSize_ += item->GetMemorySize ();
		CheckCache ();
	}

	void PixmapCacheManager::PixmapDeleted (PageGraphicsItem *item)
	{
		CurrentSize_ -= item->GetMemorySize ();
		RecentlyUsed_.removeOne (item);
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

			CurrentSize_ -= page->GetMemorySize ();
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
}
