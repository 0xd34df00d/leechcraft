#include "networkmonitor.h"
#include <typeinfo>
#include <QMenu>
#include "requestmodel.h"
#include "headermodel.h"

using namespace LeechCraft::Plugins;
using namespace LeechCraft::Plugins::NetworkMonitor;

void Plugin::Init (ICoreProxy_ptr proxy)
{
	NetworkAccessManager_ = proxy->GetNetworkAccessManager ();

	Ui_.setupUi (this);

	Model_ = new RequestModel (this);
	Ui_.RequestsView_->setModel (Model_);
	connect (Ui_.RequestsView_->selectionModel (),
			SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
			Model_,
			SLOT (handleCurrentChanged (const QModelIndex&)));

	Ui_.RequestHeadersView_->setModel (Model_->GetRequestHeadersModel ());
	Ui_.ReplyHeadersView_->setModel (Model_->GetReplyHeadersModel ());

	connect (Ui_.ClearFinished_,
			SIGNAL (toggled (bool)),
			Model_,
			SLOT (setClear (bool)));

	connect (NetworkAccessManager_,
			SIGNAL (requestCreated (QNetworkAccessManager::Operation,
					const QNetworkRequest&, QNetworkReply*)),
			Ui_.RequestsView_->model (),
			SLOT (handleRequest (QNetworkAccessManager::Operation,
					const QNetworkRequest&, QNetworkReply*)));

	QAction *showAction = new QAction (tr ("Network monitor..."),
			this);
	showAction->setProperty ("ActionIcon", "networkmonitor_plugin");
	connect (showAction,
			SIGNAL (triggered ()),
			this,
			SLOT (show ()));
	Actions_.push_back (showAction);
}

void Plugin::Release ()
{
	qDeleteAll (Actions_);
}

QString Plugin::GetName () const
{
	return "NetworkMonitor";
}

QString Plugin::GetInfo () const
{
	return tr ("Monitors HTTP network requests.");
}

QIcon Plugin::GetIcon () const
{
	return QIcon ();
}

QStringList Plugin::Provides () const
{
	return QStringList ();
}

QStringList Plugin::Needs () const
{
	return QStringList ();
}

QStringList Plugin::Uses () const
{
	return QStringList ();
}

void Plugin::SetProvider (QObject*, const QString&)
{
}

QList<QAction*> Plugin::GetActions () const
{
	return Actions_;
}

Q_EXPORT_PLUGIN2 (leechcraft_networkmonitor, Plugin);

