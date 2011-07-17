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

#ifndef INTERFACES_IINFO_H
#define INTERFACES_IINFO_H
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QTabBar>
#include <QtNetwork/QNetworkAccessManager>
#include "structures.h"
#include <QIcon>

class IShortcutProxy;
class QTreeView;
class QModelIndex;
class QIcon;
class QMainWindow;
class QAbstractItemModel;
class QTabWidget;

namespace LeechCraft
{
	namespace Util
	{
		class BaseSettingsManager;
	};

	/** @brief Class for hook-based communication between plugins.
	 * 
	 * This interface is designed to be implemented by classes that
	 * allow plugins to communicate with each other using hooks. Usage
	 * of somewhat standard implementation, Util::DefaultHookProxy, is
	 * encouraged.
	 * 
	 * The implementation of this interface may also be considered to be
	 * used as a container for parameters that could be passed to hooks,
	 * modified there and used back in the default handler. For that,
	 * GetValue() and SetValue() members are used.
	 * 
	 * Parameters are identified by their names, and the names are
	 * usually documented for each corresponding hook.
	 * 
	 * So, a hook should get the current value of the parameter by
	 * calling GetValue(), do the required work and possibly update the
	 * parameter by calling SetValue().
	 * 
	 * Please note that if several hooks are connected to a single hook
	 * point, the changes to parameters made by previously called hooks
	 * would be visible to next hooks in chain. That is intentional and
	 * by design.
	 * 
	 * It only makes sense to pass parameters like that for objects of
	 * types that are copyable and are usually passed by value or by
	 * reference. For example, that may be standard scalar types (int,
	 * bool), or QString, QByteArray and such.
	 * 
	 * The hook may cancel the default handler of an event by calling
	 * CancelDefault().
	 * 
	 * @sa Util::DefaultHookProxy
	 */
	class IHookProxy
	{
	public:
		virtual ~IHookProxy () {}

		/** @brief Cancels default handler of the event.
		 * 
		 * A canceled event handler can't be uncanceled later.
		 */
		virtual void CancelDefault () = 0;

		/** Returns the current "return value" of this hook call chain.
		 *
		 * @return The current "return value".
		 */
		virtual const QVariant& GetReturnValue () const = 0;

		/** Sets the "return value" of this hook chain. Consequent
		 * calls to this function would overwrite the previously set
		 * value.
		 *
		 * @param[in] value The new return value of this hook.
		 */
		virtual void SetReturnValue (const QVariant& value) = 0;
		
		/** @brief Returns the value of the given parameter.
		 * 
		 * This function returns current value of the given parameter,
		 * or a null QVariant() if no such parameter has been set.
		 * 
		 * @param[in] name The name of the parameter.
		 * @return The parameter's value or null QVariant() if no such
		 * parameter exists.
		 * 
		 * @sa SetValue()
		 */
		virtual QVariant GetValue (const QByteArray& name) const = 0;

		/** @brief Updates the value of the given parameter.
		 * 
		 * This function sets the value of the parameter identified by
		 * name, possibly overwriting previous value (if any).
		 * 
		 * Setting a null QVariant as value effectively erases the
		 * parameter value.
		 * 
		 * @param[in] name The name of the parameter.
		 * @param[in] value The new value of the parameter.
		 * 
		 * @sa GetValue()
		 */
		virtual void SetValue (const QByteArray& name, const QVariant& value) = 0;
	};

	typedef boost::shared_ptr<IHookProxy> IHookProxy_ptr;

	enum HookID
	{
		/** Is called in the beginnign of creating request in the
		 * application-wide network access manager.
		 *
		 * IHookProxy::CancelDefault() cancels default request creation
		 * and returns the reply obtained from the hook.
		 */
		HIDNetworkAccessManagerCreateRequest,

		/** Is called in the beginning of the manual job addition
		 * handler.
		 *
		 * IHookProxy::CancelDefault() cancels default processing.
		 */
		HIDManualJobAddition,

		/** Is called in the beginning of the method handling the
		 * "couldHandle" request (IInfo::couldHandle() signal).
		 *
		 * IHookProxy::CancelDefault() cancels default processing
		 * and returns the result from the hook from the handler.
		 */
		HIDCouldHandle,

