#include <limits>
#include <stdexcept>
#include <list>
#include <functional>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QtDebug>
#include <QTimer>
#include <QNetworkProxy>
#include <QApplication>
#include <QAction>
#include <QKeyEvent>
#include <QClipboard>
#include <QDir>
#include <QLocalServer>
#include <QTextCodec>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QFileDialog>
#include <QAuthenticator>
#include <QLocalSocket>
#include <QNetworkDiskCache>
#include <plugininterface/util.h>
#include <plugininterface/proxy.h>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iembedtab.h>
#include <interfaces/imultitabs.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iwindow.h>
#include <interfaces/structures.h>
#include <plugininterface/customcookiejar.h>
#include "application.h"
#include "mainwindow.h"
#include "pluginmanager.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "mergemodel.h"
#include "historymodel.h"
#include "authenticationdialog.h"
#include "sslerrorsdialog.h"
#include "sqlstoragebackend.h"
#include "requestparser.h"
#include "handlerchoicedialog.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

LeechCraft::Core::Core ()
: Server_ (new QLocalServer)
, MergeModel_ (new MergeModel (QStringList (tr ("Name"))
			<< tr ("State")
			<< tr ("Progress")))
, HistoryMergeModel_ (new MergeModel (QStringList (tr ("Filename"))
			<< tr ("Path")
			<< tr ("Date")))
, RequestNormalizer_ (new RequestNormalizer (MergeModel_, HistoryMergeModel_))
, NetworkAccessManager_ (new QNetworkAccessManager)
, CookieSaveTimer_ (new QTimer ())
, StorageBackend_ (new SQLStorageBackend ())
{
	MergeModel_->setProperty ("__LeechCraft_own_core_model", true);
	HistoryMergeModel_->setProperty ("__LeechCraft_own_core_model", true);
	connect (NetworkAccessManager_.get (),
			SIGNAL (authenticationRequired (QNetworkReply*,
					QAuthenticator*)),
			this,
			SLOT (handleAuthentication (QNetworkReply*,
					QAuthenticator*)));
	connect (NetworkAccessManager_.get (),
			SIGNAL (proxyAuthenticationRequired (const QNetworkProxy&,
					QAuthenticator*)),
			this,
			SLOT (handleProxyAuthentication (const QNetworkProxy&,
					QAuthenticator*)));
	connect (NetworkAccessManager_.get (),
			SIGNAL (sslErrors (QNetworkReply*,
					const QList<QSslError>&)),
			this,
			SLOT (handleSslErrors (QNetworkReply*,
					const QList<QSslError>&)));

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (file.open (QIODevice::ReadOnly))
	{
		CustomCookieJar *jar = new CustomCookieJar (this);
		jar->Load (file.readAll ());
		NetworkAccessManager_->setCookieJar (jar);
	}

	try
	{
		CreateIfNotExists ("core/cache");
		QNetworkDiskCache *cache = new QNetworkDiskCache (NetworkAccessManager_.get ());
		cache->setCacheDirectory (QDir::homePath () + "/.leechcraft/core/cache");
		NetworkAccessManager_->setCache (cache);
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO << e.what () << "so continuing without cache";
	}

	connect (CookieSaveTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);

	PluginManager_ = new PluginManager (this);
	connect (PluginManager_,
			SIGNAL (loadProgress (const QString&)),
			this,
			SIGNAL (loadProgress (const QString&)));

    ClipboardWatchdog_ = new QTimer (this);
    connect (ClipboardWatchdog_,
			SIGNAL (timeout ()),
			this,
			SLOT (handleClipboardTimer ()));
    ClipboardWatchdog_->start (2000);

	Server_->listen ("LeechCraft local socket");
	connect (Server_.get (),
			SIGNAL (newConnection ()),
			this,
			SLOT (handleNewLocalServerConnection ()));

	QList<QByteArray> proxyProperties;
	proxyProperties << "ProxyEnabled"
		<< "ProxyHost"
		<< "ProxyPort"
		<< "ProxyLogin"
		<< "ProxyPassword"
		<< "ProxyType"
		<< "CacheSize";
	XmlSettingsManager::Instance ()->RegisterObject (proxyProperties,
			this, "handleProxySettings");

	handleProxySettings ();
}

