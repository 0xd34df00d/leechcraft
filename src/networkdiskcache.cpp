#include "networkdiskcache.h"

NetworkDiskCache::NetworkDiskCache (QObject *parent)
: QNetworkDiskCache (parent)
{
}

QIODevice* NetworkDiskCache::prepare (const QNetworkCacheMetaData& metadata)
{
	if (metadata.url ().scheme () == "https")
		return 0;
	else
		return QNetworkDiskCache::prepare (metadata);
}