		/** Is called in the beginning of the method handling the
		 * "gotEntity" request (IInfo::gotEntity() signal).
		 *
		 * IHookProxy::CancelDefault() cancels default processing
		 * and returns the result from the hook from the handler.
		 */
		HIDGotEntity,

		/** Is called in the beginning of the method handling the
		 * changed of the status bar text from the plugins.
		 *
		 * IHookProxy::CancelDefault() cancels default processing
		 * and returns the result from the hook from the handler.
		 */
		HIDStatusBarChanged
	};

	template<int>
	struct HookSignature;

	template<>
		struct HookSignature<HIDNetworkAccessManagerCreateRequest>
		{
			/** @param[in,out] manager The network access manager.
			 * @param[in,out] op The operation performed.
			 * @param[in,out] req The request to which the reply should be
			 * created.
			 * @param[in,out] op The device with the outgoing data.
			 * @return The newly created reply. If
			 * IHookProxy::CancelDefault() wasn't called, the return
			 * value is ignored.
			 */
			typedef boost::function<QNetworkReply* (IHookProxy_ptr,
					QNetworkAccessManager *manager,
					QNetworkAccessManager::Operation *op,
					QNetworkRequest *req,
					QIODevice **dev)> Signature_t;
		};

	template<>
		struct HookSignature<HIDManualJobAddition>
		{
			/** @param[in,out] entity The entity to be manually added.
			 * @paarm[in,out] The location where the entity should be saved.
			 */
			typedef boost::function<void (IHookProxy_ptr,
					QString *entity)> Signature_t;
		};

	template<>
		struct HookSignature<HIDCouldHandle>
		{
			/** @param[in,out] entity The entity to be queried for.
			 * @return Whether the entity can be handled or not. If
			 * IHookProxy::CancelDefault() wasn't called, the return
			 * value is ignored.
			 */
			typedef boost::function<bool (IHookProxy_ptr,
					LeechCraft::Entity *entity)> Signature_t;
		};

	template<>
		struct HookSignature<HIDGotEntity>
		{
			/** @param[in,out] entity The entity that is either got or
			 * should be delegated.
			 * @param[in,out] id The id of task in case of delegation.
			 * Please note that this pointer can be NULL.
			 * @param[in,out] provider The provider object in case of
			 * delegation. Please not that this pointer can be NULL.
			 * @param[in,out] sender The sender of the signal.
			 * @return Whether the entity was delegated or not. If
			 * IHookProxy::CancelDefault() wasn't called, the return
			 * value is ignored.
			 */
			typedef boost::function<bool (IHookProxy_ptr,
					LeechCraft::Entity *entity,
					int *id,
					QObject **provider,
					QObject *sender)> Signature_t;
		};

	template<>
		struct HookSignature<HIDStatusBarChanged>
		{
			/** @param[in,out] sendWidget The widget that sent this
			 * signal.
			 * @param[in,out] newStatus The new contents of the status
			 * bar.
			 */
			typedef boost::function<void (IHookProxy_ptr,
					QWidget *sendWidget,
					QString *newStatus)> Signature_t;
		};

	template<int id>
		struct HooksContainer
		{
			typedef QList<typename HookSignature<id>::Signature_t> Functors_t;
			Functors_t Functors_;
		};
};

#define HOOKS_TYPES_LIST (HIDNetworkAccessManagerCreateRequest)\
	(HIDManualJobAddition)\
	(HIDCouldHandle)\
	(HIDGotEntity)\
	(HIDStatusBarChanged)

/** @brief Tags manager's interface.
 *
 * This interface is for communication with the tags manager.
 *
 * Object returned by the GetObject() function emits these signals:
 * - tagsUpdated(const QStringList& tags) when the tags are updated.
 */
class ITagsManager
{
public:
	typedef QString tag_id;

	virtual ~ITagsManager () {}

