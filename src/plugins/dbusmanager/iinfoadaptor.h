#ifndef PLUGINS_DBUSMANAGER_IINFOADAPTOR_H
#define PLUGINS_DBUSMANAGER_IINFOADAPTOR_H
#include <QDBusAbstractAdaptor>

class IInfo;
class QObject;

namespace LeechCract
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class IInfoAdaptor : public QDBusAbstractAdaptor
			{
				Q_OBJECT

				IInfo *Object_;
			public:
				IInfoAdaptor (QObject*);
			};
		};
	};
};

#endif

