/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "standardnamfactory.h"
#include <chrono>
#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <util/network/networkdiskcache.h>
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC::Util
{
	StandardNAMFactory::StandardNAMFactory (QString subpath,
			CacheSizeGetter_f getter,
			BaseSettingsManager& settings,
			QQmlEngine *engine)
	: Subpath_ (std::move (subpath))
	, CacheSizeGetter_ (std::move (getter))
	{
		settings.RegisterObject ("TransferTimeout",
				this,
				[this] (int seconds)
				{
					const auto newTimeout = std::chrono::seconds { seconds };
					RequestTimeout_ = newTimeout;
					emit requestTimeoutChanged (newTimeout);
				});

		if (engine)
			engine->setNetworkAccessManagerFactory (this);
	}

	QNetworkAccessManager* StandardNAMFactory::create (QObject *parent)
	{
		auto nam = new QNetworkAccessManager (parent);
		connect (this,
				&StandardNAMFactory::requestTimeoutChanged,
				nam,
				qOverload<std::chrono::milliseconds> (&QNetworkAccessManager::setTransferTimeout));

		auto cache = new NetworkDiskCache (Subpath_, nam);
		cache->setMaximumCacheSize (CacheSizeGetter_ ());
		nam->setCache (cache);
		nam->setTransferTimeout (RequestTimeout_.load ());

		return nam;
	}
}