	/** @brief Returns the ID of the given tag.
	 *
	 * If there is no such tag, it's added to the tag collection and the
	 * id of the new tag is returned.
	 *
	 * @param[in] tag The tag that should be identified.
	 * @return The ID of the tag.
	 *
	 * @sa GetTag
	 */
	virtual tag_id GetID (const QString& tag) = 0;

	/** @brief Returns the tag with the given id.
	 *
	 * If there is no such tag, a null QString is returned. A sensible
	 * plugin would delete the given id from the list of assigned tags
	 * for all the items with this id.
	 *
	 * @param[in] id The id of the tag.
	 * @return The tag.
	 *
	 * @sa GetID
	 */
	virtual QString GetTag (tag_id id) const = 0;

	/** Returns all tags existing in LeechCraft now.
	 *
	 * @return List of all tags.
	 */
	virtual QStringList GetAllTags () const = 0;

	/** @brief Splits the given string with tags to the list of the tags.
	 *
	 * @param[in] string String with tags.
	 * @return The list of the tags.
	 */
	virtual QStringList Split (const QString& string) const = 0;

	/** @brief Joins the given tags into one string that's suitable to
	 * display to the user.
	 *
	 * @param[in] tags List of tags.
	 * @return The joined string with tags.
	 */
	virtual QString Join (const QStringList& tags) const = 0;

	/** @brief Returns the completion model for this.
	 */
	virtual QAbstractItemModel* GetModel () = 0;

	/** @brief Returns the tags manager as a QObject to get access to
	 * all the meta-stuff.
	 */
	virtual QObject* GetObject () = 0;
};

/** @brief This interface is used to represent LeechCraft's global
 * plugin manager.
 *
 * Through is interface you can get information about currently
 * installed and running plugins, communicate with them at low-level
 * (for example, by getting a pointer to the corresponding QObject* and
 * casting it to any interface you want). It is also possible to inject
 * new plugins into LeechCraft via this interface and try to release
 * already running ones.
 *
 * This object also has the following signals:
 * - pluginInjected(QObject*), which is emitted after a successful
 *   plugin injection.
 */
class IPluginsManager
{
public:
	virtual ~IPluginsManager () {}

	/** @brief Returns list of pointers to all present plugins.
	 *
	 * @return The list of pointers to all present plugins.
	 */
	virtual QObjectList GetAllPlugins () const = 0;

	/** @brief Filters the given list of plugins and returns only those
	 * that can be casted to the given template type.
	 *
	 * Please note that you will almost always call it as Filter<T*> —
	 * note that you check against the pointer type.
	 *
	 * @param[in] source The list of plugins to filter.
	 * @return The list of plugins from source that can be casted ty type T.
	 */
	template<typename T> QObjectList Filter (QObjectList source) const
	{
		QObjectList result;
		Q_FOREACH (QObject *sp, source)
			if (qobject_cast<T> (sp))
				result << sp;
		return result;
	}

	/** @brief This is the same as Filter<T> (GetAllPlugins()).
	 *
	 * This function takes all available plugins from GetAllPlugins()
	 * and returns only those that can be casted to T via passing the
	 * result of GetAllPlugins() to Filter<T>().
	 *
	 * @return The list of pointers to plugin instances that are
	 * castable to type T.
	 */
	template<typename T> QObjectList GetAllCastableRoots () const
	{
		return Filter<T> (GetAllPlugins ());
	}

	/** @brief Similar to GetAlLCastableRoots() and provided for
	 * convenience.
	 *
	 * This function is almost the same as GetAllCastableRoots(), except
	 * it returns the list of pointers to the corresponding interface.
	 * Generally, it takes the result of GetAllCastableRoots() and
	 * converts each pointer from the list returned by
	 * GetAllCastableRoots() to the requested interface.
	 *
	 * @return The list of pointers to the requested interface.
	 */
	template<typename T> QList<T> GetAllCastableTo () const
	{
		QObjectList roots = GetAllCastableRoots<T> ();
		QList<T> result;
		Q_FOREACH (QObject *root, roots)
			result << qobject_cast<T> (root);
		return result;
	}

