#ifndef ADAPTOR_H
#define ADAPTOR_H
#include <QDBusAbstractAdaptor>

class Core;
class QDBusMessage;

class Adaptor : public QDBusAbstractAdaptor
{
	Q_OBJECT

	Q_CLASSINFO ("D-Bus Interface", "org.leechcraft.DBus.Manager");
	Q_PROPERTY (QString OrganizationName READ GetOrganizationName);
	Q_PROPERTY (QString ApplicationName READ GetApplicationName);

	Core *Core_;
public:
	Adaptor (Core*);

	QString GetOrganizationName () const;
	QString GetApplicationName () const;
public slots:
	QString Greeter (const QString&, const QDBusMessage&, QString&);
signals:
	void aboutToQuit ();
};

#endif