LeechCraft::Core::~Core ()
{
}

Core& LeechCraft::Core::Instance ()
{
	static Core core;
	return core;
}

void LeechCraft::Core::Release ()
{
	saveCookies ();
	XmlSettingsManager::Instance ()->setProperty ("FirstStart", "false");
	RequestNormalizer_.reset ();
	MergeModel_.reset ();
	HistoryMergeModel_.reset ();

    PluginManager_->Release ();
    delete PluginManager_;
    ClipboardWatchdog_->stop ();
    delete ClipboardWatchdog_;

	NetworkAccessManager_.reset ();

	Server_.reset ();
	StorageBackend_.reset ();
}

void LeechCraft::Core::SetReallyMainWindow (MainWindow *win)
{
    ReallyMainWindow_ = win;
	ReallyMainWindow_->GetTabWidget ()->installEventFilter (this);
}

QObjectList LeechCraft::Core::GetSettables () const
{
	return PluginManager_->GetAllCastableRoots<IHaveSettings*> ();
}

QAbstractItemModel* LeechCraft::Core::GetPluginsModel () const
{
	return PluginManager_;
}

QAbstractItemModel* LeechCraft::Core::GetTasksModel () const
{
	return RequestNormalizer_->GetModel ();
}

PluginManager* LeechCraft::Core::GetPluginManager () const
{
	return PluginManager_;
}

QWidget* LeechCraft::Core::GetControls (const QModelIndex& index) const
{
	QVariant data = index.data (RoleControls);
	return data.value<QWidget*> ();
}

QWidget* LeechCraft::Core::GetAdditionalInfo (const QModelIndex& index) const
{
	QVariant data = index.data (RoleAdditionalInfo);
	return data.value<QWidget*> ();
}

QStringList LeechCraft::Core::GetTagsForIndex (int index,
		QAbstractItemModel *model) const
{
	MergeModel::const_iterator modIter =
		dynamic_cast<MergeModel*> (model)->GetModelForRow (index);

	int starting = dynamic_cast<MergeModel*> (model)->
		GetStartingRow (modIter);

	return (*modIter)->
		data ((*modIter)->index (index - starting, 0), RoleTags)
		.toStringList ();
}

void LeechCraft::Core::DelayedInit ()
{
	connect (this,
			SIGNAL (error (QString)),
			ReallyMainWindow_,
			SLOT (catchError (QString)));

	TabContainer_.reset (new TabContainer (ReallyMainWindow_->GetTabWidget ()));
	XmlSettingsManager::Instance ()->RegisterObject ("ShowTabNames",
			TabContainer_.get (), "handleTabNames");

	emit loadProgress (tr ("Preinitialization..."));
	PluginManager_->CheckPlugins ();
	emit loadProgress (tr ("Calculation dependencies..."));
    PluginManager_->CalculateDependencies ();
    PluginManager_->InitializePlugins ();

    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        IInfo *info = qobject_cast<IInfo*> (plugin);
		emit loadProgress (tr ("Setting up %1...").arg (info->GetName ()));

		IJobHolder *ijh = qobject_cast<IJobHolder*> (plugin);
		IEmbedTab *iet = qobject_cast<IEmbedTab*> (plugin);
		IMultiTabs *imt = qobject_cast<IMultiTabs*> (plugin);

		const QMetaObject *qmo = plugin->metaObject ();

		if (qmo->indexOfSignal (QMetaObject::
					normalizedSignature ("gotEntity (const "
						"LeechCraft::DownloadEntity&)").constData ()) != -1)
			connect (plugin,
					SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
					this,
					SLOT (handleGotEntity (LeechCraft::DownloadEntity)));
		if (qmo->indexOfSignal (QMetaObject::
					normalizedSignature ("delegateEntity (const "
						"LeechCraft::DownloadEntity&, "
						"int*, QObject**)").constData ()) != -1)
			connect (plugin,
					SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
							int*, QObject**)),
					this,
					SLOT (handleGotEntity (LeechCraft::DownloadEntity,
							int*, QObject**)));
		if (qmo->indexOfSignal (QMetaObject::
					normalizedSignature ("downloadFinished (const "
						"QString&)").constData ()) != -1)
			connect (plugin,
					SIGNAL (downloadFinished (const QString&)),
					this,
					SIGNAL (downloadFinished (const QString&)));
		if (qmo->indexOfSignal (QMetaObject::
					normalizedSignature ("log (const "
						"QString&)").constData ()) != -1)
			connect (plugin,
					SIGNAL (log (const QString&)),
					this,
					SLOT (handleLog (const QString&)));

		if (ijh)
			InitJobHolder (plugin);

		if (iet)
			InitEmbedTab (plugin);

		if (imt)
			InitMultiTab (plugin);
	}

	TabContainer_->handleTabNames ();

	QTimer::singleShot (1000,
			this,
			SLOT (pullCommandLine ()));
}

