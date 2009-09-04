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

#ifndef INTERFACES_IINFO_H
#define INTERFACES_IINFO_H
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/seq.hpp>
#include <QString>
#include <QStringList>
#include <QtPlugin>
#include <QtNetwork/QNetworkAccessManager>
#include "structures.h"

class IShortcutProxy;
class QTreeView;
class QModelIndex;
class QIcon;
class QMainWindow;
class QAbstractItemModel;

namespace LeechCraft
{
	namespace Util
	{
		class BaseSettingsManager;
	};

	class IHookProxy
	{
	public:
		virtual ~IHookProxy () {}

		virtual void CancelDefault () = 0;
	};

	typedef boost::shared_ptr<IHookProxy> IHookProxy_ptr;

	enum HookID
	{
		HIDDownloadFinishedNotification,
		HIDNetworkAccessManagerCreateRequest,
		HIDManualJobAddition,
		HIDCouldHandle,
		HIDGotEntity,
		HIDStatusBarChanged,
		HIDLogString
	};

	template<int>
	struct HookSignature;

	template<>
		struct HookSignature<HIDDownloadFinishedNotification>
		{
			/** @param[in] msg Message to show.
			 * @param[in] show If the notification is enabled in the
			 * settings.
			 */
			typedef boost::function<void (IHookProxy_ptr,
					const QString& msg, bool show)> Signature_t;
		};

	template<>
		struct HookSignature<HIDNetworkAccessManagerCreateRequest>
		{
			typedef boost::function<QNetworkReply* (IHookProxy_ptr,
					QNetworkAccessManager*,
					QNetworkAccessManager::Operation*,
					QNetworkRequest*,
					QIODevice**)> Signature_t;
		};

	template<>
		struct HookSignature<HIDManualJobAddition>
		{
			typedef boost::function<void (IHookProxy_ptr,
					QString *entity,
					QString *location)> Signature_t;
		};

	template<>
		struct HookSignature<HIDCouldHandle>
		{
			typedef boost::function<bool (IHookProxy_ptr,
					const LeechCraft::DownloadEntity& entity)> Signature_t;
		};

	template<>
		struct HookSignature<HIDGotEntity>
		{
			typedef boost::function<bool (IHookProxy_ptr,
					LeechCraft::DownloadEntity *entity,
					int *id,
					QObject **provider,
					QObject *sender)> Signature_t;
		};

	template<>
		struct HookSignature<HIDStatusBarChanged>
		{
			typedef boost::function<void (IHookProxy_ptr,
					QWidget *sendWidget,
					QString *newStatus)> Signature_t;
		};

	template<>
		struct HookSignature<HIDLogString>
		{
			typedef boost::function<void (IHookProxy_ptr,
					QString *string,
					QObject *sender)> Signature_t;
		};

	template<int id>
		struct HooksContainer
		{
			typedef QList<typename HookSignature<id>::Signature_t> Functors_t;
			Functors_t Functors_;
		};
};

#define HOOKS_TYPES_LIST (HIDDownloadFinishedNotification)\
	(HIDNetworkAccessManagerCreateRequest)\
	(HIDManualJobAddition)\
	(HIDCouldHandle)\
	(HIDGotEntity)\
	(HIDStatusBarChanged)\
	(HIDLogString)

/** @brief Tags manager's interface.
 *
 * This interface is for communication with the tags manager.
 */
class ITagsManager
{
public:
	typedef QString tag_id;

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

	virtual ~ITagsManager () {}
};

class IPluginsManager
{
public:
	virtual QObjectList GetAllPlugins () const = 0;

	template<typename T> QObjectList Filter (QObjectList source) const
	{
		QObjectList result;
		Q_FOREACH (QObject *sp, source)
			if (qobject_cast<T> (sp))
				result << sp;
		return result;
	}

	virtual ~IPluginsManager () {}
};

/** @brief Proxy class for the communication with LeechCraft.
 *
 * Allows to talk with LeechCraft, requesting and getting various
 * services.
 */
class ICoreProxy
{
public:
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
	 */
	virtual const IShortcutProxy* GetShortcutProxy () const = 0;

	/** Returns the currently active QTreeView from a Summary tab or 0
	 * if currently active tab is not Summary.
	 */
	virtual QTreeView* GetCurrentView () const = 0;

