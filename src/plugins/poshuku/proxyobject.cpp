#include "proxyobject.h"
#include "core.h"

QMenu* ProxyObject::GetPluginsMenu () const
{
	return Core::Instance ().GetPluginsMenu ();
}