	/** @brief Returns plugin identified by its id.
	 *
	 * If there is no such plugin with the given id, this function
	 * returns a null pointer.
	 *
	 * @param[in] id The ID of the plugin.
	 * @return The plugin instance or null if no such plugin exists.
	 */
	virtual QObject* GetPluginByID (const QByteArray& id) const = 0;

	/** @brief Returns the library path from which plugin instance
	 * object was loaded loaded.
	 *
	 * If the path could not be determined for some reason (for example,
	 * if the plugin was injected or provided by a plugin adaptor, or if
	 * the given object is not a plugin instance) this function returns
	 * an empty string.
	 *
	 * Please note that you cannot get library path for an arbitrary
	 * object. Only objects that are plugin instances ("root" objects in
	 * a plugin) are supported.
	 *
	 * @param[in] object The object for which to get the library path.
	 * @return
	 */
	virtual QString GetPluginLibraryPath (const QObject* object) const = 0;

	/** @brief Injects the given plugin object.
	 *
	 * The object's semantics are the same as the semantics of the
	 * "root" plugin object (returned by QPluginLoader::instance()).
	 * Thus, object should be castable at least to IInfo interface in
	 * order to be successfully loaded.
	 *
	 * @param[in] object The pointer to the "root" plugin object.
	 *
	 * @sa ReleasePlugin()
	 */
	virtual void InjectPlugin (QObject *object) = 0;

	/** @brief Releases and removes the given plugin object.
	 *
	 * This function tries to release and remove the given object plugin
	 * from LeechCraft. Because of that, object has to be injected
	 * earlier with InjectPlugin().
	 *
	 * Usage of this function is discouraged as it may lead to
	 * instabilities, because other plugins may have established
	 * connections and kept pointers to the object.
	 *
	 * @param[in] object The object previously injected with
	 * InjectPlugin().
	 *
	 * @sa InjectPlugin()
	 */
	virtual void ReleasePlugin (QObject *object) = 0;

	/** @brief Returns the pointer to plugin manager as a QObject.
	 *
	 * You can connect to signals of the plugin manager with the use of
	 * this function, for example.
	 *
	 * @return The plugin manager as a QObject.
	 */
	virtual QObject* GetObject () = 0;
};


class ICoreTabWidget
{
public:
	virtual ~ICoreTabWidget () {}
	
	virtual QObject* GetObject () = 0;
	virtual int WidgetCount () const = 0;
	virtual QWidget* Widget (int) const = 0;
	virtual void AddAction2TabBarLayout (QTabBar::ButtonPosition, QAction*) = 0;
	virtual int IndexOf (QWidget*) const = 0;
	virtual QIcon TabIcon (int) const = 0;
	virtual QString TabText (int) const = 0;
	virtual bool IsPinTab (int) const = 0;
	virtual void setCurrentIndex (int) = 0;
	virtual void setCurrentWidget (QWidget*) = 0;
};


/** @brief Proxy class for the communication with LeechCraft.
 *
 * Allows one to talk with LeechCraft, requesting and getting various
 * services.
 */
class ICoreProxy
{
public:
	virtual ~ICoreProxy () {}

	/** @brief Returns application-wide network access manager.
	 *
	 * If your plugin wants to work well with other internet-related
	 * ones and wants to integrate with application-wide cookie database
	 * and network cache, it should use the returned
	 * QNetworkAccessManager.
	 *
	 * @return Application-wide QNetworkAccessManager.
	 */
	virtual QNetworkAccessManager* GetNetworkAccessManager () const = 0;

	/** @brief Returns the shortcut proxy used to communicate with the
	 * shortcut manager.
	 *
	 * @sa IShortcutProxy
	 */
	virtual const IShortcutProxy* GetShortcutProxy () const = 0;

	/** @brief Maps the given index up to the plugin's through the
	 * hierarchy of LeechCraft's models
	 */
	virtual QModelIndex MapToSource (const QModelIndex&) const = 0;

	/** @brief Returns the LeechCraft's settings manager.
	 *
	 * In the returned settings manager you can use any property name
	 * you want if it starts from "PluginsStorage". To avoid name
	 * collisions from different plugins it's strongly encouraged to
	 * also use the plugin name in the property. So the property name
	 * would look like "PluginsStorage/PluginName/YourProperty".
	 */
	virtual LeechCraft::Util::BaseSettingsManager* GetSettingsManager () const = 0;

