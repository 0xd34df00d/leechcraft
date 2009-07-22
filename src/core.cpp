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
#include <QToolBar>
#include <QKeyEvent>
#include <QClipboard>
#include <QDir>
#include <QLocalServer>
#include <QTextCodec>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QFileDialog>
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
#include <interfaces/ihaveshortcuts.h>
#include <interfaces/iwindow.h>
#include <interfaces/itoolbarembedder.h>
#include <interfaces/structures.h>
#include "application.h"
#include "mainwindow.h"
#include "pluginmanager.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "mergemodel.h"
#include "sqlstoragebackend.h"
#include "requestparser.h"
#include "handlerchoicedialog.h"
#include "tagsmanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Util;

HookProxy::HookProxy ()
: Cancelled_ (false)
{
}

HookProxy::~HookProxy ()
{
}

void HookProxy::CancelDefault ()
{
	Cancelled_ = true;
}

bool HookProxy::IsCancelled () const
{
	return Cancelled_;
}

LeechCraft::Core::Core ()
: Server_ (new QLocalServer)
, MergeModel_ (new MergeModel (QStringList (tr ("Name"))
			<< tr ("State")
			<< tr ("Progress")))
, RequestNormalizer_ (new RequestNormalizer (MergeModel_))
, NetworkAccessManager_ (new NetworkAccessManager)
, StorageBackend_ (new SQLStorageBackend)
, DirectoryWatcher_ (new DirectoryWatcher)
{
	MergeModel_->setObjectName ("Core MergeModel");
	MergeModel_->setProperty ("__LeechCraft_own_core_model", true);
	connect (NetworkAccessManager_.get (),
			SIGNAL (error (const QString&)),
			this,
			SIGNAL (error (const QString&)));

	connect (DirectoryWatcher_.get (),
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
			this,
			SLOT (handleGotEntity (LeechCraft::DownloadEntity)));

	StorageBackend_->Prepare ();

	PluginManager_ = new PluginManager (this);

	ClipboardWatchdog_ = new QTimer (this);
	connect (ClipboardWatchdog_,
			SIGNAL (timeout ()),
			this,
			SLOT (handleClipboardTimer ()));
	ClipboardWatchdog_->start (2000);

	Server_->listen (Application::GetSocketName ());
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
	XmlSettingsManager::Instance ()->setProperty ("FirstStart", "false");
	DirectoryWatcher_.reset ();
	RequestNormalizer_.reset ();
	MergeModel_.reset ();

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
	ReallyMainWindow_->installEventFilter (this);
}

MainWindow* LeechCraft::Core::GetReallyMainWindow ()
{
	return ReallyMainWindow_;
}

const IShortcutProxy* LeechCraft::Core::GetShortcutProxy () const
{
	return ReallyMainWindow_->GetShortcutProxy ();
}

QObjectList LeechCraft::Core::GetSettables () const
{
	return PluginManager_->GetAllCastableRoots<IHaveSettings*> ();
}

QObjectList LeechCraft::Core::GetShortcuts () const
{
	return PluginManager_->GetAllCastableRoots<IHaveShortcuts*> ();
}

