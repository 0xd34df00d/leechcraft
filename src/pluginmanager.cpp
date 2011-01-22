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
#include "plugintreebuilder.h"
#include "config.h"

namespace LeechCraft
{
	PluginManager::PluginManager (const QStringList& pluginPaths, QObject *parent)
	: QAbstractItemModel (parent)
	, DefaultPluginIcon_ (QIcon (":/resources/images/defaultpluginicon.svg"))
	, PluginTreeBuilder_ (new PluginTreeBuilder)
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

		PluginTreeBuilder_->AddObjects (Plugins_);
		PluginTreeBuilder_->Calculate ();
		QObjectList ordered = PluginTreeBuilder_->GetResult ();

		Q_FOREACH (QObject *obj, ordered)
		{
			IInfo *ii = qobject_cast<IInfo*> (obj);
			try
			{
				qDebug () << "Initializing" << ii->GetName ();
				ii->Init (ICoreProxy_ptr (new CoreProxy ()));
				Core::Instance ().Setup (obj);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "while initializing"
						<< obj
						<< "got"
						<< e.what ();
				continue;
			}
		}

		QObjectList plugins2 = GetAllCastableRoots<IPlugin2*> ();
		Q_FOREACH (IPluginReady *provider, GetAllCastableTo<IPluginReady*> ())
		{
			const QSet<QByteArray>& expected = provider->GetExpectedPluginClasses ();
			Q_FOREACH (QObject *ip2, plugins2)
				if (qobject_cast<IPlugin2*> (ip2)->
						GetPluginClasses ().intersect (expected).size ())
					provider->AddPlugin (ip2);
		}

		Q_FOREACH (QObject *obj, ordered)
		{
			IInfo *ii = qobject_cast<IInfo*> (obj);
			try
			{
				ii->SecondInit ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "while initializing"
						<< obj
						<< "got"
						<< e.what ();
				continue;
			}
		}

		Q_FOREACH (QObject *plugin, GetAllPlugins ())
		Core::Instance ().PostSecondInit (plugin);
	}

	void PluginManager::Release ()
	{
		QObjectList ordered = PluginTreeBuilder_->GetResult ();
		std::reverse (ordered.begin (), ordered.end ());
		Q_FOREACH (QObject *obj, ordered)
		{
			try
			{
				IInfo *ii = qobject_cast<IInfo*> (obj);
				if (!ii)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< obj
							<< "to IInfo";
					continue;
				}
				qDebug () << "Releasing" << ii->GetName ();
				ii->Release ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< obj
						<< e.what ();
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown release error for"
						<< obj;
			}
		}
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
		return PluginTreeBuilder_->GetResult ();
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
	}

	void PluginManager::ReleasePlugin (QObject *object)
	{
		try
		{
			qDebug () << "Releasing"
					<< qobject_cast<IInfo*> (object)->GetName ();
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

	const QStringList& PluginManager::GetPluginLoadErrors () const
	{
		return PluginLoadErrors_;
	}

	void PluginManager::FindPlugins ()
	{
#ifdef Q_WS_WIN
		ScanDir (QApplication::applicationDirPath () + "/plugins/bin");
#elif defined (Q_WS_MAC)
		ScanDir (QApplication::applicationDirPath () + "/../plugins");
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
					<< *i;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get providers"
					<< *i;
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
					<< *i;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
					<< "failed to get class"
					<< *i;
			}
		}
		return result;
	}
}

