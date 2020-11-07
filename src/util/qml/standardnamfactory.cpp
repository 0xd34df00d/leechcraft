/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "standardnamfactory.h"
#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <util/network/networkdiskcache.h>

namespace LC::Util
{
	StandardNAMFactory::StandardNAMFactory (QString subpath,
			CacheSizeGetter_f getter,
			QQmlEngine *engine)
	: Subpath_ (std::move (subpath))
	, CacheSizeGetter_ (std::move (getter))
	{
		if (engine)
			engine->setNetworkAccessManagerFactory (this);
	}

	QNetworkAccessManager* StandardNAMFactory::create (QObject *parent)
	{
		auto nam = new QNetworkAccessManager (parent);

		auto cache = new NetworkDiskCache (Subpath_, nam);
		cache->setMaximumCacheSize (CacheSizeGetter_ ());
		nam->setCache (cache);

		return nam;
	}
}
