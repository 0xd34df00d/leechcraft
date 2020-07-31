/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QMap>
#include <QMultiMap>
#include <QStringList>
#include <QDir>
#include <QIcon>
#include "loaders/ipluginloader.h"
#include "interfaces/iinfo.h"
#include "interfaces/core/ipluginsmanager.h"

namespace LC
{
	class MainWindow;
	class PluginTreeBuilder;

	class PluginManager : public QAbstractItemModel
						, public IPluginsManager
	{
		Q_OBJECT
		Q_INTERFACES (IPluginsManager)

		const bool DBusMode_;

		using PluginsContainer_t = QList<Loaders::IPluginLoader_ptr>;

		// Only currently loaded plugins
		mutable PluginsContainer_t PluginContainers_;
		using Plugins_t = QList<QObject*>;
		mutable Plugins_t Plugins_;

		QMap<QObject*, Loaders::IPluginLoader_ptr> Obj2Loader_;

		// All plugins ever seen
		PluginsContainer_t AvailablePlugins_;

		QStringList Headers_;
		QIcon DefaultPluginIcon_;
		QStringList PluginLoadErrors_;
		mutable QMap<QByteArray, QObject*> PluginID2PluginCache_;

		std::shared_ptr<PluginTreeBuilder> PluginTreeBuilder_;

		mutable bool CacheValid_ = false;
		mutable QObjectList SortedCache_;

		class PluginLoadProcess;
	public:
		enum Roles
		{
			PluginObject = Qt::UserRole + 100,
			PluginID,
			PluginFilename
		};

		enum class InitStage
		{
			BeforeFirst,
			BeforeSecond,
			PostSecond,
			Complete
		};
	private:
		InitStage InitStage_ = InitStage::BeforeFirst;
	public:
		using Size_t = PluginsContainer_t::size_type;

		PluginManager (const QStringList& pluginPaths, QObject *parent = 0);

		int columnCount (const QModelIndex& = QModelIndex ()) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& = QModelIndex ()) const override;
		bool setData (const QModelIndex&, const QVariant&, int) override;

		Size_t GetSize () const;
		void Init (bool safeMode);
		void Release ();
		QString Name (const Size_t& pos) const;
		QString Info (const Size_t& pos) const;

		QList<Loaders::IPluginLoader_ptr> GetAllAvailable () const;
		QObjectList GetAllPlugins () const override;
		QString GetPluginLibraryPath (const QObject*) const override;

		QObject* GetPluginByID (const QByteArray&) const override;

		QObjectList GetFirstLevels (const QByteArray& pclass) const;
		QObjectList GetFirstLevels (const QSet<QByteArray>& pclasses) const;

		void InjectPlugin (QObject *object) override;
		void ReleasePlugin (QObject *object) override;

		void SetAllPlugins (Qt::CheckState);

		QObject* GetQObject () override;

		void OpenSettings (QObject*) override;

		ILoadProgressReporter_ptr CreateLoadProgressReporter (QObject*) override;

		QIcon GetPluginIcon (QObject*) override;

		const QStringList& GetPluginLoadErrors () const;

		InitStage GetInitStage () const;
	private:
		void SetInitStage (InitStage);

		QStringList FindPluginsPaths () const;
		void FindPlugins ();
		void ScanPlugins (const QStringList&);

		/** Tries to load all the plugins and filters out those who fail
		 * various sanity checks.
		 */
		void CheckPlugins ();

		/** Fills the Plugins_ list with all instances, both from "real"
		 * plugins and from adaptors.
		 */
		void FillInstances ();

		/** Tries to perform IInfo::Init() on all plugins. Returns the
		 * list of plugins that failed.
		 */
		QList<QObject*> FirstInitAll (PluginLoadProcess*);

		/** Tries to perform IInfo::Init() on plugins and returns the
		 * first plugin that has failed to initialize. This function
		 * stops initializing plugins upon first failure. If all plugins
		 * were initialized successfully, this function returns NULL.
		 */
		QObject* TryFirstInit (QObjectList, PluginLoadProcess*);

		/** Plainly tries to find a corresponding QPluginLoader and
		 * unload the corresponding library.
		 */
		void TryUnload (QObjectList);

		Loaders::IPluginLoader_ptr MakeLoader (const QString&);

		QList<Plugins_t::iterator> FindProviders (const QString&);
		QList<Plugins_t::iterator> FindProviders (const QSet<QByteArray>&);
	signals:
		void pluginInjected (QObject*);

		void initStageChanged (PluginManager::InitStage);
	};
}