bool LeechCraft::Core::ShowPlugin (int id)
{
    QObject *plugin = PluginManager_->data (PluginManager_->index (id, 0)).value <QObject*> ();
    IWindow *w = qobject_cast<IWindow*> (plugin);
    if (w)
    {
        w->ShowWindow ();
        return true;
    }
    else
        return false;
}

void LeechCraft::Core::TryToAddJob (const QString& name, const QString& where)
{
	DownloadEntity e;
	e.Entity_ = name.toUtf8 ();
	e.Location_ = where;
	e.Parameters_ = FromUserInitiated;

    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        IDownload *di = qobject_cast<IDownload*> (plugin);
        if (di && di->CouldDownload (e))
        {
			di->AddJob (e);
			return;
        }
    }
    emit error (tr ("No plugins are able to download \"%1\"").arg (name));
}

void LeechCraft::Core::SetNewRow (const QModelIndex& index)
{
	QModelIndex mapped = MapToSourceRecursively (index);
	QList<IJobHolder*> holders = PluginManager_->GetAllCastableTo<IJobHolder*> ();
	if (index.isValid ())
	{
		QObject *plugin = Representation2Object_ [mapped.model ()];

		IJobHolder *ijh = qobject_cast<IJobHolder*> (plugin);

		for (QList<IJobHolder*>::iterator i = holders.begin (),
				end = holders.end (); i != end; ++i)
			if (*i == ijh)
				(*i)->ItemSelected (mapped);
			else
				(*i)->ItemSelected (QModelIndex ());

		QObjectList watchers = PluginManager_->GetSelectedDownloaderWatchers ();
		foreach (QObject *pEntity, watchers)
			QMetaObject::invokeMethod (pEntity,
					"selectedDownloaderChanged",
					Q_ARG (QObject*, plugin));
		return;
	}
	else
		for (QList<IJobHolder*>::iterator i = holders.begin (),
				end = holders.end (); i != end; ++i)
			(*i)->ItemSelected (QModelIndex ());
}

bool LeechCraft::Core::SameModel (const QModelIndex& i1, const QModelIndex& i2) const
{
	QModelIndex mapped1 = MapToSourceRecursively (i1);
	QModelIndex mapped2 = MapToSourceRecursively (i2);
	return mapped1.model () == mapped2.model ();
}

void LeechCraft::Core::UpdateFiltering (const QString& text)
{
	RequestNormalizer_->SetRequest (text);
}

void LeechCraft::Core::Activated (const QModelIndex& index)
{
	DownloadEntity e =
	{
		index.data (HistoryModel::RolePath).toString ().toUtf8 (),
		QString (),
		index.data (RoleMime).toString (),
		FromUserInitiated
	};
	handleGotEntity (e);
}

QPair<qint64, qint64> LeechCraft::Core::GetSpeeds () const
{
    qint64 download = 0;
    qint64 upload = 0;
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        IDownload *di = qobject_cast<IDownload*> (plugin);
        if (di)
        {
            download += di->GetDownloadSpeed ();
            upload += di->GetUploadSpeed ();
        }
    }

    return QPair<qint64, qint64> (download, upload);
}

int LeechCraft::Core::CountUnremoveableTabs () const
{
	// + 1 because of tabs with downloaders
	return PluginManager_->GetAllCastableTo<IEmbedTab*> ().size () + 1;
}

