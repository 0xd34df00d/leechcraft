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

#ifndef CORE_H
#define CORE_H
#include <memory>
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QString>
#include <QPair>
#include <QTimer>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "pluginmanager.h"
#include "tabmanager.h"
#include "storagebackend.h"
#include "networkaccessmanager.h"
#include "directorywatcher.h"
#include "localsockethandler.h"
#include "clipboardwatcher.h"

class QAbstractProxyModel;
class QAction;
class IDownload;
class IShortcutProxy;
class QToolBar;

namespace LeechCraft
{
	class MainWindow;
	class NewTabMenuManager;

	/** Contains all the plugins' models, maps from end-user's tree view
	 * to plugins' models and much more.
	 */
	class Core : public QObject
	{
		Q_OBJECT

		PluginManager *PluginManager_;
		MainWindow *ReallyMainWindow_;
		std::auto_ptr<TabManager> TabManager_;
		std::auto_ptr<QNetworkAccessManager> NetworkAccessManager_;
		std::auto_ptr<StorageBackend> StorageBackend_;
		std::auto_ptr<DirectoryWatcher> DirectoryWatcher_;
		std::auto_ptr<ClipboardWatcher> ClipboardWatcher_;
		std::auto_ptr<LocalSocketHandler> LocalSocketHandler_;
		boost::shared_ptr<NewTabMenuManager> NewTabMenuManager_;
		QList<Entity> QueuedEntities_;

		Core ();
	public:
		enum FilterType
		{
			FTFixedString
			, FTWildcard
			, FTRegexp
			, FTTags
		};

		virtual ~Core ();
		static Core& Instance ();
		void Release ();

		/** Sets the pointer to the main window.
		 */
		void SetReallyMainWindow (MainWindow*);

		/** Returns the pointer to the main window. The result is valid
		 * only if a valid window was set with SetReallyMainWindow().
		 */
		MainWindow* GetReallyMainWindow ();

		/** Returns the pointer to the app-wide shortcut proxy.
		 */
		const IShortcutProxy* GetShortcutProxy () const;

		/** Returns all plugins that implement IHaveSettings as
		 * QObjectList.
		 *
		 * @return List of objects.
		 */
		QObjectList GetSettables () const;

		/** Returns all plugins that implement IHaveShortcuts as
		 * QObjectList.
		 *
		 * @return List of objects.
		 */
		QObjectList GetShortcuts () const;

		/** Returns all the actions from plugins that implement
		 * IToolBarEmbedder.
		 *
		 * @return List of actions.
		 */
		QList<QList<QAction*> > GetActions2Embed () const;

		/** Returns the model which manages the plugins, displays
		 * various info about them like name, description, icon and
		 * allows to switch them off.
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

		/** Returns pointer to the storage backend of the Core.
		 */
		StorageBackend* GetStorageBackend () const;

		/** Returns toolbar for plugin that represents the tab widget's
		 * page with given index. If the index is invalid or plugin
		 * doesn't provide a toolbar, 0 is returned.
		 *
		 * @param[in] index Index of the tab widget's page with the
		 * plugin.
		 * @return Toolbar for the given plugin's page.
		 */
		QToolBar* GetToolBar (int index) const;

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

		/** Maps given index from a model obtained from GetTasksModel()
		 * to the index provided by a corresponding plugin's model.
		 */
		QModelIndex MapToSource (const QModelIndex& index) const;

		/** Returns the app-wide TabContainer.
		 */
		TabManager* GetTabManager () const;

		/** Returns the app-wide new tab menu manager.
		 */
		NewTabMenuManager* GetNewTabMenuManager () const;

		/** Sets up connections for the given object which is expected
		 * to be a plugin instance.
		 */
		void Setup (QObject *object);

		void PostSecondInit (QObject *object);

		/** Some preprocessor black magick to initialize storage and a
		 * method per each hook signature.
		 */
		template<LeechCraft::HookID id>
			typename LeechCraft::HooksContainer<id>::Functors_t GetHooks () const;
#define LC_STRN(a) a##_
#define LC_DEFINE_REGISTER(a) \
	private: \
		LeechCraft::HooksContainer<a> LC_STRN(a); \
	public: \
		void RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t);
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER (HOOKS_TYPES_LIST);
#undef LC_DEFINE_REGISTER

