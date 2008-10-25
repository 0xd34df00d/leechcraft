#include "core.h"
#include <QtDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include "adaptor.h"

Core::Core ()
{
	connect (qApp,
			SIGNAL (aboutToQuit ()),
			this,
			SIGNAL (aboutToQuit ()));
	new Adaptor (this);

	QDBusConnection::sessionBus ().registerService ("org.LeechCraft.DBus");
	QDBusConnection::sessionBus ().registerObject ("/org/LeechCraft/Manager", this);
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Greeter (const QString& msg)
{
	qDebug () << Q_FUNC_INFO << msg;
}

void Core::DumpError ()
{
	qDebug () << Q_FUNC_INFO
		<< Connection_->lastError ().message ();
}