QNetworkAccessManager* LeechCraft::Core::GetNetworkAccessManager () const
{
	return NetworkAccessManager_.get ();
}

bool LeechCraft::Core::eventFilter (QObject *watched, QEvent *e)
{
	if (ReallyMainWindow_ &&
			watched == ReallyMainWindow_->GetTabWidget ())
	{
		if (e->type () == QEvent::KeyRelease)
		{
			QKeyEvent *key = static_cast<QKeyEvent*> (e);
			bool handled = false;

			if (key->modifiers () & Qt::ControlModifier)
			{
				if (key->key () == Qt::Key_BracketLeft)
				{
					TabContainer_->RotateLeft ();
					handled = true;
				}
				else if (key->key () == Qt::Key_BracketRight)
				{
					TabContainer_->RotateRight ();
					handled = true;
				}
			}
			if (handled)
				return true;
			else
				TabContainer_->ForwardKeyboard (key);
		}
	}
	return QObject::eventFilter (watched, e);
}

void LeechCraft::Core::handleProxySettings () const
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
		QNetworkProxy::ProxyType pt = QNetworkProxy::HttpProxy;
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
	NetworkAccessManager_->setProxy (pr);

	static_cast<QNetworkDiskCache*> (NetworkAccessManager_->cache ())->
		setMaximumCacheSize (XmlSettingsManager::Instance ()->
				property ("CacheSize").toInt () * 1048576);
}

namespace
{
	void PassSelectionsByOne (QObject *object,
			const QModelIndexList& selected,
			const QByteArray& function)
	{
		for (QModelIndexList::const_iterator i = selected.begin (),
				end = selected.end (); i != end; ++i)
			QMetaObject::invokeMethod (object,
					function,
					Q_ARG (int, i->row ()));
	}
};

void LeechCraft::Core::handlePluginAction ()
{
	QAction *source = qobject_cast<QAction*> (sender ());
	QString slot = source->property ("Slot").toString ();
	QString signal = source->property ("Signal").toString ();
	QVariant varWhole = source->property ("WholeSelection");
	bool whole = varWhole.isValid () && varWhole.toBool ();

	if (slot.isEmpty () && signal.isEmpty ())
		return;

	QObject *object = source->property ("Object").value<QObject*> ();

	QModelIndexList origSelection = ReallyMainWindow_->GetSelectedRows ();
	QModelIndexList selected;
	for (QModelIndexList::const_iterator i = origSelection.begin (),
			end = origSelection.end (); i != end; ++i)
		selected.push_back (MapToSource (*i));

	if (whole)
	{
		std::deque<int> selections;
		for (QModelIndexList::const_iterator i = selected.begin (),
				end = selected.end (); i != end; ++i)
			selections.push_back (i->row ());

		if (!slot.isEmpty ())
			QMetaObject::invokeMethod (object,
					slot.toLatin1 (),
					Q_ARG (std::deque<int>, selections));

		if (!signal.isEmpty ())
			QMetaObject::invokeMethod (object,
					signal.toLatin1 (),
					Q_ARG (std::deque<int>, selections));
	}
	else
	{
		if (!slot.isEmpty ())
			PassSelectionsByOne (object, selected, slot.toLatin1 ());
		if (!signal.isEmpty ())
			PassSelectionsByOne (object, selected, signal.toLatin1 ());
	}
}

void LeechCraft::Core::toggleMultiwindow ()
{
	TabContainer_->ToggleMultiwindow ();
}

void LeechCraft::Core::deleteSelectedHistory (const QModelIndex& index)
{
	/* TODO reimplement in new terms
	QModelIndex mapped = FilterModel_->mapToSource (index);
	HistoryModel *model =
		static_cast<HistoryModel*> (*HistoryMergeModel_->
				GetModelForRow (mapped.row ()));
	model->RemoveItem (HistoryMergeModel_->mapToSource (mapped));
	*/
}

