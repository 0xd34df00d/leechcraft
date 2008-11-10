#include <WServer>
#include "core.h"
#include <WText>
#include <WContainerWidget>
#include <WApplication>
#include <WBreak>
#include <WTabWidget>
#include <WTreeView>
#include <WEnvironment>
#include <QStringList>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QUrl>
#include <QSettings>
#include <QtDebug>
#include <interfaces/interfaces.h>
#include <plugininterface/proxy.h>
#include <plugininterface/mergemodel.h>
#include "qtowabstractitemmodeladaptor.h"

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

	BuildInterface (result->root (), e);

	return result;
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

void Core::BuildInterface (Wt::WContainerWidget *root, const Wt::WEnvironment& e)
{
	if (!TasksModel_ || !HistoryModel_)
	{
		new Wt::WText (Wt::WString (tr ("LeechCraft::Remoter isn't "
						"initialized yet, sorry. Please try again "
						"in a couple of seconds.").toStdString ()));
		return;
	}

	Wt::WTabWidget *tabWidget = new Wt::WTabWidget (root);

	Wt::WTreeView *downloadersView = new Wt::WTreeView (root);
	SetupDownloadersView (downloadersView);

	Wt::WTreeView *historyView = new Wt::WTreeView (root);
	SetupHistoryView (historyView);

	tabWidget->addTab (downloadersView,
			tr ("Downloaders").toStdString ());
	tabWidget->addTab (historyView,
			tr ("Download history").toStdString ());

	new Wt::WBreak (root);

	int majVer = 0, minVer = 0, patchVer = 0;
	e.libraryVersion (majVer, minVer, patchVer);

	new Wt::WText (Wt::WString ("Rendered by Wt {1}.{2}.{3} for LeechCraft")
			.arg (majVer)
			.arg (minVer)
			.arg (patchVer),
			root);
}

void Core::SetupDownloadersView (Wt::WTreeView *view)
{
	QToWAbstractItemModelAdaptor *adaptor =
		new QToWAbstractItemModelAdaptor (TasksModel_, view);
	view->setModel (adaptor);
}

void Core::SetupHistoryView (Wt::WTreeView *view)
{
	QToWAbstractItemModelAdaptor *adaptor =
		new QToWAbstractItemModelAdaptor (HistoryModel_, view);
	view->setModel (adaptor);
}

