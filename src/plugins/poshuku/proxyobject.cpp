#include "proxyobject.h"
#include "core.h"

using namespace LeechCraft::Plugins::Poshuku;

QMenu* ProxyObject::GetPluginsMenu () const
{
	return Core::Instance ().GetPluginsMenu ();
}

QNetworkAccessManager* ProxyObject::GetNetworkAccessManager () const
{
	return Core::Instance ().GetNetworkAccessManager ();
}