void LeechCraft::Core::handleGotEntity (DownloadEntity p, int *id, QObject **pr)
{
	QString string = tr ("Too long to show");
	if (p.Entity_.size () < 1000)
		string = QTextCodec::codecForName ("UTF-8")->toUnicode (p.Entity_);

	std::auto_ptr<HandlerChoiceDialog> dia (new HandlerChoiceDialog (string));

	QObjectList plugins = PluginManager_->GetAllCastableRoots<IDownload*> ();
	for (int i = 0; i < plugins.size (); ++i)
	{
		IDownload *id = qobject_cast<IDownload*> (plugins.at (i));
		IInfo *ii = qobject_cast<IInfo*> (plugins.at (i));
		if (id->CouldDownload (p))
			dia->Add (ii, id);
	}
	plugins = PluginManager_->GetAllCastableRoots<IEntityHandler*> ();
	for (int i = 0; i < plugins.size (); ++i)
	{
		IEntityHandler *ih = qobject_cast<IEntityHandler*> (plugins.at (i));
		IInfo *ii = qobject_cast<IInfo*> (plugins.at (i));
		if (ih->CouldHandle (p))
			dia->Add (ii, ih);
	}

	if (p.Parameters_ & FromUserInitiated)
	{
		if (!dia->NumChoices () ||
				dia->exec () == QDialog::Rejected)
			return;

		IDownload *sd = dia->GetDownload ();
		IEntityHandler *sh = dia->GetEntityHandler ();
		if (sd)
		{
			QString suggestion;
			if (p.Location_.size ())
				suggestion = QFileInfo (p.Location_).absolutePath ();
			else
				suggestion = XmlSettingsManager::Instance ()->Property ("EntitySavePath",
						QDesktopServices::storageLocation (QDesktopServices::DocumentsLocation))
					.toString ();
			QString dir = QFileDialog::getExistingDirectory (0,
					tr ("Select save path"),
					suggestion,
					!QFileDialog::ShowDirsOnly);

			if (dir.isEmpty ())
				return;

			XmlSettingsManager::Instance ()->
				setProperty ("EntitySavePath", dir);

			p.Location_ = dir;
			int l = sd->AddJob (p);
			if (id)
				*id = l;
			if (pr)
			{
				QObjectList plugins = PluginManager_->GetAllCastableRoots<IDownload*> ();
				*pr = *std::find_if (plugins.begin (), plugins.end (),
						boost::bind (std::equal_to<IDownload*> (),
							sd,
							boost::bind<IDownload*> (
								static_cast<IDownload* (*) (const QObject*)> (qobject_cast<IDownload*>),
								_1
								)));
			}
		}
		if (sh)
			sh->Handle (p);
	}
	else if (dia->GetDownload ())
	{
		IDownload *sd = dia->GetDownload ();
		if (p.Location_.isEmpty ())
			p.Location_ = QDir::tempPath ();
		int l = sd->AddJob (p);
		if (id)
			*id = l;
		if (pr)
		{
			QObjectList plugins = PluginManager_->GetAllCastableRoots<IDownload*> ();
			*pr = *std::find_if (plugins.begin (), plugins.end (),
					boost::bind (std::equal_to<IDownload*> (),
						sd,
						boost::bind<IDownload*> (
							static_cast<IDownload* (*) (const QObject*)> (qobject_cast<IDownload*>),
							_1
							)));
		}
	}
	else
		emit log (tr ("Could not handle download entity %1.")
				.arg (string));
}

void LeechCraft::Core::handleClipboardTimer ()
{
    QString text = QApplication::clipboard ()->text ();
    if (text.isEmpty () || text == PreviousClipboardContents_)
        return;

    PreviousClipboardContents_ = text;

	DownloadEntity e =
	{
		text.toUtf8 (),
		QString (),
		QString (),
		FromUserInitiated
	};

    if (XmlSettingsManager::Instance ()->property ("WatchClipboard").toBool ())
        handleGotEntity (e);
}

void LeechCraft::Core::embeddedTabWantsToFront ()
{
	IEmbedTab *iet = qobject_cast<IEmbedTab*> (sender ());
	if (!iet)
		return;

	ReallyMainWindow_->show ();
	TabContainer_->bringToFront (iet->GetTabContents ());
}

void LeechCraft::Core::handleStatusBarChanged (QWidget *contents, const QString& msg)
{
	if (contents->visibleRegion ().isEmpty ())
		return;

	ReallyMainWindow_->statusBar ()->showMessage (msg, 30000);
}