	/** @brief Returns the current theme's icon paths for the given name.
	 * Similar to the mapping files.
	 *
	 * There can be different files for different icon sizes. Scalable
	 * icons are considered to have a special value for size: 0.
	 * The return value is the mapping from icon size (only one
	 * dimension since icons are rectangluar) to the path to the graphic
	 * file.
	 *
	 * @param[in] name The name of the icon to search for.
	 * @return Size -> path mapping.
	 *
	 * @sa GetIcon
	 */
	virtual QMap<int, QString> GetIconPath (const QString& name) const = 0;

	/** Returns the current theme's icon for the given on and off
	 * states. Similar to the mapping files.
	 *
	 * @param[in] on The name of the icon in the "on" state.
	 * @param[in] off The name of the icon in the "off" state, if any.
	 * @return The QIcon object created from image files which could be
	 * obtained via GetIconPath().
	 *
	 * @sa GetIconPath
	 */
	virtual QIcon GetIcon (const QString& on, const QString& off = QString ()) const = 0;

	/** Returns main LeechCraft's window.
	 */
	virtual QMainWindow* GetMainWindow () const = 0;

	/** Returns the main tab widget.
	 */
	virtual ICoreTabWidget* GetTabWidget () const = 0;

	/** Returns the application-wide tags manager.
	 */
	virtual ITagsManager* GetTagsManager () const = 0;

	/** Returns the list of all possible search categories from the
	 * finders installed.
	 */
	virtual QStringList GetSearchCategories () const = 0;

	/** @brief Returns an ID for a delegated task from the pool.
	 *
	 * Use this in your downloader plugin when generating an ID for a
	 * newly added task. This way you can avoid ID clashes with other
	 * downloaders.
	 *
	 * @return The ID of the task.
	 *
	 * @sa FreeID()
	 */
	virtual int GetID () = 0;

	/** @brief Marks an ID previously returned by GetID as unused.
	 *
	 * Returns the id to the global ID pool. Use this in your downloader
	 * plugins after your download finishes.
	 *
	 * @param[in] id An ID previously obtained by GetID().
	 *
	 * @sa GetID()
	 */
	virtual void FreeID (int id) = 0;

	/** @brief Returns the application's plugin manager.
	 */
	virtual IPluginsManager* GetPluginsManager () const = 0;

	/** @brief Returns the version of LeechCraft core and base system.
	 */
	virtual QString GetVersion () const = 0;

	/** @brief Returns the pointer to itself as QObject*.
	 *
	 * Just to avoid nasty reinterpret_casts.
	 */
	virtual QObject* GetSelf () = 0;

#define LC_DEFINE_REGISTER(a) virtual void RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t) = 0;
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER (HOOKS_TYPES_LIST);
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER
};

typedef boost::shared_ptr<ICoreProxy> ICoreProxy_ptr;

/** @brief Required interface for every plugin.
 *
 * This interface is a base for all plugins loadable by LeechCraft. If a
 * plugin doesn't provide this interface (qobject_cast<IInfo*> to it
 * fails), it would be considered as a malformed one and would be
 * unloaded.
 *
 * The initialization of plugins is split into two stages: Init() and
 * SecondInit().
 *
 * In the first one plugins ought to initialize their data structures,
 * allocate memory and perform other initializations that don't depend
 * depend on other plugins. After the first stage (after Init()) the
 * plugin should be in a usable state: no call shall fail because
 * required data hasn't been initialized.
 *
 * In the second stage, SecondInit(), plugins can fill the data that
 * depends on other plugins. For example, in SecondInit() the Tab++
 * plugin, which shows a fancy tab tree, queries the MainWindow about
 * existing tabs and about plugins that can open tabs and connects to
 * their respective signals.
 *
 * So, as the rule of thumb, if the action required to initialize your
 * plugin depends on others - move it to SecondInit(), leaving in Init()
 * only basic initialization/allocation stuff like allocation memory for
 * the objects.
 */
