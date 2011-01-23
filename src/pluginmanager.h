/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include <boost/shared_ptr.hpp>
#include <QAbstractItemModel>
#include <QMap>
#include <QMultiMap>
#include <QStringList>
#include <QPluginLoader>
#include <QIcon>
#include "interfaces/iinfo.h"

namespace LeechCraft
{
	class MainWindow;
	class PluginTreeBuilder;

	class PluginManager : public QAbstractItemModel
						, public IPluginsManager
	{
		Q_OBJECT
		Q_INTERFACES (IPluginsManager);

		typedef boost::shared_ptr<QPluginLoader> QPluginLoader_ptr;
		typedef QList<QPluginLoader_ptr> PluginsContainer_t;

		// Only currently loaded plugins
		mutable PluginsContainer_t PluginContainers_;
		typedef QList<QObject*> Plugins_t;
		mutable Plugins_t Plugins_;

		// All plugins ever seen
		PluginsContainer_t AvailablePlugins_;
		QMap<QString, PluginsContainer_t::const_iterator> FeatureProviders_;

		QStringList Headers_;
		QIcon DefaultPluginIcon_;
		QStringList PluginLoadErrors_;
		mutable QMap<QByteArray, QObject*> PluginID2PluginCache_;

		boost::shared_ptr<PluginTreeBuilder> PluginTreeBuilder_;
	public:
		typedef PluginsContainer_t::size_type Size_t;
		PluginManager (const QStringList& pluginPaths, QObject *parent = 0);
		virtual ~PluginManager ();

		virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
		virtual bool setData (const QModelIndex&, const QVariant&, int);

		Size_t GetSize () const;
		void Init ();
		void Release ();
		QString Name (const Size_t& pos) const;
		QString Info (const Size_t& pos) const;

		QObjectList GetAllPlugins () const;
		QString GetPluginLibraryPath (const QObject*) const;

		QObject* GetPluginByID (const QByteArray&) const;

		void InjectPlugin (QObject *object);
		void ReleasePlugin (QObject *object);

		QObject* GetObject ();

		QObject* GetProvider (const QString&) const;

		const QStringList& GetPluginLoadErrors () const;
	private:
		void FindPlugins ();
		void ScanDir (const QString&);
		/** Tries to load all the plugins and filters out those who fail
		 * various sanity checks.
		 */
		void CheckPlugins ();

		QList<Plugins_t::iterator> FindProviders (const QString&);
		QList<Plugins_t::iterator> FindProviders (const QSet<QByteArray>&);
	signals:
		void pluginInjected (QObject*);
	};
};

#endif