void LeechCraft::Core::handleLog (const QString& message)
{
	IInfo *ii = qobject_cast<IInfo*> (sender ());
	emit log (ii->GetName () + ": " + message);
}

void LeechCraft::Core::handleAuthentication (QNetworkReply *reply, QAuthenticator *authen)
{
	QString msg = tr ("The URL<br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (reply->url ().toString ())
		.arg (authen->realm ());
	msg = msg.left (200);

	DoCommonAuth (msg, authen);
}

void LeechCraft::Core::handleProxyAuthentication (const QNetworkProxy& proxy, QAuthenticator *authen)
{
	QString msg = tr ("The proxy <br /><code>%1</code><br />with "
			"realm<br /><em>%2</em><br />requires authentication.")
		.arg (proxy.hostName () + ":" + QString::number (proxy.port ()))
		.arg (authen->realm ());
	msg = msg.left (200);

	DoCommonAuth (msg, authen);
}

void LeechCraft::Core::handleSslErrors (QNetworkReply *reply, const QList<QSslError>& errors)
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_Poshuku");
	settings.beginGroup ("SSL exceptions");
	QStringList keys = settings.allKeys ();
	if (keys.contains (reply->url ().toString ())) 
	{
		if (settings.value (reply->url ().toString ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else if (keys.contains (reply->url ().host ()))
	{
		if (settings.value (reply->url ().host ()).toBool ())
			reply->ignoreSslErrors ();
	}
	else
	{
		QString msg = tr ("<code>%1</code><br />has SSL errors."
				" What do you want to do?")
			.arg (reply->url ().toString ());
		std::auto_ptr<SslErrorsDialog> dia (
				new SslErrorsDialog (msg,
					errors,
					qApp->activeWindow ())
				);

		bool ignore = (dia->exec () == QDialog::Accepted);
		if (ignore)
			reply->ignoreSslErrors ();

		SslErrorsDialog::RememberChoice choice = dia->GetRememberChoice ();

		if (choice != SslErrorsDialog::RCNot)
		{
			if (choice == SslErrorsDialog::RCFile)
				settings.setValue (reply->url ().toString (),
						ignore);
			else
				settings.setValue (reply->url ().host (),
						ignore);
		}
	}
	settings.endGroup ();
}

void LeechCraft::Core::pullCommandLine ()
{
	QStringList arguments = qobject_cast<Application*> (qApp)->Arguments ();
	if (!(arguments.size () > 1 &&
			!arguments.last ().startsWith ('-')))
		return;

	DownloadEntity e =
	{
		arguments.last ().toUtf8 (),
		QString (),
		QString (),
		FromUserInitiated
	};
	handleGotEntity (e);
}

void LeechCraft::Core::handleNewLocalServerConnection ()
{
	ReallyMainWindow_->show ();
	ReallyMainWindow_->activateWindow ();
	ReallyMainWindow_->raise ();
	std::auto_ptr<QLocalSocket> socket (Server_->nextPendingConnection ());
	// I think 100 msecs would be more than enough for the local
	// connections.
	if (!socket->bytesAvailable ())
		socket->waitForReadyRead (1000);

	QByteArray read = socket->readAll ();
	QDataStream in (read);
	QStringList arguments;
	in >> arguments;

	if (!(arguments.size () > 1 &&
			!arguments.last ().startsWith ('-')))
		return;

	DownloadEntity e =
	{
		arguments.last ().toUtf8 (),
		QString (),
		QString (),
		FromUserInitiated
	};
	handleGotEntity (e);
}

void LeechCraft::Core::saveCookies () const
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	if (!dir.exists ("core") &&
			!dir.mkdir ("core"))
	{
		emit error (tr ("Could not create Core directory."));
		return;
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		emit error (tr ("Could not save cookies, error opening cookie file."));
	else
		file.write (static_cast<CustomCookieJar*> (NetworkAccessManager_->cookieJar ())->Save ());
}

void LeechCraft::Core::DoCommonAuth (const QString& msg, QAuthenticator *authen)
{
	QString realm = authen->realm ();

	QString suggestedUser = authen->user ();
	QString suggestedPassword = authen->password ();

	if (suggestedUser.isEmpty ())
		StorageBackend_->GetAuth (realm, suggestedUser, suggestedPassword);

	std::auto_ptr<AuthenticationDialog> dia (
			new AuthenticationDialog (msg,
				suggestedUser,
				suggestedPassword,
				qApp->activeWindow ())
			);
	if (dia->exec () == QDialog::Rejected)
		return;

	QString login = dia->GetLogin ();
	QString password = dia->GetPassword ();
	authen->setUser (login);
	authen->setPassword (password);

	if (dia->ShouldSave ())
		StorageBackend_->SetAuth (realm, login, password);
}

QModelIndex LeechCraft::Core::MapToSource (const QModelIndex& index) const
{
	return MapToSourceRecursively (index);
}

void LeechCraft::Core::InitJobHolder (QObject *plugin)
{
	IJobHolder *ijh = qobject_cast<IJobHolder*> (plugin);
	QAbstractItemModel *model = ijh->GetRepresentation ();
	Representation2Object_ [model] = plugin;
	MergeModel_->AddModel (model);

	if (ijh->GetRepresentation ())
	{
		QWidget *controlsWidget = ijh->GetRepresentation ()->
			index (0, 0).data (RoleControls).value<QWidget*> ();
		if (controlsWidget)
		{
			QList<QAction*> actions = controlsWidget->actions ();
			for (QList<QAction*>::iterator i = actions.begin (),
					end = actions.end (); i != end; ++i)
			{
				connect (*i,
						SIGNAL (triggered ()),
						this,
						SLOT (handlePluginAction ()));
				Action2Model_ [*i] = model;
			}

			controlsWidget->setParent (ReallyMainWindow_);
		}
		QWidget *additional = ijh->GetRepresentation ()->
			index (0, 0).data (RoleAdditionalInfo).value<QWidget*> ();
		if (additional)
			additional->setParent (ReallyMainWindow_);
	}

	QAbstractItemModel *historyModel = ijh->GetHistory ();
	if (historyModel)
	{
		History2Object_ [historyModel] = plugin;
		HistoryMergeModel_->AddModel (historyModel);
	}
}

void LeechCraft::Core::InitEmbedTab (QObject *plugin)
{
	IInfo *ii = qobject_cast<IInfo*> (plugin);
	IEmbedTab *iet = qobject_cast<IEmbedTab*> (plugin);
	TabContainer_->add (ii->GetName (),
			iet->GetTabContents (),
			ii->GetIcon ());
	connect (plugin,
			SIGNAL (bringToFront ()),
			this,
			SLOT (embeddedTabWantsToFront ()));
}

void LeechCraft::Core::InitMultiTab (QObject *plugin)
{
	connect (plugin,
			SIGNAL (addNewTab (const QString&, QWidget*)),
			TabContainer_.get (),
			SLOT (add (const QString&, QWidget*)));
	connect (plugin,
			SIGNAL (removeTab (QWidget*)),
			TabContainer_.get (),
			SLOT (remove (QWidget*)));
	connect (plugin,
			SIGNAL (changeTabName (QWidget*, const QString&)),
			TabContainer_.get (),
			SLOT (changeTabName (QWidget*, const QString&)));
	connect (plugin,
			SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
			TabContainer_.get (),
			SLOT (changeTabIcon (QWidget*, const QIcon&)));
	connect (plugin,
			SIGNAL (changeTooltip (QWidget*, QWidget*)),
			TabContainer_.get (),
			SLOT (changeTooltip (QWidget*, QWidget*)));
	connect (plugin,
			SIGNAL (statusBarChanged (QWidget*, const QString&)),
			this,
			SLOT (handleStatusBarChanged (QWidget*, const QString&)));
	connect (plugin,
			SIGNAL (raiseTab (QWidget*)),
			TabContainer_.get (),
			SLOT (bringToFront (QWidget*)));
}

QModelIndex LeechCraft::Core::MapToSourceRecursively (QModelIndex index) const
{
	const QAbstractProxyModel *model = 0;
	while ((model = qobject_cast<const QAbstractProxyModel*> (index.model ())) &&
			model->property ("__LeechCraft_own_core_model").toBool ())
		index = model->mapToSource (index);
	return index;
}

