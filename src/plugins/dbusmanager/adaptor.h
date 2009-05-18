#ifndef ADAPTOR_H
#define ADAPTOR_H
#include <QDBusAbstractAdaptor>
#include <QStringList>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DBusManager
		{
			class Core;
			class QDBusMessage;

			class Adaptor : public QDBusAbstractAdaptor
			{
				Q_OBJECT

				Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.DBus.Manager");
				Q_PROPERTY (QString OrganizationName READ GetOrganizationName);
				Q_PROPERTY (QString ApplicationName READ GetApplicationName);

				Core *Core_;
			public:
				Adaptor (Core*);

				QString GetOrganizationName () const;
				QString GetApplicationName () const;
			public slots:
				QString Greeter (const QString&, const QDBusMessage&);
				QStringList GetLoadedPlugins ();
			signals:
				void aboutToQuit ();
				void someEventHappened (const QString&);
			};
		};
	};
};

#endif

