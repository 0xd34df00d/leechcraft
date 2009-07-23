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

