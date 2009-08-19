/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "networkdiskcache.h"
#include <QtDebug>
#include <QDateTime>
#include <QDir>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	NetworkDiskCache::NetworkDiskCache (QObject *parent)
	: QNetworkDiskCache (parent)
	{
		setCacheDirectory (QDir::homePath () + "/.leechcraft/core/cache");

		XmlSettingsManager::Instance ()->RegisterObject ("CacheSize",
				this, "handleCacheSize");
		handleCacheSize ();
	}

	QIODevice* NetworkDiskCache::prepare (const QNetworkCacheMetaData& metadata)
	{
		// Some quirks for disk cache, for example, don't cache https://
		// stuff as well as scripts.
		if (QString ("4.5.1") == qVersion () &&
				(metadata.url ().scheme () == "https" ||
				 metadata.url ().path ().endsWith ("js")))
			return 0;
		else
			return QNetworkDiskCache::prepare (metadata);
	}

	void NetworkDiskCache::handleCacheSize ()
	{
		setMaximumCacheSize (XmlSettingsManager::Instance ()->
				property ("CacheSize").toInt () * 1048576);
		expire ();
	}
};

