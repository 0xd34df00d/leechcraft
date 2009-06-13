#ifndef PROXYOBJECT_H
#define PROXYOBJECT_H
#include <QObject>
#include "interfaces/iproxyobject.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class ProxyObject : public QObject
							  , public IProxyObject
			{
				Q_OBJECT
				Q_INTERFACES (LeechCraft::Plugins::Poshuku::IProxyObject);
			public:
				QMenu* GetPluginsMenu () const;
				QNetworkAccessManager* GetNetworkAccessManager () const;
			};
		};
	};
};

#endif