		virtual bool eventFilter (QObject*, QEvent*);
	public slots:
		/** Handles changes of proxy settings in the Settings Dialog.
		 */
		void handleProxySettings () const;
		/* Dispatcher of button clicks in the Settings Dialog (of the
		 * settings of type 'pushbutton').
		 */
		void handleSettingClicked (const QString&);

		/** Handles the entity which could be anything - path to a file,
		 * link, contents of a .torrent file etc. If the entity is a
		 * string, this parameter is considered to be an UTF-8
		 * representation of it.
		 *
		 * If id is not null and the job is handled by a downloader,
		 * the return value of IDownloader::AddJob() is assigned to *id.
		 * The same is with the provider.
		 *
		 * @param[in] entity Entity.
		 * @param[out] id The ID of the job if applicable.
		 * @param[out] provider The provider that downloads this job.
		 * @return True if the entity was actually handled.
		 */
		bool handleGotEntity (LeechCraft::Entity entity,
				int *id = 0, QObject **provider = 0);
	private slots:
		/** Returns whether the given entity could be handlerd.
		 *
		 * @param[in] entity The download entity to be checked.
		 * @param[out] could Whether the given entity could be checked.
		 */
		void handleCouldHandle (const LeechCraft::Entity& entity,
				bool *could);

		void queueEntity (LeechCraft::Entity);

		void pullEntityQueue ();

		void handlePluginLoadErrors ();

		/** Handles requests to show a tab above others.
		 */
		void embeddedTabWantsToFront ();

		/** Handles requests to change statusbar's status text.
		 *
		 * @param[in] sender The sender of the event.
		 * @param[in] msg The message to show.
		 */
		void handleStatusBarChanged (QWidget *sender, const QString& msg);
	private:
		enum ObjectType
		{
			OTDownloaders,
			OTHandlers
		};
		/** Returns the list of objects, either downloaders or handlers,
		 * that are able to handle given entity.
		 *
		 * @param[in] entity The download entity to download/handle.
		 * @param[in] downloaders Query for downloaders (if true) or
		 * handlers (if false).
		 * @param[in] detectOnly Only detect the ability to handle the
		 * entity — return immediately after the first suitable object
		 * was found.
		 *
		 * @return The list of objects that are able/download the entity.
		 */
		QList<QObject*> GetObjects (const LeechCraft::Entity& entity,
				ObjectType type, bool detectOnly) const;

		/** Checks whether given entity could be handled or downloaded.
		 *
		 * @param[in] entity The entity to check.
		 * @return Whether the given entity could be handled.
		 */
		bool CouldHandle (LeechCraft::Entity entity) const;

		/** Initializes IInfo's signals of the object.
		 */
		void InitDynamicSignals (QObject *object);

		/** Initializes the object as a IJobHolder. The object is assumed
		 * to be a valid IJobHolder*.
		 */
		void InitJobHolder (QObject *object);

		/** Initializes the object as a IEmbedTab. The object is assumed
		 * to be a valid IEmbedTab*.
		 */
		void InitEmbedTab (QObject *object);

		/** Initializes the object as a IMultiTabs. The object is assumed
		 * to be a valid IMultiTabs*.
		 */
		void InitMultiTab (QObject *object);

		void InitCommonTab (QObject *object);

		/** Handles the notification. Either logs it or shows to the
		 * user, depending in the notification and preferences.
		 *
		 * @param[in] entity Entity with the notification.
		 */
		void HandleNotify (const LeechCraft::Entity& entity);
	signals:
		/** Notifies the user about an error by a pop-up message box.
		 */
		void error (QString error) const;

		/** Sends the message to the log.
		 */
		void log (const QString& message);
	};
#define LC_DEFINE_REGISTER(a) \
	template<> \
		LeechCraft::HooksContainer<LeechCraft::a>::Functors_t Core::GetHooks<a> () const;
	LC_EXPANDER (HOOKS_TYPES_LIST);
#undef LC_DEFINE_REGISTER
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_STRN
};



#endif

