#include <boost/signals.hpp>
#include <WServer>
#include <WApplication>
#include <QStringList>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QUrl>
#include <QSettings>
#include <QtDebug>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>
#include "core.h"

Wt::WApplication* CreateApplication (const Wt::WEnvironment& e)
{
	return 0;
}

Core::Core ()
{
	Server_ = new Wt::WServer ();

	QStringList args = QCoreApplication::arguments ();
	int argc = args.size ();
	char **argv = new char* [argc];
	for (int i = 0; i < argc; ++i)
		argv [i] = args.at (i).toLatin1 ().data ();

//	Server_->setServerConfiguration (argc, argv, "config.xml");
	Server_->addEntryPoint (Wt::WServer::Application, &CreateApplication);
	try
	{
		if (!Server_->start ())
		{
			qWarning () << Q_FUNC_INFO << "could not start server";
		}
	}
	catch (const Wt::WServer::Exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
	}
}

Core& Core::Instance ()
{
    static Core inst;
    return inst;
}

void Core::Release ()
{
	try
	{
		Server_->stop ();
	}
	catch (const Wt::WServer::Exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
	}
	delete Server_;
}

void Core::AddObject (QObject *object, const QString& feature)
{
}