class IInfo
{
public:
	/** @brief Initializes the plugin.
	 *
	 * Init is called by the LeechCraft when it has finished
	 * initializing its core and is ready to initialize plugins.
	 * Generally, all initialization code should be placed into this
	 * method instead of plugin's instance object's constructor.
	 *
	 * After this call plugin should be in a usable state. That means
	 * that all data members should be initialized, callable from other
	 * plugins etc. That also means that in this function you can't rely
	 * on other plugins being initialized.
	 *
	 * @param[in] proxy The pointer to proxy to LeechCraft.
	 *
	 * @sa Release
	 * @sa SecondInit
	 */
	virtual void Init (ICoreProxy_ptr proxy) = 0;

	/** @brief Performs second stage of initialization.
	 *
	 * This function is called when all the plugins are initialized by
	 * IInfo::Init() and may now communicate with others with no fear of
	 * getting init-order bugs.
	 *
	 * @sa Init
	 */
	virtual void SecondInit () = 0;

	/** @brief Returns the unique ID of the plugin.
	 *
	 * The ID should never change, event between different versions of
	 * the plugin and between renames of the plugin. It should be unique
	 * among all other plugins, thus the Vendor.AppName form is
	 * suggested. For example, Poshuku Browser plugin would return an ID
	 * like "org.LeechCraft.Poshuku", and Poshuku CleanWeb, which is
	 * Poshuku plugin, would return "org.LeechCraft.Poshuku.CleanWeb".
	 *
	 * The ID is allowed to consist of upper- and lowercase latin
	 * letters, numbers, dots¸ plus and minus sign.
	 *
	 * @return Unique and persistent ID of the plugin.
	 */
	virtual QByteArray GetUniqueID () const = 0;

	/** @brief Returns the name of the plugin.
	 *
	 * This name is used only for the UI, internal communication is done
	 * through pointers to QObjects representing plugin instance
	 * objects.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @sa GetInfo
	 * @sa GetIcon
	 */
	virtual QString GetName () const = 0;

	/** @brief Returns the information string about the plugin.
	 *
	 * Information string is only used to describe the plugin to the
	 * user.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @sa GetName
	 * @sa GetInfo
	 */
	virtual QString GetInfo () const = 0;

	/** @brief Returns the list of provided features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 * 
	 * The default implementation returns an empty list.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of provided features.
	 *
	 * @sa Needs
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Provides () const
	{
		return QStringList ();
	}

	/** @brief Returns the list of needed features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * If not all requirements are satisfied, LeechCraft would mark the
	 * plugin as unusable and would not make it active or use its
	 * features returned by Provides() in dependency calculations. So,
	 * the returned list should mention those features that plugin can't
	 * live without and would not work at all.
	 * 
	 * The default implementation returns an empty list.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of needed features.
	 *
	 * @sa Provides
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Needs () const
	{
		return QStringList ();
	}

	/** @brief Returns the list of used features.
	 *
	 * The return value is used by LeechCraft to calculate the
	 * dependencies between plugins and link them together by passing
	 * object pointers to SetProvider().
	 *
	 * If not all requirements are satisfied, LeechCraft would still
	 * consider the plugin as usable and functional, make it available
	 * to user and use the features returned by Provides() in dependency
	 * calculations. So, the return list should mention those features
	 * that plugin can use but which are not required to start it and do
	 * some basic work.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of used features.
	 *
	 * @sa Provides
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Uses () const
	{
		return QStringList ();
	}

	/** @brief Sets the provider plugin for a given feature.
	 *
	 * This function is called by LeechCraft after dependency
	 * calculations to inform plugin about other plugins which provide
	 * the required features.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @param[in] object Pointer to plugin instance of feature provider.
	 * @param[in] feature The feature which this object provides.
	 *
	 * @sa Provides
	 * @sa Needs
	 * @sa Uses
	 */
	virtual void SetProvider (QObject* object,
			const QString& feature)
	{
		Q_UNUSED (object);
		Q_UNUSED (feature);
	}

