#include <limits>
#include <list>
#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QtDebug>
#include <QTimer>
#include <QNetworkProxy>
#include <QApplication>
#include <QAction>
#include <QClipboard>
#include <QDir>
#include <QLocalServer>
#include "mainwindow.h"
#include "pluginmanager.h"
#include "core.h"
#include "exceptions/logic.h"
#include "exceptions/outofbounds.h"
#include "xmlsettingsmanager.h"
#include "mergemodel.h"

Main::Core::Core ()
: Server_ (new QLocalServer)
, MergeModel_ (new MergeModel)
{
	PluginManager_ = new Main::PluginManager (this);

    ClipboardWatchdog_ = new QTimer (this);
    connect (ClipboardWatchdog_,
			SIGNAL (timeout ()),
			this,
			SLOT (handleClipboardTimer ()));
    ClipboardWatchdog_->start (2000);

	Server_->listen ("LeechCraft local socket");
}

Main::Core::~Core ()
{
}

Main::Core& Main::Core::Instance ()
{
	static Core core;
	return core;
}

void Main::Core::Release ()
{
    PluginManager_->Release ();
    delete PluginManager_;
    ClipboardWatchdog_->stop ();
    delete ClipboardWatchdog_;

	Server_.reset ();
	MergeModel_.reset ();
}

void Main::Core::SetReallyMainWindow (Main::MainWindow *win)
{
    ReallyMainWindow_ = win;
}

Main::MainWindow* Main::Core::GetReallyMainWindow ()
{
    return ReallyMainWindow_;
}

QAbstractItemModel* Main::Core::GetPluginsModel () const
{
	return PluginManager_;
}

QAbstractProxyModel* Main::Core::GetTasksModel () const
{
	return MergeModel_.get ();
}

QWidget* Main::Core::GetControls (int index) const
{
	QAbstractItemModel *model = *MergeModel_->GetModelForRow (index);
	const IJobHolder *ijh = dynamic_cast<const IJobHolder*> (Representation2Object_ [model]);

	return ijh->GetControls ();
}

QWidget* Main::Core::GetAdditionalInfo (int index) const
{
	QAbstractItemModel *model = *MergeModel_->GetModelForRow (index);
	const IJobHolder *ijh = dynamic_cast<const IJobHolder*> (Representation2Object_ [model]);

	return ijh->GetAdditionalInfo ();
}

void Main::Core::DelayedInit ()
{
	connect (this,
			SIGNAL (error (QString)),
			parent (),
			SLOT (catchError (QString)));

    PluginManager_->InitializePlugins (ReallyMainWindow_);
    PluginManager_->CalculateDependencies ();
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        connect (this, SIGNAL (hidePlugins ()), plugin, SLOT (handleHidePlugins ()));
        IDownload *download = dynamic_cast<IDownload*> (plugin);
		IJobHolder *ijh = dynamic_cast<IJobHolder*> (plugin);

		const QMetaObject *qmo = plugin->metaObject ();

        if (download)
			connect (plugin,
					SIGNAL (fileDownloaded (const QString&)),
					this,
					SLOT (handleFileDownload (const QString&)));

		if (qmo->indexOfSignal (QMetaObject::
					normalizedSignature ("downloadFinished (const "
						"QString&)").constData ()) != -1)
			connect (plugin,
					SIGNAL (downloadFinished (const QString&)),
					this,
					SIGNAL (downloadFinished (const QString&)));

		if (ijh)
		{
			QAbstractItemModel *model = ijh->GetRepresentation ();
			Representation2Object_ [model] = plugin;
			MergeModel_->AddModel (model);

			QList<QAction*> actions = ijh->GetControls ()->actions ();
			for (QList<QAction*>::iterator i = actions.begin (),
					end = actions.end (); i != end; ++i)
			{
				connect (*i,
						SIGNAL (triggered ()),
						this,
						SLOT (handlePluginAction ()));
				Action2Model_ [*i] = model;
			}
		}
    }

	XmlSettingsManager::Instance ()->RegisterObject ("ProxyEnabled", this, "handleProxySettings");
	XmlSettingsManager::Instance ()->RegisterObject ("ProxyHost", this, "handleProxySettings");
	XmlSettingsManager::Instance ()->RegisterObject ("ProxyPort", this, "handleProxySettings");
	XmlSettingsManager::Instance ()->RegisterObject ("ProxyLogin", this, "handleProxySettings");
	XmlSettingsManager::Instance ()->RegisterObject ("ProxyPassword", this, "handleProxySettings");
	XmlSettingsManager::Instance ()->RegisterObject ("ProxyType", this, "handleProxySettings");

	handleProxySettings ();
}

bool Main::Core::ShowPlugin (IInfo::ID_t id)
{
    QObject *plugin = PluginManager_->FindByID (id);
    IWindow *w = qobject_cast<IWindow*> (plugin);
    if (w)
    {
        w->ShowWindow ();
        return true;
    }
    else
        return false;
}

void Main::Core::HideAll ()
{
    emit hidePlugins ();
}

void Main::Core::TryToAddJob (const QString& name, const QString& where)
{
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
		IInfo *ii = dynamic_cast<IInfo*> (plugin);
        IDownload *di = dynamic_cast<IDownload*> (plugin);
		LeechCraft::TaskParameters tp = LeechCraft::FromCommonDialog | LeechCraft::Autostart;
        if (di && di->CouldDownload (name, tp))
        {
			IDirectDownload *idd = dynamic_cast<IDirectDownload*> (plugin);
			IPeer2PeerDownload *ip2p =
				dynamic_cast<IPeer2PeerDownload*> (plugin);

			if (idd)
			{
				DirectDownloadParams ddp = { name, where };
				idd->AddJob (ddp, tp);
			}
			else if (ip2p)
				ip2p->AddJob (name, tp);
            return;
        }
    }
    emit error (tr ("No plugins are able to download \"%1\"").arg (name));
}

