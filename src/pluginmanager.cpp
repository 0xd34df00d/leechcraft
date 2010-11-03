/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <stdexcept>
#include <algorithm>
#include <QApplication>
#include <QDir>
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <plugininterface/util.h>
#include <plugininterface/exceptions.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ipluginadaptor.h>
#include <interfaces/ihaveshortcuts.h>
#include "core.h"
#include "pluginmanager.h"
#include "mainwindow.h"
#include "xmlsettingsmanager.h"
#include "coreproxy.h"
#include "config.h"

using namespace LeechCraft;

namespace LeechCraft
{
	PluginManager::DepTreeItem::DepTreeItem ()
	: Plugin_ (0)
	, Initialized_ (false)
	{
	}

	void PluginManager::DepTreeItem::Print (int margin)
	{
		QString pre (margin, ' ');
		qDebug () << pre << Plugin_ << Initialized_;
		qDebug () << pre << "Needed:";
		QList<DepTreeItem_ptr> items = Needed_.values ();
		Q_FOREACH (DepTreeItem_ptr item, items)
			item->Print (margin + 2);
		qDebug () << pre << "Used:";
		items = Used_.values ();
		Q_FOREACH (DepTreeItem_ptr item, items)
			item->Print (margin + 2);
	}

	PluginManager::Finder::Finder (QObject *o)
	: Object_ (o)
	{
	}

	bool PluginManager::Finder::operator() (PluginManager::DepTreeItem_ptr item) const
	{
		return Object_ == item->Plugin_;
	}

	PluginManager::PluginManager (const QStringList& pluginPaths, QObject *parent)
	: QAbstractItemModel (parent)
	, DefaultPluginIcon_ (QIcon (":/resources/images/defaultpluginicon.svg"))
	{
		Headers_ << tr ("Name")
			<< tr ("Description");

		if (pluginPaths.isEmpty ())
			FindPlugins ();
		else
		{
			qDebug () << Q_FUNC_INFO << "explicit paths given, entering forced loading mode";
			Q_FOREACH (const QString& path, pluginPaths)
			{
				qDebug () << "adding" << path;
				QPluginLoader_ptr loader (new QPluginLoader (path));
				PluginContainers_.push_back (loader);
			}
		}
	}

	PluginManager::~PluginManager ()
	{
	}

