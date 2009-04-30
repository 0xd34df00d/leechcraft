#include "proxyobject.h"
#include "core.h"

QMenu* ProxyObject::GetPluginsMenu () const
{
	return Core::Instance ().GetPluginsMenu ();
}

QNetworkAccessManager* ProxyObject::GetNetworkAccessManager () const
{
	return Core::Instance ().GetNetworkAccessManager ();
}

