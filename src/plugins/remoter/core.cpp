#include <WServer>
#include "core.h"
#include <WApplication>
#include <WEnvironment>
#include <QStringList>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QUrl>
#include <QToolBar>
#include <QSettings>
#include <QtDebug>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>
#include <plugininterface/mergemodel.h>
#include "qtowabstractitemmodeladaptor.h"
#include "qtowtoolbaradaptor.h"
#include "interface.h"

using LeechCraft::Util::MergeModel;

Wt::WApplication* CreateApplication (const Wt::WEnvironment& e)
{
	return Core::Instance ().CreateApplication (e);;
}

Core::Core ()
: Server_ (0)
, TasksModel_ (0)
, HistoryModel_ (0)
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
		if (Server_)
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
	Server_ = 0;
}

void Core::SetHistoryModel (MergeModel *model)
{
	HistoryModel_ = model;
}

void Core::SetDownloadersModel (MergeModel *model)
{
	TasksModel_ = model;
}

void Core::AddObject (QObject *object, const QString& feature)
{
	Objects_ << object;
}

Wt::WApplication* Core::CreateApplication (const Wt::WEnvironment& e)
{
	Wt::WApplication *result = new Wt::WApplication (e);
	result->setTitle ("LeechCraft");

	Interface *interface = new Interface (result, e);

	return result;
}

MergeModel* Core::GetTasksModel () const
{
	return TasksModel_;
}

MergeModel* Core::GetHistoryModel () const
{
	return HistoryModel_;
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
	if (!args.contains ("--http-port"))
		delta += 2;

	int argc = args.size ();
	char **argv = new char* [argc + delta];
	for (int i = 0; i < argc; ++i)
	{
		QByteArray wa = args.at (i).toLocal8Bit ();
		argv [i] = new char [wa.size () + 1];
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
		allocDelta -= 2;
	}
	if (!args.contains ("--http-port"))
	{
		argv [argc + allocDelta - 2] = "--http-port";
		argv [argc + allocDelta - 1] = "14600";
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
		Server_ = 0;
		return;
	}

	// As after every black magic, we should clean up.
	for (int i = 0; i < argc; ++i)
		delete [] argv [i];
	delete [] argv;

	Server_->addEntryPoint (Wt::WServer::Application, &::CreateApplication);
	try
	{
		if (!Server_->start ())
		{
			qWarning () << Q_FUNC_INFO << "could not start server";
		}
	}
	catch (const Wt::WServer::Exception& e)
	{
		qWarning () << Q_FUNC_INFO << "Failed to start server:";
		qWarning () << e.what ();
		delete Server_;
		Server_ = 0;
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO << "Failed to start server:";
		qWarning () << e.what ();
		delete Server_;
		Server_ = 0;
	}
}