void Main::Core::Activated (const QModelIndex& index)
{
	ShowPlugin (index.row ());
}

void Main::Core::SetNewRow (const QModelIndex& index)
{
	MergeModel::const_iterator modIter = MergeModel_->GetModelForRow (index.row ());
	QObject *plugin = Representation2Object_ [*modIter];

	IEmbedModel *iee = dynamic_cast<IEmbedModel*> (plugin);
	qDebug () << iee;
	if (iee)
		iee->ItemSelected (MergeModel_->mapToSource (index));
}

QPair<qint64, qint64> Main::Core::GetSpeeds () const
{
    qint64 download = 0;
    qint64 upload = 0;
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        IDownload *di = dynamic_cast<IDownload*> (plugin);
        if (di)
        {
            download += di->GetDownloadSpeed ();
            upload += di->GetUploadSpeed ();
        }
    }

    return QPair<qint64, qint64> (download, upload);
}

void Main::Core::handleProxySettings () const
{
	bool enabled = XmlSettingsManager::Instance ()->property ("ProxyEnabled").toBool ();
	QNetworkProxy pr;
	if (enabled)
	{
		pr.setHostName (XmlSettingsManager::Instance ()->property ("ProxyHost").toString ());
		pr.setPort (XmlSettingsManager::Instance ()->property ("ProxyPort").toInt ());
		pr.setUser (XmlSettingsManager::Instance ()->property ("ProxyLogin").toString ());
		pr.setPassword (XmlSettingsManager::Instance ()->property ("ProxyPassword").toString ());
		QString type = XmlSettingsManager::Instance ()->property ("ProxyType").toString ();
		QNetworkProxy::ProxyType pt;
		if (type == "socks5")
			pt = QNetworkProxy::Socks5Proxy;
		else if (type == "tphttp")
			pt = QNetworkProxy::HttpProxy;
		else if (type == "chttp")
			pr = QNetworkProxy::HttpCachingProxy;
		else if (type == "cftp")
			pr = QNetworkProxy::FtpCachingProxy;
		pr.setType (pt);
	}
	else
		pr.setType (QNetworkProxy::NoProxy);
	QNetworkProxy::setApplicationProxy (pr);
}

void Main::Core::handlePluginAction ()
{
	QAction *source = dynamic_cast<QAction*> (sender ());
	QString slot = source->property ("Slot").toString ();
	QString signal = source->property ("Signal").toString ();

	if (slot.isEmpty () && signal.isEmpty ())
		return;

	QObject *object = source->property ("Object").value<QObject*> ();

	std::list<int> selected = ReallyMainWindow_->GetSelectedRows ();
	QAbstractItemModel *model = Action2Model_ [source];
	int startingRow = MergeModel_->GetStartingRow (MergeModel_->FindModel (model));
	int endingRow = startingRow + model->rowCount ();
	if (!slot.isEmpty ())
	{
		for (std::list<int>::const_iterator i = selected.begin (),
				end = selected.end (); i != end; ++i)
		{
			if (*i < startingRow)
				continue;
			if (*i >= endingRow)
				break;

			QMetaObject::invokeMethod (object,
					slot.toLatin1 (),
					Q_ARG (int, *i));
		}
	}
	if (!signal.isEmpty ())
	{
		for (std::list<int>::const_iterator i = selected.begin (),
				end = selected.end (); i != end; ++i)
		{
			if (*i < startingRow)
				continue;
			if (*i >= endingRow)
				break;

			QMetaObject::invokeMethod (object,
					signal.toLatin1 (),
					Q_ARG (int, *i));
		}
	}
}

void Main::Core::handleFileDownload (const QString& file, bool fromBuffer)
{
    if (!fromBuffer && !XmlSettingsManager::Instance ()->property ("QueryPluginsToHandleFinished").toBool ())
        return;

    QObjectList plugins = PluginManager_->GetAllCastableTo<IDownload*> ();
    for (int i = 0; i < plugins.size (); ++i)
    {
        IDownload *id = dynamic_cast<IDownload*> (plugins.at (i));
        IInfo *ii = dynamic_cast<IInfo*> (plugins.at (i));
		LeechCraft::TaskParameters tp = LeechCraft::Autostart | LeechCraft::FromAnother;
        if (id->CouldDownload (file, tp))
        {
            if (QMessageBox::question (qobject_cast<QWidget*> (qobject_cast<QObject*> (this)->parent ()),
                tr ("Question"),
                tr ("File %1 could be handled by plugin %2, would you like to?").arg (file).arg (ii->GetName ()),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
                continue;
			IDirectDownload *idd = qobject_cast<IDirectDownload*>
				(plugins.at (i));
			IPeer2PeerDownload *ip2p = qobject_cast<IPeer2PeerDownload*>
				(plugins.at (i));

			if (idd)
			{
				DirectDownloadParams ddp = { file, "" };
				idd->AddJob (ddp, tp);
			}
			else if (ip2p)
				ip2p->AddJob (file, tp);
        }
    }
}

void Main::Core::handleClipboardTimer ()
{
    QString text = QApplication::clipboard ()->text ();
    if (text.isEmpty () || text == PreviousClipboardContents_)
        return;

    PreviousClipboardContents_ = text;

    if (XmlSettingsManager::Instance ()->property ("WatchClipboard").toBool ())
        handleFileDownload (text, true);
}

