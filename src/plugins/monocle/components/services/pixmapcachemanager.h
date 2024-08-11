/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>

namespace LC::Monocle
{
	class PageGraphicsItem;

	class PixmapCacheManager : public QObject
	{
		qint64 CurrentSize_ = 0;
		qint64 MaxSize_ = 0;

		QVector<PageGraphicsItem*> RecentlyUsed_;
		QHash<PageGraphicsItem*, int> ItemsSizes_;
	public:
		explicit PixmapCacheManager (QObject* = nullptr);

		void PixmapPainted (PageGraphicsItem*);
		void PixmapChanged (PageGraphicsItem*);
		void PixmapDeleted (PageGraphicsItem*);
	private:
		void CheckCache ();
	};
}
