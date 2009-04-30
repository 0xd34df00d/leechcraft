#ifndef PROXYOBJECT_H
#define PROXYOBJECT_H
#include <QObject>
#include "interfaces/iproxyobject.h"

class ProxyObject : public QObject
				  , public LeechCraft::Poshuku::IProxyObject
{
	Q_OBJECT
	Q_INTERFACES (LeechCraft::Poshuku::IProxyObject);
public:
	QMenu* GetPluginsMenu () const;
};

#endif