	/** @brief Destroys the plugin.
	 *
	 * This function is called to notify that the plugin would be
	 * unloaded soon - either the application is closing down or the
	 * plugin is unloaded for some reason. Plugin should free its
	 * resources and especially all the GUI stuff in this function
	 * instead of plugin instance's destructor.
	 *
	 * @sa Init
	 */
	virtual void Release () = 0;

	/** @brief Returns the plugin icon.
	 *
	 * The icon is used only in GUI stuff.
	 *
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return Icon object.
	 *
	 * @sa GetName
	 * @sa GetInfo
	 */
	virtual QIcon GetIcon () const = 0;

	/** @brief Virtual destructor.
	 *
 * - gotEntity (const LeechCraft::Entity& entity);
 *   Notifies other plugins about a new entity.
 * - delegateEntity (const LeechCraft::Entity& entity, int *id, QObject **provider);
 *   Entity delegation request. If a suitable provider is found, the
 *   entity is delegated to it, id is set according to the task ID
 *   returned from the provider, and provider is set to point to the
 *   provider object.
	 */
	virtual ~IInfo () {}

	/** @brief This signal is emitted by plugin to query if the given
	 * entity could be handled.
	 *
	 * If there is at least one plugin that can handle the given entity,
	 * the could is set to true, otherwise it is set to false.
	 *
	 * @param[out] entity The entity to check if could be handled.
	 * @param[in] could The pointer to the variable that would contain
	 * the result of the check.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @sa gotEntity(), delegateEntity()
	 */
	virtual void couldHandle (const LeechCraft::Entity& entity, bool *could)
	{
		Q_UNUSED (entity);
		Q_UNUSED (could);
	}

	/** @brief This signal is emitted by plugin to notify the Core and
	 * other plugins about an entity.
	 *
	 * In this case, the plugin doesn't care what would happen next to
	 * the entity after the announcement and whether it would be catched
	 * by any other plugin at all. This is the opposite to the semantics
	 * of delegateEntity().
	 *
	 * This signal is typically emitted, for example, when a plugin has
	 * just finished downloading something and wants to notify other
	 * plugins about newly created files.
	 *
	 * This signal is asynchronous: the handling happends after the
	 * control gets back to the event loop.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] entity The entity.
	 */
	virtual void gotEntity (const LeechCraft::Entity& entity)
	{
		Q_UNUSED (entity);
	}

	/** @brief This signal is emitted by plugin to delegate the entity
	 * to an another plugin.
	 *
	 * In this case, the plugin actually cares whether the entity would
	 * be handled. This signal is typically used, for example, to
	 * delegate a download request.
	 *
	 * id and provider are used in download delegation requests. If
	 * these parameters are not NULL and the entity is handled, they are
	 * set to the task ID returned by the corresponding IDownload
	 * instance and the main plugin instance of the handling plugin,
	 * respectively. Thus, setting the id to a non-NULL value means that
	 * only downloading may occur as result but no handling.
	 *
	 * Nevertheless, if you need to enable entity handlers to handle
	 * your request as well, you may leave the id parameter as NULL and
	 * just set the provider to a non-NULL value.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] entity The entity to delegate.
	 * @param[in] id The pointer to the variable that would contain the
	 * task ID of this delegate request, or NULL.
	 * @param[in] provider The pointer to the main plugin instance of
	 * the plugin that handles this delegate request, or NULL.
	 */
	virtual void delegateEntity (const LeechCraft::Entity& entity,
			int *id, QObject **provider)
	{
		Q_UNUSED (entity);
		Q_UNUSED (id);
		Q_UNUSED (provider);
	}
};

Q_DECLARE_INTERFACE (IInfo, "org.Deviant.LeechCraft.IInfo/1.0");
Q_DECLARE_INTERFACE (ICoreProxy, "org.Deviant.LeechCraft.ICoreProxy/1.0");
Q_DECLARE_INTERFACE (ITagsManager, "org.Deviant.LeechCraft.ITagsManager/1.0");
Q_DECLARE_INTERFACE (IPluginsManager, "org.Deviant.LeechCraft.IPluginsManager/1.0");
Q_DECLARE_INTERFACE (ICoreTabWidget, "org.Deviant.LeechCraft.ICoreTabWidget/1.0");

#endif
