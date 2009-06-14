#ifndef PLUGINS_DBUSMANAGER_CORE_H
#define PLUGINS_DBUSMANAGER_CORE_H
#include <memory>
#include <QObject>
#include <QDBusConnection>
#include <QStringList>
#include <interfaces/iinfo.h>
#include "notificationmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class Core : public QObject
			{
				Q_OBJECT

				std::auto_ptr<QDBusConnection> Connection_;
				std::auto_ptr<NotificationManager> NotificationManager_;

				ICoreProxy_ptr Proxy_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;
				QString Greeter (const QString&);
				QStringList GetLoadedPlugins ();
			private:
				void DumpError ();
			signals:
				void aboutToQuit ();
				void someEventHappened (const QString&);
			};
		};
	};
};

#endif

