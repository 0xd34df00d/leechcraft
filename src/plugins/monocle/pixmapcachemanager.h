/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Monocle
{
	class PageGraphicsItem;

	class PixmapCacheManager : public QObject
	{
		Q_OBJECT

		qint64 CurrentSize_ = 0;
		qint64 MaxSize_ = 0;
		QList<PageGraphicsItem*> RecentlyUsed_;
	public:
		PixmapCacheManager (QObject* = 0);

		void PixmapPainted (PageGraphicsItem*);
		void PixmapChanged (PageGraphicsItem*);
		void PixmapDeleted (PageGraphicsItem*);
	private:
		void CheckCache ();
	private slots:
		void handleCacheSizeChanged ();
	};
}
}
