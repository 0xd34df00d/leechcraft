#ifndef PLUGINS_DBUSMANAGER_NOTIFICATIONMANAGER_H
#define PLUGINS_DBUSMANAGER_NOTIFICATIONMANAGER_H
#include <memory>
#include <QObject>
#include <QDBusInterface>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class NotificationManager : public QObject
			{
				Q_OBJECT

				std::auto_ptr<QDBusInterface> Connection_;
			public:
				NotificationManager (QObject* = 0);
				void HandleFinishedNotification (LeechCraft::IHookProxy*, const QString&, bool);
			};
		};
	};
};

#endif