	/** @brief Maps the given index up to the plugin's through the
	 * hierarchy of LeechCraft's models
	 */
	virtual QModelIndex MapToSource (const QModelIndex&) const = 0;

	/** Returns the LeechCraft's settings manager.
	 */
	virtual LeechCraft::Util::BaseSettingsManager* GetSettingsManager () const = 0;

	/** Returns the current theme's icon for the given on and off
	 * states. Similar to the mapping files.
	 */
	virtual QIcon GetIcon (const QString& on, const QString& off = QString ()) const = 0;

	/** Returns main LeechCraft's window.
	 */
	virtual QMainWindow* GetMainWindow () const = 0;

	/** Returns the application-wide tags manager.
	 */
	virtual ITagsManager* GetTagsManager () const = 0;

	/** Returns the list of all possible search categories from the
	 * finders installed.
	 */
	virtual QStringList GetSearchCategories () const = 0;

	/** Opens a new Summary page with the given query.
	 */
	virtual void OpenSummary (const QString& query = QString ()) const = 0;

	/** Returns an ID for a delegated task from the pool.
	 */
	virtual int GetID () = 0;

	/** Marks an ID previously returned by GetID to the pool.
	 */
	virtual void FreeID (int) = 0;

	/** Returns the object that reemits the signals from the currently
	 * active QTreeView.
	 */
	virtual QObject* GetTreeViewReemitter () const = 0;

	/** @brief Returns the application's plugin manager.
	 */
	virtual IPluginsManager* GetPluginsManager () const = 0;

#define LC_DEFINE_REGISTER(a) virtual void RegisterHook (LeechCraft::HookSignature<LeechCraft::a>::Signature_t) = 0;
#define LC_TRAVERSER(z,i,array) LC_DEFINE_REGISTER (BOOST_PP_SEQ_ELEM(i, array))
#define LC_EXPANDER(Names) BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), LC_TRAVERSER, Names)
	LC_EXPANDER (HOOKS_TYPES_LIST);
#undef LC_EXPANDER
#undef LC_TRAVERSER
#undef LC_DEFINE_REGISTER

	virtual ~ICoreProxy () {}
};

typedef boost::shared_ptr<ICoreProxy> ICoreProxy_ptr;

/** @brief Required interface for every plugin.
 *
 * This interface is a base for all plugins loadable by LeechCraft. If a
 * plugin doesn't provide this interface (qobject_cast<IInfo*> to it
 * fails), it would be considered as a malformed one and would be
 * unloaded.
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
	 * @param[in] proxy The pointer to proxy to LeechCraft.
	 *
	 * @sa Release
	 */
	virtual void Init (ICoreProxy_ptr proxy) = 0;

	/** @brief Returns the name of the plugin.
	 *
	 * This name is used only for the UI, all internals communicate with
	 * each other through pointers to QObjects representing plugin
	 * instance objects.
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
	 * @note This function should be able to work before Init() is
	 * called.
	 *
	 * @return List of provided features.
	 *
	 * @sa Needs
	 * @sa Uses
	 * @sa SetProvider
	 */
	virtual QStringList Provides () const = 0;

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
	 * There are also some special values:
	 * - *
	 *   Passes all plugins to the plugin.
	 * - services::historyModel
	 *   Pushes the pointer to merged but not filtered history model
	 *   into the plugin via pushHistoryModel(MergeModel*) slot.
	 * - services::downloadersModel
	 *   Pushes the pointer to merged but not filtered downloaders model
	 *   into the plugin via pushDownloadersModel(MergeModel*) slot.
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
	virtual QStringList Needs () const = 0;

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
	virtual QStringList Uses () const = 0;

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
			const QString& feature) = 0;

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
	 */
	virtual ~IInfo () {}
};

Q_DECLARE_INTERFACE (IInfo, "org.Deviant.LeechCraft.IInfo/1.0");
Q_DECLARE_INTERFACE (ICoreProxy, "org.Deviant.LeechCraft.ICoreProxy/1.0");
Q_DECLARE_INTERFACE (ITagsManager, "org.Deviant.LeechCraft.ITagsManager/1.0");
Q_DECLARE_INTERFACE (IPluginsManager, "org.Deviant.LeechCraft.IPluginsManager/1.0");

#endif

