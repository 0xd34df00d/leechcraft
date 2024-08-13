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

	void PixmapCacheManager::RegisterPage (PageGraphicsItem& page)
	{
		connect (&page,
				&PageGraphicsItem::itemPainted,
				this,
				[&] { BumpPage (page); });
		connect (&page,
				&PageGraphicsItem::itemPixmapChanged,
				this,
				[&]
				{
					BumpPage (page);

					CurrentSize_ -= ItemsSizes_ [&page];
					CurrentSize_ += page.GetMemorySize ();
					ItemsSizes_ [&page] = page.GetMemorySize ();

					CheckCache ();
				});
		connect (&page,
				&QObject::destroyed,
				this,
				[&]
				{
					CurrentSize_ -= ItemsSizes_.take (&page);
					RecentlyUsed_.removeOne (&page);
				});
	}

	void PixmapCacheManager::BumpPage (PageGraphicsItem& page)
	{
		if (!RecentlyUsed_.isEmpty () && RecentlyUsed_.last () == &page)
			return;

		RecentlyUsed_.removeOne (&page);
		RecentlyUsed_ << &page;
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
