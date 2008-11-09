#include <WServer>
#include "core.h"
#include <WApplication>
#include <QStringList>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QUrl>
#include <QSettings>
#include <QtDebug>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>

Wt::WApplication* CreateApplication (const Wt::WEnvironment& e)
{
	return 0;
}

Core::Core ()
: Server_ (0)
{
	InitializeServer ();
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

void Core::InitializeServer ()
{
	delete Server_;
	Server_ = new Wt::WServer ();

	// Some black magic to reconstruct the argc/argv passed to us by the
	// OS as Wt requires at.
	int delta = 0;
	QStringList args = QCoreApplication::arguments ();
	if (!args.contains ("--docroot"))
		delta += 2;
	if (!args.contains ("--http-address"))
		delta += 2;

	int argc = args.size ();
	char **argv = new char* [argc + delta];
	for (int i = 0; i < argc; ++i)
	{
		QByteArray wa = args.at (i).toLocal8Bit ();
		argv [i] = new char [wa.size ()];
		std::strcpy (argv [i], wa.constData ());
	}
	int allocDelta = delta;
	if (!args.contains ("--docroot"))
	{
		argv [argc + allocDelta - 2] = "--docroot";
		argv [argc + allocDelta - 1] = ".";
		allocDelta -= 2;
	}
	if (!args.contains ("--http-address"))
	{
		argv [argc + allocDelta - 2] = "--http-address";
		argv [argc + allocDelta - 1] = "127.0.0.1";
	}

	try
	{
		Server_->setServerConfiguration (argc + delta, argv, "");
	}
	catch (const Wt::WServer::Exception& e)
	{
		qWarning () << Q_FUNC_INFO << "could not set server's configuration:";
		qWarning () << e.what ();
		delete Server_;
		return;
	}

	// As after every black magic, we should clean up.
	for (int i = 0; i < argc; ++i)
		delete [] argv [i];
	delete [] argv;

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
		delete Server_;
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO << e.what ();
		delete Server_;
	}
}