	int PluginManager::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant PluginManager::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid () ||
				index.row () >= GetSize ())
			return QVariant ();

		switch (index.column ())
		{
			case 0:
				switch (role)
				{
					case Qt::DisplayRole:
						{
							QSettings settings (QCoreApplication::organizationName (),
									QCoreApplication::applicationName () + "-pg");
							settings.beginGroup ("Plugins");
							settings.beginGroup (AvailablePlugins_.at (index.row ())->fileName ());
							QVariant result = settings.value ("Name");
							settings.endGroup ();
							settings.endGroup ();
							return result;
						}
					case Qt::DecorationRole:
						{
							QSettings settings (QCoreApplication::organizationName (),
									QCoreApplication::applicationName () + "-pg");
							settings.beginGroup ("Plugins");
							settings.beginGroup (AvailablePlugins_.at (index.row ())->fileName ());
							QVariant result = settings.value ("Icon");
							settings.endGroup ();
							settings.endGroup ();
							if (result.value<QIcon> ().isNull () &&
									result.value<QPixmap> ().isNull ())
								result = DefaultPluginIcon_;
							return result;
						}
					case Qt::CheckStateRole:
						{
							QSettings settings (QCoreApplication::organizationName (),
									QCoreApplication::applicationName () + "-pg");
							settings.beginGroup ("Plugins");
							settings.beginGroup (AvailablePlugins_.at (index.row ())->fileName ());
							bool result = settings.value ("AllowLoad", true).toBool ();
							settings.endGroup ();
							settings.endGroup ();
							return result ? Qt::Checked : Qt::Unchecked;
						}
					case Qt::ForegroundRole:
						return QApplication::palette ()
							.brush (AvailablePlugins_.at (index.row ())->isLoaded () ?
									QPalette::Normal :
									QPalette::Disabled,
								QPalette::WindowText);
					default:
						return QVariant ();
				}
			case 1:
				if (role == Qt::DisplayRole)
				{
					QSettings settings (QCoreApplication::organizationName (),
							QCoreApplication::applicationName () + "-pg");
					settings.beginGroup ("Plugins");
					settings.beginGroup (AvailablePlugins_.at (index.row ())->fileName ());
					QVariant result = settings.value ("Info");
					settings.endGroup ();
					settings.endGroup ();
					return result;
				}
				else if (role == Qt::ForegroundRole)
					return QApplication::palette ()
						.brush (AvailablePlugins_.at (index.row ())->isLoaded () ?
								QPalette::Normal :
								QPalette::Disabled,
							QPalette::WindowText);
				else
					return QVariant ();
			default:
				return QVariant ();
		}
	}

	Qt::ItemFlags PluginManager::flags (const QModelIndex& index) const
	{
		Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		if (index.column () == 0)
			result |= Qt::ItemIsUserCheckable;
		return result;
	}

	QVariant PluginManager::headerData (int column, Qt::Orientation orient, int role) const
	{
		if (role != Qt::DisplayRole ||
				orient != Qt::Horizontal)
			return QVariant ();

		return Headers_.at (column);
	}

	QModelIndex PluginManager::index (int row, int column, const QModelIndex&) const
	{
		return createIndex (row, column);
	}

	QModelIndex PluginManager::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int PluginManager::rowCount (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return AvailablePlugins_.size ();
		else
			return 0;
	}

	bool PluginManager::setData (const QModelIndex& index,
			const QVariant& data, int role)
	{
		if (index.column () != 0 ||
				role != Qt::CheckStateRole)
			return false;

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");
		settings.beginGroup (AvailablePlugins_.at (index.row ())->fileName ());
		settings.setValue ("AllowLoad", data.toBool ());
		settings.endGroup ();
		settings.endGroup ();

		emit dataChanged (index, index);

		return true;
	}

	PluginManager::Size_t PluginManager::GetSize () const
	{
		return AvailablePlugins_.size ();
	}

	void PluginManager::Init ()
	{
		CheckPlugins ();
		Q_FOREACH (QPluginLoader_ptr loader, PluginContainers_)
		{
			QObject *inst = loader->instance ();
			Plugins_ << inst;
			IPluginAdaptor *ipa = qobject_cast<IPluginAdaptor*> (inst);
			if (ipa)
				Plugins_ << ipa->GetPlugins ();
		}
		CalculateDependencies ();
		InitializePlugins ();
	}

	void PluginManager::Release ()
	{
		while (Roots_.size ())
		{
			try
			{
				Release (Roots_.takeAt (0));
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO << e.what ();
			}
			catch (...)
			{
				QMessageBox::warning (Core::Instance ().GetReallyMainWindow (),
						"LeechCraft",
						tr ("Release of one or more plugins failed."));
			}
		}

		Plugins_.clear ();
	}

	QString PluginManager::Name (const PluginManager::Size_t& pos) const
	{
		return (qobject_cast<IInfo*> (Plugins_ [pos]))->GetName ();
	}

	QString PluginManager::Info (const PluginManager::Size_t& pos) const
	{
		return qobject_cast<IInfo*> (Plugins_ [pos])->GetInfo ();
	}

	QObjectList PluginManager::GetAllPlugins () const
	{
		struct RecursiveCollector
		{
			QObjectList Result_;
			void operator() (DepTreeItem_ptr item)
			{
				if (!Result_.contains (item->Plugin_))
					Result_ << item->Plugin_;

				Q_FOREACH (DepTreeItem_ptr child,
						item->Needed_ + item->Used_)
					(*this) (child);
			}
		} rc;
		rc.Result_ = Plugins_;
		Q_FOREACH (DepTreeItem_ptr item, Roots_)
			rc (item);
		return rc.Result_;
	}

	QString PluginManager::GetPluginLibraryPath (const QObject *object) const
	{
		Q_FOREACH (QPluginLoader_ptr loader, PluginContainers_)
			if (loader->instance () == object)
				return loader->fileName ();
		return QString ();
	}

	QObject* PluginManager::GetPluginByID (const QByteArray& id) const
	{
		if (!PluginID2PluginCache_.contains (id))
			Q_FOREACH (QObject *plugin, GetAllPlugins ())
				if (qobject_cast<IInfo*> (plugin)->GetUniqueID () == id)
				{
					PluginID2PluginCache_ [id] = plugin;
					break;
				}

		return PluginID2PluginCache_ [id];
	}

	void PluginManager::InjectPlugin (QObject *object)
	{
		DepTreeItem_ptr depItem (new DepTreeItem ());
		depItem->Plugin_ = object;
		try
		{
			qobject_cast<IInfo*> (object)->Init (ICoreProxy_ptr (new CoreProxy ()));
			Core::Instance ().Setup (object);

			qobject_cast<IInfo*> (object)->SecondInit ();
			Core::Instance ().PostSecondInit (object);

			IPlugin2 *ip2 = qobject_cast<IPlugin2*> (object);
			if (ip2)
			{
				QSet<QByteArray> classes = ip2->GetPluginClasses ();
				Q_FOREACH (IPluginReady *ipr, GetAllCastableTo<IPluginReady*> ())
					if (!ipr->GetExpectedPluginClasses ()
							.intersect (classes).isEmpty ())
						ipr->AddPlugin (object);
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to init the object with"
				<< e.what ()
				<< "for"
				<< object;
			throw;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to init"
				<< object;
			throw;
		}

		emit pluginInjected (object);

		depItem->Initialized_ = true;
		Roots_ << depItem;
	}

	void PluginManager::ReleasePlugin (QObject *object)
	{
		DepTreeItem_ptr depItem = GetDependency (object);
		if (!depItem->Initialized_)
		{
			QString str;
			QDebug debug (&str);
			debug << "dep item for"
					<< object
					<< "isn't initialized";
			qWarning () << Q_FUNC_INFO
					<< str;
			throw DependencyException (str);
		}

		if (depItem->Belongs_.size ())
		{
			QList<QObject*> holders;
			Q_FOREACH (DepTreeItem_ptr dep, depItem->Belongs_)
				holders << dep->Plugin_;

			QString str;
			QDebug debug (&str);
			debug << object
					<< "cannot be released because of"
					<< holders;
			qWarning () << Q_FUNC_INFO
					<< str;
			throw ReleaseFailureException (str, holders);
		}

		try
		{
			qobject_cast<IInfo*> (object)->Release ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to release the unloading object with"
				<< e.what ()
				<< "for"
				<< object;
			throw;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to release the unloading object"
				<< object;
		}
	}

	QObject* PluginManager::GetObject ()
	{
		return this;
	}

	QObject* PluginManager::GetProvider (const QString& feature) const
	{
		if (!FeatureProviders_.contains (feature))
			return 0;
		return (*FeatureProviders_ [feature])->instance ();
	}

	void PluginManager::Unload (QObject *plugin)
	{
		Unload (Find (plugin));
	}

	const QStringList& PluginManager::GetPluginLoadErrors () const
	{
		return PluginLoadErrors_;
	}

	void PluginManager::FindPlugins ()
	{
#ifdef Q_WS_WIN
		ScanDir (QApplication::applicationDirPath () + "/plugins/bin");
#elif defined (Q_WS_MAC)
		ScanDir (QApplication::applicationDirPath () + "../PlugIns/bin");
#else
		QString libdir (PLUGINS_LIBDIR);
		ScanDir (QString ("/usr/local/%1/leechcraft/plugins")
				.arg (libdir));
		ScanDir (QString ("/usr/%1/leechcraft/plugins")
				.arg (libdir));
#endif
	}

	void PluginManager::ScanDir (const QString& dir)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");

		QDir pluginsDir = QDir (dir);
		Q_FOREACH (QFileInfo fileinfo,
				pluginsDir.entryInfoList (QStringList ("*leechcraft_*"),
					QDir::Files))
		{
			QString name = fileinfo.canonicalFilePath ();
			settings.beginGroup (name);

			QPluginLoader_ptr loader (new QPluginLoader (name));
			if (settings.value ("AllowLoad", true).toBool ())
				PluginContainers_.push_back (loader);

			loader->setLoadHints (QLibrary::ExportExternalSymbolsHint);

			AvailablePlugins_.push_back (loader);

			settings.endGroup ();
		}

		settings.endGroup ();
	}

	void PluginManager::CheckPlugins ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "-pg");
		settings.beginGroup ("Plugins");

		for (int i = 0; i < PluginContainers_.size (); ++i)
		{
			QPluginLoader_ptr loader = PluginContainers_.at (i);

			QString file = loader->fileName ();

			if (!QFileInfo (loader->fileName ()).isFile ())
			{
				qWarning () << "A plugin isn't really a file, aborting load:"
						<< file;
				PluginLoadErrors_ << tr ("Refusing to load plugin from %1 because it's not a file.")
						.arg (QFileInfo (file).fileName ());
				PluginContainers_.removeAt (i--);
				continue;
			}

			loader->load ();
			if (!loader->isLoaded ())
			{
				qWarning () << "Could not load library:"
					<< file
					<< ";"
					<< loader->errorString ();
				PluginLoadErrors_ << tr ("Could not load plugin from %1: %2.")
						.arg (QFileInfo (file).fileName ())
						.arg (loader->errorString ());
				PluginContainers_.removeAt (i--);
				continue;
			}

			QObject *pluginEntity;
			try
			{
				pluginEntity = loader->instance ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to construct the instance with"
					<< e.what ()
					<< "for"
					<< file;
				PluginLoadErrors_ << tr ("Could not load plugin from %1: "
							"failed to construct plugin instance with exception %2.")
						.arg (QFileInfo (file).fileName ())
						.arg (e.what ());
				PluginContainers_.removeAt (i--);
				continue;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to construct the instance for"
					<< file;
				PluginLoadErrors_ << tr ("Could not load plugin from %1: "
							"failed to construct plugin instance.")
						.arg (QFileInfo (file).fileName ());
				PluginContainers_.removeAt (i--);
				continue;
			}

			IInfo *info = qobject_cast<IInfo*> (pluginEntity);
			if (!info)
			{
				qWarning () << "Casting to IInfo failed:"
						<< file;
				PluginLoadErrors_ << tr ("Could not load plugin from %1: "
							"unable to cast plugin instance to IInfo*.")
						.arg (QFileInfo (file).fileName ());
				PluginContainers_.removeAt (i--);
				continue;
			}

			QString name;
			QString pinfo;
			QIcon icon;
			try
			{
				name = info->GetName ();
				pinfo = info->GetInfo ();
				icon = info->GetIcon ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get name/icon"
					<< e.what ()
					<< "for"
					<< file;
				PluginLoadErrors_ << tr ("Could not load plugin from %1: "
							"unable to get name/info/icon with exception %2.")
						.arg (QFileInfo (file).fileName ())
						.arg (e.what ());
				PluginContainers_.removeAt (i--);
				continue;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get name/icon"
					<< file;
				PluginLoadErrors_ << tr ("Could not load plugin from %1: "
							"unable to get name/info/icon.")
						.arg (QFileInfo (file).fileName ());
				PluginContainers_.removeAt (i--);
				continue;
			}
			settings.beginGroup (file);
			settings.setValue ("Name", name);
			settings.setValue ("Info", pinfo);
			settings.setValue ("Icon", icon.pixmap (48, 48));
			settings.endGroup ();
		}

		settings.endGroup ();
	}

	QList<PluginManager::Plugins_t::iterator>
		PluginManager::FindProviders (const QString& feature)
	{
		QList<Plugins_t::iterator> result;
		for (Plugins_t::iterator i = Plugins_.begin ();
				i != Plugins_.end (); ++i)
		{
			try
			{
				if (qobject_cast<IInfo*> (*i)->
						Provides ().contains (feature))
					result << i;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get providers with"
					<< e.what ()
					<< "for"
					<< (*Find (*i))->fileName ();
				Unload (Find (*i));
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get providers"
					<< (*Find (*i))->fileName ();
				Unload (Find (*i));
			}
		}
		return result;
	}

	QList<PluginManager::Plugins_t::iterator>
		PluginManager::FindProviders (const QSet<QByteArray>& expecteds)
	{
		QList<Plugins_t::iterator> result;
		for (Plugins_t::iterator i = Plugins_.begin ();
				i != Plugins_.end (); ++i)
		{
			IPlugin2 *ip2 = qobject_cast<IPlugin2*> (*i);
			try
			{
				if (ip2)
				{
					QSet<QByteArray> pcs = ip2->GetPluginClasses ();
					qDebug () << qobject_cast<IInfo*> (*i)->GetName () << pcs << expecteds;
					if (!pcs.intersect (expecteds).isEmpty ())
						result << i;
				}
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get class with"
					<< e.what ()
					<< "for"
					<< (*Find (*i))->fileName ();
				Unload (Find (*i));
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get class"
					<< (*Find (*i))->fileName ();
				Unload (Find (*i));
			}
		}
		return result;
	}

	PluginManager::DepTreeItem_ptr
		PluginManager::GetDependency
			(QObject *entity)
	{
		struct Finder
		{
			QObject *Plugin_;
			DepTreeItem_ptr Result_;

			Finder (QObject *plugin)
			: Plugin_ (plugin)
			{
			}

			bool operator() (DepTreeItem_ptr item)
			{
				QList<DepTreeItem_ptr> deps =
					item->Needed_.values () + item->Used_.values ();
				std::sort (deps.begin (), deps.end ());
				QList<DepTreeItem_ptr>::const_iterator last =
					std::unique (deps.begin (), deps.end ());

				for (QList<DepTreeItem_ptr>::const_iterator i = deps.begin ();
						i != last; ++i)
					if ((*i)->Plugin_ == Plugin_)
					{
						Result_ = *i;
						return true;
					}
				for (QList<DepTreeItem_ptr>::const_iterator i = deps.begin ();
						i != last; ++i)
					if (operator() (*i))
						return true;

				return false;
			}
		};

		Finder finder (entity);
#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO << Roots_;
#endif
		Q_FOREACH (DepTreeItem_ptr dep, Roots_)
		{
#ifdef QT_DEBUG
			qDebug () << dep->Plugin_;
#endif
			if (dep->Plugin_ == entity)
			{
#ifdef QT_DEBUG
				qDebug () << "REMOVING for" << entity;
#endif
				Roots_.removeAll (dep);
				return dep;
			}
			else
			{
				if (finder (dep))
					return finder.Result_;
			}
		}

#ifdef QT_DEBUG
		qDebug () << "not found";
#endif

		return DepTreeItem_ptr ();
	}

	void PluginManager::CalculateDependencies ()
	{
		for (Plugins_t::iterator i = Plugins_.begin ();
				i < Plugins_.end (); ++i)
		{
#ifdef QT_DEBUG
			qDebug () << Q_FUNC_INFO << (*i);
#endif
			if (!GetDependency (*i))
			{
#ifdef QT_DEBUG
				qDebug () << Q_FUNC_INFO << "would CalculateSingle";
#endif
				try
				{
					Roots_ << CalculateSingle (i);
				}
				catch (...)
				{
					qWarning () << Q_FUNC_INFO
						<< "CalculateSingle failed";
				}
#ifdef QT_DEBUG
				DumpTree ();
#endif
			}
		}
#ifdef QT_DEBUG
		qDebug () << "after calculating dependencies";
		DumpTree ();
#endif
	}

	PluginManager::DepTreeItem_ptr
		PluginManager::CalculateSingle
			(PluginManager::Plugins_t::iterator i)
	{
		QObject *entity = *i;
#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO << "calculating for" << entity;
#endif
		try
		{
			return CalculateSingle (entity, i);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to get required stuff with"
				<< e.what ()
				<< "for"
				<< (*Find (*i))->fileName ();
			Unload (Find (*i));
			throw;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to get required stuff"
				<< (*Find (*i))->fileName ();
			Unload (Find (*i));
			throw;
		}
	}

	PluginManager::DepTreeItem_ptr
		PluginManager::CalculateSingle (QObject *entity,
				PluginManager::Plugins_t::iterator pos)
	{
		DepTreeItem_ptr possibly = GetDependency (entity);
		if (possibly)
			return possibly;

		IInfo *info = qobject_cast<IInfo*> (entity);
		QStringList needs = info->Needs ();
		QStringList uses = info->Uses ();

#ifdef QT_DEBUG
		qDebug () << "new item" << info->GetName ();
#endif
		DepTreeItem_ptr newDep (new DepTreeItem ());
		newDep->Plugin_ = entity;

		Q_FOREACH (QString need, needs)
		{
			QList<Plugins_t::iterator> providers = FindProviders (need);
			Q_FOREACH (Plugins_t::iterator p,
					providers)
			{
				// It's initialized already.
				if (p < pos)
				{
					DepTreeItem_ptr depprov = GetDependency (*p);
					// TODO register entity in depprov->Belongs_
					if (depprov)
						newDep->Needed_.insert (need, depprov);
				}
				else if (p > pos)
					newDep->Needed_.insert (need, CalculateSingle (p));
			}
		}

		Q_FOREACH (QString use, uses)
		{
			QList<Plugins_t::iterator> providers = FindProviders (use);
			Q_FOREACH (Plugins_t::iterator p,
					providers)
			{
				// It's initialized already.
				if (p < pos)
				{
					DepTreeItem_ptr depprov = GetDependency (*p);
					if (depprov)
						newDep->Used_.insert (use, depprov);
				}
				else if (p > pos)
					newDep->Used_.insert (use, CalculateSingle (p));
			}
		}

		IPluginReady *ipr = qobject_cast<IPluginReady*> (entity);
		if (ipr)
		{
			QList<Plugins_t::iterator> providers =
				FindProviders (ipr->GetExpectedPluginClasses ());
			Q_FOREACH (Plugins_t::iterator p,
					providers)
			{
				// It's initialized already.
				if (p < pos)
				{
					DepTreeItem_ptr depprov = GetDependency (*p);
					if (depprov)
						newDep->Used_.insert ("__lc_plugin2", depprov);
				}
				else if (p > pos)
					newDep->Used_.insert ("__lc_plugin2", CalculateSingle (p));
			}
		}

		return newDep;
	}

	void PluginManager::InitializePlugins ()
	{
		Q_FOREACH (DepTreeItem_ptr item, Roots_)
			InitializeSingle (item);

		struct InitSecond
		{
			PluginManager *That_;

			InitSecond (PluginManager *that)
			: That_ (that)
			{
			}

			bool operator() (DepTreeItem_ptr item) const
			{
				if (!item ||
						!item->Initialized_)
					return false;;

				try
				{
					qobject_cast<IInfo*> (item->Plugin_)->SecondInit ();
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << 2 << e.what ();
					That_->Unload (That_->Find (item));
					return false;
				}
				catch (...)
				{
					qWarning () << Q_FUNC_INFO << 2;
					That_->Unload (That_->Find (item));
					return false;
				}

				return true;
			}
		} init (this);


		for (Plugins_t::iterator i = Plugins_.begin ();
				i < Plugins_.end (); ++i)
		{
			if (!init (FindTreeItem (*i)))
				continue;

			/* _PLUGINS
			IPluginAdaptor *ipa = qobject_cast<IPluginAdaptor*> (*i);
			if (ipa)
				Q_FOREACH (QObject *plugin, ipa->GetPlugins ())
					init (FindTreeItem (plugin));
			*/
		}

		Q_FOREACH (QObject *plugin, GetAllPlugins ())
			Core::Instance ().PostSecondInit (plugin);
	}

	bool PluginManager::InitializeSingle (PluginManager::DepTreeItem_ptr item)
	{
		QList<QString> keys = item->Needed_.uniqueKeys ();
		Q_FOREACH (QString key, keys)
		{
			QList<DepTreeItem_ptr> providers = item->Needed_.values (key);
			bool wasSuccessful = false;
			for (QList<DepTreeItem_ptr>::const_iterator i = providers.begin (),
					end = providers.end (); i != end; ++i)
			{
				if (!(*i)->Initialized_)
					InitializeSingle (*i);

				if (!(*i)->Initialized_)
					item->Needed_.remove (key, *i);
				else
				{
					wasSuccessful = true;

					qobject_cast<IInfo*> (item->Plugin_)->
						SetProvider ((*i)->Plugin_, key);;
				}
			}

			if (!wasSuccessful)
				return false;
		}

		keys = item->Used_.uniqueKeys ();
		Q_FOREACH (QString key, keys)
		{
			QList<DepTreeItem_ptr> providers = item->Used_.values (key);
			for (QList<DepTreeItem_ptr>::const_iterator i = providers.begin (),
					end = providers.end (); i != end; ++i)
			{
				if (!(*i)->Initialized_)
					InitializeSingle (*i);

				if (!(*i)->Initialized_)
					item->Used_.remove (key, *i);
				else
				{
					if (key == "__lc_plugin2")
						qobject_cast<IPluginReady*> (item->Plugin_)->
							AddPlugin ((*i)->Plugin_);
					else
						qobject_cast<IInfo*> (item->Plugin_)->
							SetProvider ((*i)->Plugin_, key);
				}
			}
		}

		try
		{
			IInfo *ii = qobject_cast<IInfo*> (item->Plugin_);
			QString name = ii->GetName ();
			qDebug () << "Initializing" << name << "...";
			ii->Init (ICoreProxy_ptr (new CoreProxy ()));
			item->Initialized_ = true;
			Core::Instance ().Setup (item->Plugin_);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << 2 << e.what ();
			Unload (Find (item));
			return false;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO << 2;
			Unload (Find (item));
			return false;
		}
		return true;
	}

	void PluginManager::Release (DepTreeItem_ptr item)
	{
		QList<DepTreeItem_ptr> deps =
			item->Needed_.values () + item->Used_.values ();
		std::sort (deps.begin (), deps.end ());
		QList<DepTreeItem_ptr>::const_iterator last =
			std::unique (deps.begin (), deps.end ());

		if (item->Initialized_)
		{
			IInfo *ii = qobject_cast<IInfo*> (item->Plugin_);
#ifdef QT_DEBUG
			try
			{
				qDebug () << Q_FUNC_INFO << ii->GetName ();
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get the name of the unloading object";
			}
#endif

			PluginsContainer_t::iterator i = Find (item);
			try
			{
				ii->Release ();
				item->Initialized_ = false;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to release the unloading object with"
					<< e.what ()
					<< "for"
					<< (*i)->fileName ();
			}
			catch (...)
			{
				PluginsContainer_t::iterator i = Find (item);
				qWarning () << Q_FUNC_INFO
					<< "failed to release the unloading object"
					<< (*i)->fileName ();
			}
			Unload (i);
		}

		for (QList<DepTreeItem_ptr>::const_iterator i = deps.begin ();
				i != last; ++i)
			Release (*i);
	}

	void PluginManager::DumpTree ()
	{
		Q_FOREACH (DepTreeItem_ptr item, Roots_)
			item->Print ();
	}

	PluginManager::DepTreeItem_ptr PluginManager::FindTreeItem (QObject *inst)
	{
		struct Descender
		{
			const QObject * const Inst_;

			Descender (const QObject * const inst)
			: Inst_ (inst)
			{
			}

			DepTreeItem_ptr operator() (DepTreeItem_ptr first) const
			{
				QList<DepTreeItem_ptr> list;
				list << first;
				while (!list.isEmpty ())
				{
					DepTreeItem_ptr item = list.takeFirst ();
					if (item->Plugin_ == Inst_)
						return item;
					else
					{
						list << item->Needed_.values ();
						list << item->Used_.values ();
					}
				}

				return DepTreeItem_ptr ();
			}
		} desc (inst);

		Q_FOREACH (DepTreeItem_ptr root, Roots_)
		{
			DepTreeItem_ptr item = desc (root);
			if (item)
				return item;
		}

		return DepTreeItem_ptr ();
	}

	namespace
	{
		struct LoaderFinder
		{
			QObject *Object_;

			LoaderFinder (QObject *o)
			: Object_ (o)
			{
			}

			bool operator() (const boost::shared_ptr<QPluginLoader>& ptr) const
			{
				return ptr->instance () == Object_;
			}
		};
	};

	PluginManager::PluginsContainer_t::iterator
		PluginManager::Find (DepTreeItem_ptr item)
	{
		return Find (item->Plugin_);
	}

	PluginManager::PluginsContainer_t::iterator
		PluginManager::Find (QObject *item)
	{
		return std::find_if (PluginContainers_.begin (), PluginContainers_.end (),
				LoaderFinder (item));
	}

	void PluginManager::Unload (PluginsContainer_t::iterator i)
	{
		if (i == PluginContainers_.end ())
			return;

		if (!UnloadQueue_.contains (i))
			UnloadQueue_ << i;

		DepTreeItem_ptr dep = GetDependency ((*i)->instance ());
		if (dep)
		{
			QPluginLoader_ptr pluginLoader = *i;
			Q_FOREACH (DepTreeItem_ptr belongs, dep->Belongs_)
				Unload (Find (belongs));

			if (dep->Initialized_)
			{
				try
				{
					qobject_cast<IInfo*> (dep->Plugin_)->Release ();
				}
				catch (const std::exception& e)
				{
					PluginsContainer_t::iterator i = Find (dep);
					qWarning () << Q_FUNC_INFO
						<< "failed to release the unloading object with"
						<< e.what ()
						<< "for"
						<< pluginLoader->fileName ();
				}
				catch (...)
				{
					PluginsContainer_t::iterator i = Find (dep);
					qWarning () << Q_FUNC_INFO
						<< "failed to release the unloading object"
						<< pluginLoader->fileName ();
				}
			}
		}

		/** TODO understand why app segfaults on exit if it's uncommented.
		if (UnloadQueue_.size () == 1)
			processUnloadQueue ();
		*/
	}

	void PluginManager::processUnloadQueue ()
	{
		for (int i = 0; i < UnloadQueue_.size (); ++i)
		{
			QPluginLoader_ptr loader = *UnloadQueue_.at (i);
			IInfo *ii = qobject_cast<IInfo*> (loader->instance ());
			QString name;
			try
			{
				name = ii->GetName ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to get name for the unload"
					<< loader->instance ()
					<< e.what ();
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to get name for the unload"
					<< loader->instance ();
			}

			try
			{
#ifdef QT_DEBUG
				qDebug () << Q_FUNC_INFO
					<< name;
#endif
				if (!loader->unload ())
					qWarning () << Q_FUNC_INFO
						<< "unable to unload"
						<< loader->instance ()
						<< loader->errorString ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to unload with exception"
					<< loader->instance ()
					<< e.what ();
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to unload with exception"
					<< loader->instance ();
			}
		}
		std::sort (UnloadQueue_.begin (), UnloadQueue_.end ());
		while (UnloadQueue_.size ())
		{
			PluginContainers_.erase (UnloadQueue_.last ());
			UnloadQueue_.pop_back ();
		}
	}
};

