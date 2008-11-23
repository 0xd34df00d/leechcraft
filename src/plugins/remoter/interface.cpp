#include "interface.h"
#include <WApplication>
#include <WEnvironment>
#include <WContainerWidget>
#include <WTreeView>
#include <WTabWidget>
#include <WTreeView>
#include <WBreak>
#include <WText>
#include <plugininterface/mergemodel.h>
#include "core.h"
#include "qtowabstractitemmodeladaptor.h"
#include "qtowtoolbaradaptor.h"
#include "downloaderswidget.h"

Interface::Interface (Wt::WApplication *app, const Wt::WEnvironment& e)
{
	BuildInterface (app->root (), e);
}

void Interface::BuildInterface (Wt::WContainerWidget *root, const Wt::WEnvironment& e)
{
	if (!Core::Instance ().GetTasksModel () ||
			!Core::Instance ().GetHistoryModel ())
	{
		new Wt::WText (Wt::WString (QObject::tr ("LeechCraft::Remoter "
						"isn't initialized yet, sorry. Please try again "
						"in a couple of seconds.").toStdString ()));
		return;
	}

	Wt::WTabWidget *tabWidget = new Wt::WTabWidget (root);

	DownloadersWidget_ = new DownloadersWidget (0);
	QToWAbstractItemModelAdaptor *adaptor =
		new QToWAbstractItemModelAdaptor (Core::Instance ().GetTasksModel (),
				DownloadersWidget_);
	DownloadersWidget_->SetModel (adaptor);

	/*
	IJobHolder *ijh = 0;
	for (int i = 0; i < Objects_.size (); ++i)
		if ((ijh = qobject_cast<IJobHolder*> (Objects_ [i])))
			new QToWToolbarAdaptor (dynamic_cast<QToolBar*> (ijh->GetControls ()),
					root);
					*/

	tabWidget->addTab (DownloadersWidget_,
			QObject::tr ("Downloaders").toStdString ());

	Wt::WTreeView *historyView = new Wt::WTreeView (0);
	SetupHistoryView (historyView);

	tabWidget->addTab (historyView,
			QObject::tr ("Download history").toStdString ());

	new Wt::WBreak (root);

	int majVer = 0, minVer = 0, patchVer = 0;
	e.libraryVersion (majVer, minVer, patchVer);

	new Wt::WText (Wt::WString ("Rendered by Wt {1}.{2}.{3} for LeechCraft")
			.arg (majVer)
			.arg (minVer)
			.arg (patchVer),
			root);
}

void Interface::SetupHistoryView (Wt::WTreeView *view)
{
	QToWAbstractItemModelAdaptor *adaptor =
		new QToWAbstractItemModelAdaptor (Core::Instance ().GetHistoryModel (),
				view);
	view->setModel (adaptor);
}


