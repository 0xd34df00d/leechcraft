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
		if (!RecentlyUsed_.isEmpty () && RecentlyUsed_.last () == item)
			return;

		RecentlyUsed_.removeOne (item);
		RecentlyUsed_ << item;
	}

	void PixmapCacheManager::PixmapChanged (PageGraphicsItem *item)
	{
		RecentlyUsed_.removeOne (item);
		RecentlyUsed_ << item;

		CurrentSize_ -= ItemsSizes_ [item];
		CurrentSize_ += item->GetMemorySize ();
		ItemsSizes_ [item] = item->GetMemorySize ();

		CheckCache ();
	}

	void PixmapCacheManager::PixmapDeleted (PageGraphicsItem *item)
	{
		CurrentSize_ -= ItemsSizes_.take (item);
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

			CurrentSize_ -= ItemsSizes_.take (page);
			page->ClearPixmap ();
			i = RecentlyUsed_.erase (i);
		}
	}
}
