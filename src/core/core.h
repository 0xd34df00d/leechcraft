/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef CORE_H
#define CORE_H
#include <memory>
#include <QObject>
#include <QString>
#include <QPair>
#include <QTimer>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "pluginmanager.h"

class QAbstractProxyModel;
class QAction;
class IDownload;
class IShortcutProxy;
class QToolBar;
class QNetworkAccessManager;

namespace LC
{
	class RootWindowsManager;
	class MainWindow;
	class NewTabMenuManager;
	class LocalSocketHandler;
	class CoreInstanceObject;
	class DockManager;

	/** Contains all the plugins' models, maps from end-user's tree view
	 * to plugins' models and much more.
	 */
	class Core : public QObject
	{
		Q_OBJECT

		PluginManager *PluginManager_ = nullptr;
		std::shared_ptr<QNetworkAccessManager> NetworkAccessManager_;
		std::shared_ptr<LocalSocketHandler> LocalSocketHandler_;
		std::shared_ptr<NewTabMenuManager> NewTabMenuManager_;
		std::shared_ptr<CoreInstanceObject> CoreInstanceObject_;
		std::shared_ptr<RootWindowsManager> RootWindowsManager_;
		DockManager *DM_;
		bool IsShuttingDown_ = false;

		Core ();
	public:
		enum FilterType
		{
			FTFixedString
			, FTWildcard
			, FTRegexp
			, FTTags
		};

		static Core& Instance ();
		void Release ();

		bool IsShuttingDown () const;

		/** Returns the dock manager over the main window.
		 */
		DockManager* GetDockManager () const;

		/** Returns the pointer to the app-wide shortcut proxy.
		 */
		IShortcutProxy* GetShortcutProxy () const;

		/** Returns all plugins that implement IHaveSettings as
		 * QObjectList.
		 *
		 * @return List of objects.
		 */
		QObjectList GetSettables () const;

		/** Returns all the actions from plugins that implement
		 * IToolBarEmbedder.
		 *
		 * @return List of actions.
		 */
		QList<QList<QAction*>> GetActions2Embed () const;

		/** Returns the model which manages the plugins, displays
		 * various info about them like name, description, icon and
		 * allows one to switch them off.
		 *
		 * For example, this model is used in the Plugin Manager page
		 * in the settings.
		 */
		QAbstractItemModel* GetPluginsModel () const;

		/** Returns pointer to the app-wide Plugin Manager.
		 *
		 * Note that plugin manager is only initialized after the call
		 * to DelayedInit().
		 */
		PluginManager* GetPluginManager () const;

		/** @brief Returns the pointer to the core instance.
		 *
		 * The core instance object is inserted into the plugin manager
		 * to simulate the plugin-like behavior of the Core: some of its
		 * functions are better and shorter expressed as if we consider
		 * the Core to be a plugin for itself.
		 */
		CoreInstanceObject* GetCoreInstanceObject () const;

		/** Performs the initialization of systems that are dependant
		 * on others, like the main window or the Tab Contents Manager.
		 */
		void DelayedInit ();

		/** Tries to add a task from the Add Task Dialog.
		 */
		void TryToAddJob (QString);

		/** Returns true if both indexes belong to the same model. If
		 * both indexes are invalid, true is returned.
		 *
		 * The passed indexes shouldn't be mapped to source from filter
		 * model or merge model, Core will do it itself.
		 *
		 * @param[in] i1 The first index.
		 * @param[in] i2 The second index.
		 * @return Whether the indexes belong to the same model.
		 */
		bool SameModel (const QModelIndex& i1, const QModelIndex& i2) const;

		/** Calculates and returns current upload/download speeds.
		 */
		QPair<qint64, qint64> GetSpeeds () const;

		/** Returns the app-wide network access manager.
		 */
		QNetworkAccessManager* GetNetworkAccessManager () const;

		RootWindowsManager* GetRootWindowsManager () const;

		/** Maps given index from a model obtained from GetTasksModel()
		 * to the index provided by a corresponding plugin's model.
		 */
		QModelIndex MapToSource (const QModelIndex& index) const;

		/** Returns the app-wide new tab menu manager.
		 */
		NewTabMenuManager* GetNewTabMenuManager () const;

		/** Sets up connections for the given object which is expected
		 * to be a plugin instance.
		 */
		void Setup (QObject *object);

		void PostSecondInit (QObject *object);
	public slots:
		/* Dispatcher of button clicks in the Settings Dialog (of the
		 * settings of type 'pushbutton').
		 */
		void handleSettingClicked (const QString&);

		/** Handles the entity which could be anything - path to a file,
		 * link, contents of a .torrent file etc. If the entity is a
		 * string, this parameter is considered to be an UTF-8
		 * representation of it.
		 *
		 * @param[in] entity Entity.
		 * @return True if the entity was actually handled.
		 */
		bool handleGotEntity (LC::Entity entity);
	private slots:
		void handlePluginLoadErrors ();
	private:
		/** Initializes IInfo's signals of the object.
		 */
		void InitDynamicSignals (const QObject *object);
	signals:
		/** Notifies the user about an error by a pop-up message box.
		 */
		void error (QString error) const;

		void initialized ();
	};
};



#endif

