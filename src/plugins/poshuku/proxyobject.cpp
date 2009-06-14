#include "proxyobject.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			QMenu* ProxyObject::GetPluginsMenu () const
			{
				return Core::Instance ().GetPluginsMenu ();
			}
			
			QNetworkAccessManager* ProxyObject::GetNetworkAccessManager () const
			{
				return Core::Instance ().GetNetworkAccessManager ();
			}
		};
	};
};