QList<QList<QAction*> > LeechCraft::Core::GetActions2Embed () const
{
	QList<IToolBarEmbedder*> plugins = PluginManager_->GetAllCastableTo<IToolBarEmbedder*> ();
	QList<QList<QAction*> > actions;
	Q_FOREACH (IToolBarEmbedder *plugin, plugins)
		actions << plugin->GetActions ();
	return actions;
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

StorageBackend* LeechCraft::Core::GetStorageBackend () const
{
	return StorageBackend_.get ();
}

QToolBar* LeechCraft::Core::GetToolBar (int index) const
{
	return TabContainer_->GetToolBar (index);
}

QToolBar* LeechCraft::Core::GetControls (const QModelIndex& index) const
{
	QVariant data = index.data (RoleControls);
	return data.value<QToolBar*> ();
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

	QStringList ids = (*modIter)->data ((*modIter)->
			index (index - starting, 0), RoleTags).toStringList ();
	QStringList result;
	Q_FOREACH (QString id, ids)
		result << TagsManager::Instance ().GetTag (id);
	return result;
}

void LeechCraft::Core::Setup (QObject *plugin)
{
	IJobHolder *ijh = qobject_cast<IJobHolder*> (plugin);
	IEmbedTab *iet = qobject_cast<IEmbedTab*> (plugin);
	IMultiTabs *imt = qobject_cast<IMultiTabs*> (plugin);

	InitDynamicSignals (plugin);

	if (ijh)
		InitJobHolder (plugin);

	if (iet)
		connect (plugin,
				SIGNAL (bringToFront ()),
				this,
				SLOT (embeddedTabWantsToFront ()));

	if (imt)
		InitMultiTab (plugin);
}

void LeechCraft::Core::DelayedInit ()
{
	connect (this,
			SIGNAL (error (QString)),
			ReallyMainWindow_,
			SLOT (catchError (QString)));

	TabContainer_.reset (new TabContainer (ReallyMainWindow_->GetTabWidget ()));

	PluginManager_->Init ();

	QObjectList plugins = PluginManager_->GetAllPlugins ();
	foreach (QObject *plugin, plugins)
	{
		IEmbedTab *iet = qobject_cast<IEmbedTab*> (plugin);
		if (iet)
			InitEmbedTab (plugin);
	}

	TabContainer_->handleTabNames ();

	QTimer::singleShot (1000,
			this,
			SLOT (pullCommandLine ()));
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
		try
		{
			if (di &&
					di->CouldDownload (e))
			{
				di->AddJob (e);
				return;
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to query/add task with"
				<< e.what ()
				<< "for"
				<< plugin;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to query/add task"
				<< plugin;
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
			{
				try
				{
					(*i)->ItemSelected (mapped);
				}
				catch (...)
				{
				}
			}
			else
			{
				try
				{
					(*i)->ItemSelected (QModelIndex ());
				}
				catch (...)
				{
				}
			}
	}
	else
		for (QList<IJobHolder*>::iterator i = holders.begin (),
				end = holders.end (); i != end; ++i)
		{
			try
			{
				(*i)->ItemSelected (QModelIndex ());
			}
			catch (...)
			{
			}
		}
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
			try
			{
				download += di->GetDownloadSpeed ();
				upload += di->GetUploadSpeed ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to get speeds"
					<< e.what ()
					<< plugin;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to get speeds"
					<< plugin;
			}
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

QModelIndex LeechCraft::Core::MapToSource (const QModelIndex& index) const
{
	return MapToSourceRecursively (index);
}

#define LC_APPENDER(a) a##_.Functors_.append (functor)
#define LC_GETTER(a) a##_.Functors_
#define LC_DEFINE_REGISTER(a) \
void LeechCraft::Core::RegisterHook (LeechCraft::HookSignature<a>::Signature_t functor) \
{ \
	LC_APPENDER(a); \
} \
template<> \
	LeechCraft::HookSignature<a>::Functors_t LeechCraft::Core::GetHooks<a> () const \
{ \
	return LC_GETTER(a); \
}
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER ((HIDDownloadFinishedNotification)
			(HIDNetworkAccessManagerCreateRequest));
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
#undef LC_GETTER
#undef LC_APPENDER


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
	else if (ReallyMainWindow_ &&
			watched == ReallyMainWindow_)
	{
		if (e->type () == QEvent::DragEnter)
		{
			QDragEnterEvent *event = static_cast<QDragEnterEvent*> (e);

			Q_FOREACH (QString format, event->mimeData ()->formats ())
			{
				DownloadEntity e = Util::MakeEntity (event->
							mimeData ()->data (format),
						QString (),
						LeechCraft::FromUserInitiated,
						format);

				if (CouldHandle (e))
				{
					event->acceptProposedAction ();
					break;
				}
			}

			return true;
		}
		else if (e->type () == QEvent::Drop)
		{
			QDropEvent *event = static_cast<QDropEvent*> (e);

			Q_FOREACH (QString format, event->mimeData ()->formats ())
			{
				DownloadEntity e = Util::MakeEntity (event->
							mimeData ()->data (format),
						QString (),
						LeechCraft::FromUserInitiated,
						format);

				if (handleGotEntity (e))
				{
					event->acceptProposedAction ();
					break;
				}
			}

			return true;
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

	QAbstractNetworkCache *cache = NetworkAccessManager_->cache ();
	if (cache)
		static_cast<QNetworkDiskCache*> (cache)->
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
		{
			try
			{
				QMetaObject::invokeMethod (object,
						function,
						Q_ARG (int, i->row ()));
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "invokation failed"
					<< e.what ()
					<< object
					<< function;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "invokation failed"
					<< object
					<< function;
			}
		}
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
		{
			try
			{
				QMetaObject::invokeMethod (object,
						slot.toLatin1 (),
						Q_ARG (std::deque<int>, selections));
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "slot invokation failed"
					<< e.what ()
					<< object
					<< slot;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "slot invokation failed"
					<< object
					<< slot;
			}
		}

		if (!signal.isEmpty ())
		{
			try
			{
				QMetaObject::invokeMethod (object,
						signal.toLatin1 (),
						Q_ARG (std::deque<int>, selections));
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "signal invokation failed"
					<< e.what ()
					<< object
					<< slot;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "signal invokation failed"
					<< object
					<< slot;
			}
		}
	}
	else
	{
		if (!slot.isEmpty ())
			PassSelectionsByOne (object, selected, slot.toLatin1 ());
		if (!signal.isEmpty ())
			PassSelectionsByOne (object, selected, signal.toLatin1 ());
	}
}

bool LeechCraft::Core::CouldHandle (const LeechCraft::DownloadEntity& e)
{
	QObjectList plugins = PluginManager_->GetAllCastableRoots<IDownload*> ();
	for (int i = 0; i < plugins.size (); ++i)
	{
		IDownload *id = qobject_cast<IDownload*> (plugins.at (i));
		try
		{
			if (id->CouldDownload (e))
				return true;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not query"
				<< e.what ()
				<< plugins.at (i);
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not query"
				<< plugins.at (i);
		}
	}
	plugins = PluginManager_->GetAllCastableRoots<IEntityHandler*> ();
	for (int i = 0; i < plugins.size (); ++i)
	{
		IEntityHandler *ih = qobject_cast<IEntityHandler*> (plugins.at (i));
		try
		{
			if (ih->CouldHandle (e))
				return true;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not query"
				<< e.what ()
				<< plugins.at (i);
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not query"
				<< plugins.at (i);
		}
	}
	return false;
}

namespace
{
	bool DoDownload (IDownload *sd,
			LeechCraft::DownloadEntity p,
			int *id,
			QObject **pr)
	{
		int l = -1;
		try
		{
			l = sd->AddJob (p);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not add job"
				<< e.what ();
			return false;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not add job";
			return false;
		}

		if (id)
			*id = l;
		if (pr)
		{
			QObjectList plugins = Core::Instance ().GetPluginManager ()->
				GetAllCastableRoots<IDownload*> ();
			*pr = *std::find_if (plugins.begin (), plugins.end (),
					boost::bind (std::equal_to<IDownload*> (),
						sd,
						boost::bind<IDownload*> (
							static_cast<IDownload* (*) (const QObject*)> (qobject_cast<IDownload*>),
							_1
							)));
		}

		return true;
	}

	bool DoHandle (IEntityHandler *sh,
			LeechCraft::DownloadEntity p)
	{
		try
		{
			sh->Handle (p);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not handle job"
				<< e.what ();
			return false;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "could not add job";
			return false;
		}
		return true;
	}
};

bool LeechCraft::Core::handleGotEntity (DownloadEntity p, int *id, QObject **pr)
{
	QString string = tr ("Too long to show");
	if (p.Entity_.canConvert<QByteArray> ())
	{
		QByteArray entity = p.Entity_.toByteArray ();
		if (entity.size () < 1000)
			string = QTextCodec::codecForName ("UTF-8")->toUnicode (entity);
	}
	else if (p.Entity_.canConvert<QUrl> ())
	{
		string = p.Entity_.toUrl ().toString ();
		if (string.size () > 150)
			string = string.left (147) + "...";
	}
	else
		string = tr ("Binary entity");

	std::auto_ptr<HandlerChoiceDialog> dia (new HandlerChoiceDialog (string));

	if (!(p.Parameters_ & LeechCraft::OnlyHandle))
	{
		QObjectList plugins = PluginManager_->GetAllCastableRoots<IDownload*> ();
		for (int i = 0; i < plugins.size (); ++i)
		{
			IDownload *id = qobject_cast<IDownload*> (plugins.at (i));
			IInfo *ii = qobject_cast<IInfo*> (plugins.at (i));
			try
			{
				if (id->CouldDownload (p))
					dia->Add (ii, id);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not query"
					<< e.what ()
					<< plugins.at (i);
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not query"
					<< plugins.at (i);
			}
		}
	}

	// Handlers don't fit when we want to delegate.
	if (!id && !(p.Parameters_ & LeechCraft::OnlyDownload))
	{
		QObjectList plugins = PluginManager_->GetAllCastableRoots<IEntityHandler*> ();
		for (int i = 0; i < plugins.size (); ++i)
		{
			IEntityHandler *ih = qobject_cast<IEntityHandler*> (plugins.at (i));
			IInfo *ii = qobject_cast<IInfo*> (plugins.at (i));
			try
			{
				if (ih->CouldHandle (p))
					dia->Add (ii, ih);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not query"
					<< e.what ()
					<< plugins.at (i);
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "could not query"
					<< plugins.at (i);
			}
		}
	}

	if (p.Parameters_ & FromUserInitiated &&
			!(p.Parameters_ & AutoAccept))
	{
		if (!dia->NumChoices () ||
				dia->exec () == QDialog::Rejected)
			return false;

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
					tr ("Select save location"),
					suggestion,
					QFileDialog::Options (~QFileDialog::ShowDirsOnly));

			if (dir.isEmpty ())
				return false;

			XmlSettingsManager::Instance ()->
				setProperty ("EntitySavePath", dir);

			p.Location_ = dir;

			if (!DoDownload (sd, p, id, pr))
			{
				if (dia->NumChoices () > 1 &&
						QMessageBox::question (0,
						tr ("Error"),
						tr ("Could not add task to the selected downloader, "
							"would you like to try another one?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
					return handleGotEntity (p, id, pr);
				else
					return false;
			}
			else
				return true;
		}
		if (sh)
		{
			if (!DoHandle (sh, p))
			{
				if (dia->NumChoices () > 1 &&
						QMessageBox::question (0,
						tr ("Error"),
						tr ("Could not handle task with the selected handler, "
							"would you like to try another one?"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
					return handleGotEntity (p, id, pr);
				else
					return false;
			}
			else
				return true;
		}
	}
	else if (dia->GetDownload ())
	{
		IDownload *sd = dia->GetDownload ();
		if (p.Location_.isEmpty ())
			p.Location_ = QDir::tempPath ();
		return DoDownload (sd, p, id, pr);
	}
	else if ((p.Parameters_ & LeechCraft::AutoAccept) &&
			dia->GetFirstEntityHandler ())
		return DoHandle (dia->GetFirstEntityHandler (), p);
	else
	{
		emit log (tr ("Could not handle download entity %1.")
				.arg (string));
		return false;
	}
	return true;
}

void LeechCraft::Core::handleCouldHandle (const LeechCraft::DownloadEntity& e, bool *could)
{
	*could = CouldHandle (e);
}

void LeechCraft::Core::handleClipboardTimer ()
{
	QString text = QApplication::clipboard ()->text ();
	if (text.isEmpty () || text == PreviousClipboardContents_)
		return;

	PreviousClipboardContents_ = text;

	DownloadEntity e = Util::MakeEntity (text.toUtf8 (),
			QString (),
			LeechCraft::FromUserInitiated);

	if (XmlSettingsManager::Instance ()->property ("WatchClipboard").toBool ())
		handleGotEntity (e);
}

void LeechCraft::Core::embeddedTabWantsToFront ()
{
	IEmbedTab *iet = qobject_cast<IEmbedTab*> (sender ());
	if (!iet)
		return;

	try
	{
		TabContainer_->bringToFront (iet->GetTabContents ());
		ReallyMainWindow_->show ();
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< sender ();
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< sender ();
	}
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
	try
	{
		emit log (ii->GetName () + ": " + message);
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< sender ();
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< sender ();
	}
}

void LeechCraft::Core::pullCommandLine ()
{
	QStringList arguments = qobject_cast<Application*> (qApp)->Arguments ();
	DoCommandLine (arguments);
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

	DoCommandLine (arguments);
}

void LeechCraft::Core::DoCommandLine (const QStringList& arguments)
{
	if (!(arguments.size () > 1 &&
			!arguments.last ().startsWith ('-')))
		return;

	TaskParameters tp;
	if (!arguments.contains ("-automatic"))
		tp |= FromUserInitiated;
	else
		tp |= AutoAccept;
	if (arguments.contains ("-handle"))
	{
		tp |= OnlyHandle;
		tp |= AutoAccept;
	}
	if (arguments.contains ("-download"))
	{
		tp |= OnlyDownload;
		tp |= AutoAccept;
	}

	DownloadEntity e = Util::MakeEntity (arguments.last ().toUtf8 (),
			QString (),
			tp);
	handleGotEntity (e);
}

void LeechCraft::Core::InitDynamicSignals (QObject *plugin)
{
	const QMetaObject *qmo = plugin->metaObject ();

	if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
					"couldHandle (const LeechCraft::DownloadEntity&, bool*)"
					).constData ()) != -1)
		connect (plugin,
				SIGNAL (couldHandle (const LeechCraft::DownloadEntity&, bool*)),
				this,
				SLOT (handleCouldHandle (const LeechCraft::DownloadEntity&, bool*)));

	if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
					"gotEntity (const LeechCraft::DownloadEntity&)"
					).constData ()) != -1)
		connect (plugin,
				SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
				this,
				SLOT (handleGotEntity (LeechCraft::DownloadEntity)));

	if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
					"delegateEntity (const LeechCraft::DownloadEntity&, int*, QObject**)"
					).constData ()) != -1)
		connect (plugin,
				SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**)),
				this,
				SLOT (handleGotEntity (LeechCraft::DownloadEntity,
						int*, QObject**)));

	if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
					"downloadFinished (const QString&)"
					).constData ()) != -1)
		connect (plugin,
				SIGNAL (downloadFinished (const QString&)),
				this,
				SIGNAL (downloadFinished (const QString&)));

	if (qmo->indexOfSignal (QMetaObject::normalizedSignature (
					"log (const QString&)"
					).constData ()) != -1)
		connect (plugin,
				SIGNAL (log (const QString&)),
				this,
				SLOT (handleLog (const QString&)));
}

void LeechCraft::Core::InitJobHolder (QObject *plugin)
{
	try
	{
		IJobHolder *ijh = qobject_cast<IJobHolder*> (plugin);
		QAbstractItemModel *model = ijh->GetRepresentation ();
		Representation2Object_ [model] = plugin;
		MergeModel_->AddModel (model);

		if (model)
		{
			QToolBar *controlsWidget = model->
				index (0, 0).data (RoleControls).value<QToolBar*> ();
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

			QWidget *additional = model->
				index (0, 0).data (RoleAdditionalInfo).value<QWidget*> ();
			if (additional)
				additional->setParent (ReallyMainWindow_);
		}
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< plugin;
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< plugin;
	}
}

void LeechCraft::Core::InitEmbedTab (QObject *plugin)
{
	try
	{
		IInfo *ii = qobject_cast<IInfo*> (plugin);
		IEmbedTab *iet = qobject_cast<IEmbedTab*> (plugin);
		TabContainer_->add (ii->GetName (),
				iet->GetTabContents (),
				ii->GetIcon ());
		TabContainer_->SetToolBar (iet->GetToolBar (),
				iet->GetTabContents ());
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< plugin;
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< plugin;
	}
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

