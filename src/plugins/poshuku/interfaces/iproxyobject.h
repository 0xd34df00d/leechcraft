#ifndef IPROXYOBJECT_H
#define IPROXYOBJECT_H
#include <QtPlugin>

class QMenu;
class QNetworkAccessManager;

namespace LeechCraft
{
	namespace Poshuku
	{
		class IProxyObject
		{
		public:
			virtual QMenu* GetPluginsMenu () const = 0;
			virtual QNetworkAccessManager* GetNetworkAccessManager () const = 0;
		};
	};
};

Q_DECLARE_INTERFACE (LeechCraft::Poshuku::IProxyObject, "org.Deviant.LeechCraft.Poshuku.IProxyObject/1.0");

#endif

